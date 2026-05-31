#ifndef MAPOBJECTS_H
#define MAPOBJECTS_H

#include "LightDefs.h"
#include <memory>
#include "SceneNode.h"

#include "Lighting.h"

struct MapLightRegistry
{
    std::vector<MapPointLight> pointLights;
    std::vector<SpotDef>       spotLights;
};

MapLightRegistry& mapLightRegistry();

// Load assets from objects-map.csv into the scene graph; fills light registry.
std::shared_ptr<SceneNode> buildMapObjectsNode(MapLightRegistry& lights);

void applyMapLights(Lighting& lighting, const MapLightRegistry& reg);

#endif // MAPOBJECTS_H
