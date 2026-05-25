#include "MapObjects.h"
#include "NodeUtils.h"
#include "GlbShape.h"
#include "Bridge.h"
#include "Crate.h"
#include "Wave.h"
#include "Gazebo.h"
#include "Umbrella.h"
#include "Bollard.h"
#include "FloodLight.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace {

constexpr const char* MAP_CSV = "objects-map.csv";

// GLB horizontal size at scale=1 (model units ≈ metres) — used to fit inside map clicks.
constexpr float GAZEBO_MODEL_XZ  = 26.0f;
constexpr float WAVE_MODEL_XZ    = 10.0f;
constexpr float BRIDGE_MODEL_XZ  = 4.0f;
constexpr float CRATE_MODEL_XZ   = 1.2f;
constexpr float UMBRELLA_SCALE   = 0.35f;
// GLB origins sit below ground — lift base so bottoms rest on turf (y=0).
constexpr float CRATE_Y_LIFT_PER_SCALE  = 0.48f;
constexpr float UMBRELLA_Y_LIFT         = 0.22f;
constexpr float FIT_MARGIN       = 0.88f;
constexpr float FIT_MARGIN_BRIDGE = 0.95f;
constexpr float SCALE_BOLLARD    = 0.07f;
constexpr float SCALE_FLOODLIGHT = 0.85f;
constexpr float Y_FLOODLIGHT     = 1.0f;
constexpr float MAX_PLACEMENT_SCALE = 1.2f;

struct PxPt { int idx; int x; int y; };

std::pair<float, float> toWorld(int px, int py)
{
    float x = (px - 1) / 819.f * 40.f - 20.f;
    float z = -((py - 33) / 938.f * 55.f - 27.5f);
    return {x, z};
}

struct Placement
{
    float x, y, z, scale, yaw;
};

float dist2d(float ax, float az, float bx, float bz)
{
    float dx = bx - ax, dz = bz - az;
    return std::sqrt(dx * dx + dz * dz);
}

Placement placementFromCorners(const std::vector<std::pair<float, float>>& corners,
                               float baseY, float scaleMul)
{
    Placement p{};
    if (corners.empty()) return p;

    float cx = 0.f, cz = 0.f;
    for (const auto& c : corners) { cx += c.first; cz += c.second; }
    cx /= (float)corners.size();
    cz /= (float)corners.size();

    float span = 0.f;
    for (size_t i = 0; i < corners.size(); ++i) {
        const auto& a = corners[i];
        const auto& b = corners[(i + 1) % corners.size()];
        span = std::max(span, dist2d(a.first, a.second, b.first, b.second));
    }

    float yaw = 0.f;
    if (corners.size() >= 2) {
        float dx = corners[1].first - corners[0].first;
        float dz = corners[1].second - corners[0].second;
        yaw = std::atan2(dx, dz);
    }

    p.x = cx;
    p.y = baseY;
    p.z = cz;
    p.scale = std::min(span * scaleMul, MAX_PLACEMENT_SCALE);
    p.yaw = yaw;
    return p;
}

// Fit uniform scale so the model sits inside the axis-aligned bbox of all markers.
Placement placementFitFootprint(const std::vector<std::pair<float, float>>& pts,
                                float baseY, float modelExtentXZ)
{
    Placement p{};
    if (pts.empty()) return p;

    float minX = pts[0].first, maxX = pts[0].first;
    float minZ = pts[0].second, maxZ = pts[0].second;
    float cx = 0.f, cz = 0.f;
    for (const auto& pt : pts) {
        minX = std::min(minX, pt.first);
        maxX = std::max(maxX, pt.first);
        minZ = std::min(minZ, pt.second);
        maxZ = std::max(maxZ, pt.second);
        cx += pt.first;
        cz += pt.second;
    }
    cx /= (float)pts.size();
    cz /= (float)pts.size();

    const float width  = maxX - minX;
    const float depth  = maxZ - minZ;
    const float fitXZ  = std::min(width, depth);

    p.x = cx;
    p.y = baseY;
    p.z = cz;
    p.scale = FIT_MARGIN * fitXZ / modelExtentXZ;
    if (p.scale > MAX_PLACEMENT_SCALE)
        p.scale = MAX_PLACEMENT_SCALE;

    if (pts.size() >= 2) {
        float dx = pts[1].first - pts[0].first;
        float dz = pts[1].second - pts[0].second;
        p.yaw = std::atan2(dx, dz);
    }
    return p;
}

std::map<std::string, std::vector<PxPt>> loadMapCsv()
{
    std::map<std::string, std::vector<PxPt>> out;
    std::ifstream in(MAP_CSV);
    if (!in) {
        std::cerr << "[MapObjects] Could not open " << MAP_CSV << "\n";
        return out;
    }

    std::string line;
    std::getline(in, line);
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string obj, ptStr, xs, ys;
        std::getline(ss, obj, ',');
        std::getline(ss, ptStr, ',');
        std::getline(ss, xs, ',');
        std::getline(ss, ys, ',');
        out[obj].push_back({std::stoi(ptStr), std::stoi(xs), std::stoi(ys)});
    }

    for (auto& kv : out)
        std::sort(kv.second.begin(), kv.second.end(),
                  [](const PxPt& a, const PxPt& b) { return a.idx < b.idx; });
    return out;
}

std::vector<std::pair<float, float>> toWorldPts(const std::vector<PxPt>& pts)
{
    std::vector<std::pair<float, float>> w;
    w.reserve(pts.size());
    for (const auto& p : pts)
        w.push_back(toWorld(p.x, p.y));
    return w;
}

void addGlb(std::shared_ptr<SceneNode>& group, GlbShape* shape)
{
    registerShape(shape);
    group->addChild(shapeNode(shape));
}

} // namespace

MapLightRegistry& mapLightRegistry()
{
    static MapLightRegistry reg;
    return reg;
}

std::shared_ptr<SceneNode> buildMapObjectsNode(MapLightRegistry& lights)
{
    auto group = std::make_shared<SceneNode>();
    lights.pointLights.clear();
    lights.spotLights.clear();
    const auto data = loadMapCsv();

    if (const auto it = data.find("crates"); it != data.end()) {
        const auto& pts = it->second;
        for (size_t i = 0; i + 3 < pts.size(); i += 4) {
            std::vector<PxPt> quad(pts.begin() + (int)i, pts.begin() + (int)i + 4);
            Placement pl = placementFitFootprint(toWorldPts(quad), 0.f, CRATE_MODEL_XZ);
            pl.y += pl.scale * CRATE_Y_LIFT_PER_SCALE;
            if (pl.scale > 0.05f)
                addGlb(group, makeGlbShape<Crate>(pl.x, pl.y, pl.z, pl.scale, pl.yaw));
        }
    }

    if (const auto it = data.find("umbrella"); it != data.end()) {
        for (const auto& p : it->second) {
            auto w = toWorld(p.x, p.y);
            addGlb(group, makeGlbShape<Umbrella>(
                w.first, UMBRELLA_Y_LIFT, w.second, UMBRELLA_SCALE, 0.f));
        }
    }

    if (const auto it = data.find("bridge"); it != data.end()) {
        const auto& pts = it->second;
        for (size_t i = 0; i + 3 < pts.size(); i += 4) {
            std::vector<PxPt> quad(pts.begin() + (int)i, pts.begin() + (int)i + 4);
            auto world = toWorldPts(quad);
            Placement pl = placementFitFootprint(world, 0.f, BRIDGE_MODEL_XZ);
            pl.scale *= (FIT_MARGIN_BRIDGE / FIT_MARGIN);
            if (pl.scale > 0.05f)
                addGlb(group, makeGlbShape<Bridge>(pl.x, pl.y, pl.z, pl.scale, pl.yaw));
        }
    }

    if (const auto it = data.find("wave"); it != data.end()) {
        std::vector<PxPt> wavePts;
        for (const auto& p : it->second)
            if (p.idx >= 1 && p.idx <= 4)
                wavePts.push_back(p);
        if (wavePts.size() == 4) {
            Placement pl = placementFitFootprint(toWorldPts(wavePts), 0.f, WAVE_MODEL_XZ);
            addGlb(group, makeGlbShape<Wave>(pl.x, pl.y, pl.z, pl.scale, pl.yaw));
        }
    }

    if (const auto it = data.find("bollard"); it != data.end()) {
        for (const auto& p : it->second) {
            auto w = toWorld(p.x, p.y);
            auto* b = makeGlbShape<Bollard>(w.first, 0.f, w.second, SCALE_BOLLARD, 0.f);
            Vec3 lp = static_cast<Bollard&>(b->mesh()).getLightPos();
            MapPointLight lamp;
            lamp.pos = lp;
            lamp.ambient = {0.f, 0.f, 0.f};
            lamp.diffuse = {0.60f, 0.50f, 0.30f};
            lamp.specular = {0.20f, 0.15f, 0.08f};
            lamp.constant = 1.f;
            lamp.linear = 0.14f;
            lamp.quadratic = 0.07f;
            lamp.castCubeShadow = true;
            lights.pointLights.push_back(lamp);
            addGlb(group, b);
        }
    }

    if (const auto it = data.find("gazebo"); it != data.end()) {
        Placement pl = placementFitFootprint(toWorldPts(it->second), 0.f, GAZEBO_MODEL_XZ);
        auto* gz = makeGlbShape<Gazebo>(pl.x, pl.y, pl.z, pl.scale, pl.yaw);
        Vec3 lp = static_cast<Gazebo&>(gz->mesh()).getLightPos();
        MapPointLight lamp;
        lamp.pos = lp;
        lamp.ambient = {0.f, 0.f, 0.f};
        lamp.diffuse = {0.80f, 0.78f, 0.70f};
        lamp.specular = {0.30f, 0.30f, 0.25f};
        lamp.constant = 1.f;
        lamp.linear = 0.07f;
        lamp.quadratic = 0.017f;
        lamp.castCubeShadow = false;
        lights.pointLights.push_back(lamp);
        addGlb(group, gz);
    }

    if (const auto it = data.find("floodlight"); it != data.end()) {
        for (const auto& p : it->second) {
            auto w = toWorld(p.x, p.y);
            auto* fl = makeGlbShape<FloodLight>(
                w.first, Y_FLOODLIGHT, w.second, SCALE_FLOODLIGHT, 0.f);
            static_cast<FloodLight&>(fl->mesh()).getSpotDefs(lights.spotLights);
            addGlb(group, fl);
        }
    }

    return group;
}

void applyMapLights(Lighting& lighting, const MapLightRegistry& reg)
{
    lighting.mapPointLights = reg.pointLights;
    lighting.spotDefs.clear();
    lighting.spotDefs.insert(lighting.spotDefs.end(),
                             reg.spotLights.begin(), reg.spotLights.end());
}
