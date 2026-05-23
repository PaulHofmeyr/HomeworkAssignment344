#include "Transformations.h"

// Returns the identity matrix
Matrix<3, 3> getIdentity()
{
    // Create a matrix
    Matrix<3, 3> matrixToReturn;

    // Set coordinate values
    matrixToReturn[0][0] = 1.0f;
    matrixToReturn[0][1] = 0.0f;
    matrixToReturn[0][2] = 0.0f;
    matrixToReturn[1][0] = 0.0f;
    matrixToReturn[1][1] = 1.0f;
    matrixToReturn[1][2] = 0.0f;
    matrixToReturn[2][0] = 0.0f;
    matrixToReturn[2][1] = 0.0f;
    matrixToReturn[2][2] = 1.0f;

    // Return matrix
    return matrixToReturn;
}

Matrix<3, 3> makeTranslation(float xDisplacement, float yDisplacement)
{
    // Create a matrix
    Matrix<3, 3> matrixToReturn = getIdentity();

    // Set translation values
    matrixToReturn[0][2] = xDisplacement;
    matrixToReturn[1][2] = yDisplacement;

    // Return matrix
    return matrixToReturn;
}

Matrix<3, 3> makeRotation(float angle)
{
    // Create a matrix
    Matrix<3, 3> matrixToReturn = getIdentity();

    // Set rotation values
    matrixToReturn[0][0] = std::cos(angle);
    matrixToReturn[0][1] = -1 * std::sin(angle);
    matrixToReturn[1][0] = std::sin(angle);
    matrixToReturn[1][1] = std::cos(angle);

    // Return matrix
    return matrixToReturn;
}

Matrix<3, 3> makeScale(float xScale, float yScale)
{
    // Create a matrix
    Matrix<3, 3> matrixToReturn = getIdentity();

    // Set scale values
    matrixToReturn[0][0] = xScale;
    matrixToReturn[1][1] = yScale;

    // Return matrix
    return matrixToReturn;
}

void flattenMatrix(const Matrix<3, 3> &inMatrix, float *outArr)
{
    // Copy values
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            outArr[i * 3 + j] = inMatrix[j][i];
        }
    }
}

Matrix<4, 4> getIdentity4()
{
    Matrix<4, 4> m;
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
    m[2][3] = 0.0f;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
    return m;
}

Matrix<4, 4> makeTranslation3D(float x, float y, float z)
{
    Matrix<4, 4> m = getIdentity4();
    m[0][3] = x;
    m[1][3] = y;
    m[2][3] = z;
    return m;
}

Matrix<4, 4> makeScale3D(float x, float y, float z)
{
    Matrix<4, 4> m = getIdentity4();
    m[0][0] = x;
    m[1][1] = y;
    m[2][2] = z;
    return m;
}

Matrix<4, 4> makeRotationX(float angle)
{
    Matrix<4, 4> m = getIdentity4();
    m[1][1] = std::cos(angle);
    m[1][2] = -std::sin(angle);
    m[2][1] = std::sin(angle);
    m[2][2] = std::cos(angle);
    return m;
}

Matrix<4, 4> makeRotationY(float angle)
{
    Matrix<4, 4> m = getIdentity4();
    m[0][0] = std::cos(angle);
    m[0][2] = std::sin(angle);
    m[2][0] = -std::sin(angle);
    m[2][2] = std::cos(angle);
    return m;
}

Matrix<4, 4> makeRotationZ(float angle)
{
    Matrix<4, 4> m = getIdentity4();
    m[0][0] = std::cos(angle);
    m[0][1] = -std::sin(angle);
    m[1][0] = std::sin(angle);
    m[1][1] = std::cos(angle);
    return m;
}

Matrix<4, 4> makeArbitraryRotation(float angle, float ax, float ay, float az)
{
    // Normalise the axis
    float len = std::sqrt(ax * ax + ay * ay + az * az);
    if (len < 0.00001f)
        return getIdentity4();
    ax /= len;
    ay /= len;
    az /= len;

    float c = std::cos(angle);
    float s = std::sin(angle);
    float t = 1.0f - c;

    // Rodrigues' rotation formula packed into a 4x4 matrix
    Matrix<4, 4> m = getIdentity4();

    m[0][0] = t * ax * ax + c;
    m[0][1] = t * ax * ay - s * az;
    m[0][2] = t * ax * az + s * ay;

    m[1][0] = t * ax * ay + s * az;
    m[1][1] = t * ay * ay + c;
    m[1][2] = t * ay * az - s * ax;

    m[2][0] = t * ax * az - s * ay;
    m[2][1] = t * ay * az + s * ax;
    m[2][2] = t * az * az + c;

    return m;
}

void flattenMatrix4(const Matrix<4, 4> &inMatrix, float *outArr)
{
    // OpenGL expects column-major order
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            outArr[i * 4 + j] = inMatrix[j][i];
}

Matrix<4, 4> makeLookAt(float eyeX, float eyeY, float eyeZ,
                        float centreX, float centreY, float centreZ,
                        float upX, float upY, float upZ)
{
    float fx = centreX - eyeX;
    float fy = centreY - eyeY;
    float fz = centreZ - eyeZ;
    float fLen = std::sqrt(fx * fx + fy * fy + fz * fz);
    fx /= fLen;
    fy /= fLen;
    fz /= fLen;

    float rx = fy * upZ - fz * upY;
    float ry = fz * upX - fx * upZ;
    float rz = fx * upY - fy * upX;
    float rLen = std::sqrt(rx * rx + ry * ry + rz * rz);
    rx /= rLen;
    ry /= rLen;
    rz /= rLen;

    float ux = ry * fz - rz * fy;
    float uy = rz * fx - rx * fz;
    float uz = rx * fy - ry * fx;

    Matrix<4, 4> m = getIdentity4();
    m[0][0] = rx;
    m[0][1] = ry;
    m[0][2] = rz;
    m[1][0] = ux;
    m[1][1] = uy;
    m[1][2] = uz;
    m[2][0] = -fx;
    m[2][1] = -fy;
    m[2][2] = -fz;

    m[0][3] = -(rx * eyeX + ry * eyeY + rz * eyeZ);
    m[1][3] = -(ux * eyeX + uy * eyeY + uz * eyeZ);
    m[2][3] = (fx * eyeX + fy * eyeY + fz * eyeZ);

    return m;
}

Matrix<4, 4> makePerspective(float fovY, float aspect, float near, float far)
{
    float tanHalf = std::tan(fovY / 2.0f);

    Matrix<4, 4> m;
    m[0][0] = 1.0f / (aspect * tanHalf);
    m[1][1] = 1.0f / tanHalf;
    m[2][2] = -(far + near) / (far - near);
    m[2][3] = -(2.0f * far * near) / (far - near);
    m[3][2] = -1.0f;

    return m;
}
Matrix<4, 4> makeOrthographic(float left, float right, float bottom, float top, float near, float far)
{
    Matrix<4, 4> m = getIdentity4();
    m[0][0] =  2.0f / (right - left);
    m[1][1] =  2.0f / (top   - bottom);
    m[2][2] = -2.0f / (far   - near);
    m[0][3] = -(right + left)   / (right - left);
    m[1][3] = -(top   + bottom) / (top   - bottom);
    m[2][3] = -(far   + near)   / (far   - near);
    return m;
}
