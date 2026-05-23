#ifndef SCENE_H
#define SCENE_H

#include "Cuboid.h"
#include "Cylinder.h"
#include "Cone.h"
#include "TriangularPrism.h"
#include "Transformations.h"

void initScene();
void drawScene(bool wireframe);
void drawRotor(bool wireframe);
void cleanupScene();

#endif // SCENE_H
// Rotor transform setter (called from main each frame)
void setRotorTransform(const Matrix<4, 4> &m);
