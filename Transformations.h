#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H

#include "Matrix.h"
#include <cmath>

// 2D transformations

Matrix<3, 3> getIdentity();
Matrix<3, 3> makeTranslation(float xDisplacement, float yDisplacement);
Matrix<3, 3> makeRotation(float angle);
Matrix<3, 3> makeScale(float xScale, float yScale);
void flattenMatrix(const Matrix<3, 3> &inMatrix, float *outArr);

// 3D transformations

Matrix<4, 4> getIdentity4();
Matrix<4, 4> makeTranslation3D(float x, float y, float z);
Matrix<4, 4> makeScale3D(float x, float y, float z);
Matrix<4, 4> makeRotationX(float angle);
Matrix<4, 4> makeRotationY(float angle);
Matrix<4, 4> makeRotationZ(float angle);
Matrix<4, 4> makeArbitraryRotation(float angle, float ax, float ay, float az);
void flattenMatrix4(const Matrix<4, 4> &inMatrix, float *outArr);

Matrix<4, 4> makeLookAt(float eyeX, float eyeY, float eyeZ,
                        float centreX, float centreY, float centreZ,
                        float upX, float upY, float upZ);

Matrix<4, 4> makePerspective(float fovY, float aspect, float near, float far);
Matrix<4, 4> makeOrthographic(float left, float right, float bottom, float top, float near, float far);

#endif