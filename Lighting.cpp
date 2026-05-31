#include "Lighting.h"
#include "Bollard.h"
#include "FloodLight.h"
#include "Gazebo.h"

void Lighting::addBollard(const Bollard& b)
{
    PointDef pd;
    pd.pos       = b.getLightPos();
    pd.ambient   = {0.0f,  0.0f,  0.0f};   // zero ambient — no colour bleed onto scene
    pd.diffuse   = {0.60f, 0.50f, 0.30f};  // warm amber diffuse only
    pd.specular  = {0.20f, 0.15f, 0.08f};
    pd.constant  = 1.0f;
    pd.linear    = 0.14f;   // ~50% at 3m, ~15% at 8m
    pd.quadratic = 0.07f;
    pd.onAtDusk  = true;
    pointDefs.push_back(pd);
}

void Lighting::addFloodLight(const FloodLight& fl)
{
    fl.getSpotDefs(spotDefs);
}

void Lighting::addGazebo(const Gazebo& g)
{
    PointDef pd;
    pd.pos       = g.getLightPos();
    pd.ambient   = {0.0f,  0.0f,  0.0f};   // zero ambient — no colour bleed
    pd.diffuse   = {0.80f, 0.78f, 0.70f};  // soft warm white
    pd.specular  = {0.30f, 0.30f, 0.25f};
    pd.constant  = 1.0f;
    pd.linear    = 0.07f;   // gentle — covers ~4-unit gazebo interior
    pd.quadratic = 0.017f;
    pd.onAtDusk  = true;
    pointDefs.push_back(pd);
}