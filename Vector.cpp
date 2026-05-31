#include "Vector.h"

template <int n>
Vector<n>::Vector()
{
    // Instantiate array of size n
    this->arr = new float[n];

    // Populate with 0.0f
    for (int i = 0; i < n; i++)
    {
        arr[i] = 0.0f;
    }
}

template <int n>
Vector<n>::Vector(std::initializer_list<float> list)
{
    // Instantiate array of size n
    this->arr = new float[n];

    // If list is longer than or equal to array size
    if (list.size() >= n)
    {
        // Take first n values
        const float *ptr = list.begin();
        for (int i = 0; i < n; i++)
        {
            arr[i] = ptr[i];
        }
    }
    else // If list is shorter than array size
    {
        // Populate array
        const float *ptr = list.begin();
        for (int i = 0; i < n; i++)
        {
            // If we're still in range of list's indexes use its data
            if (i <= (int)list.size())
            {
                arr[i] = ptr[i];
            }
            else // else, default to 0.0f
            {
                arr[i] = 0.0f;
            }
        }
    }
}

template <int n>
Vector<n>::Vector(float *inArr)
{
    // Create a shallow copy of arr values
    this->arr = new float[n];
}

template <int n>
Vector<n>::~Vector()
{
    // Delete dynamically allocated memory
    delete[] this->arr;
    this->arr = nullptr;
}

template <int n>
Vector<n>::Vector(const Vector<n> &inVector)
{
    // Instantiate array of size n
    this->arr = new float[n];

    // Create deep copy of inVector's data
    for (int i = 0; i < n; i++)
    {
        arr[i] = inVector[i];
    }
}

template <int n>
Vector<n>::Vector(const Matrix<n, 1> &inMatrix)
{
    // Instantiate array of size n
    this->arr = new float[n];

    // Copy the values from the matrix into the vector's array
    for (int i = 0; i < n; i++)
    {
        arr[i] = inMatrix[i][0];
    }
}

template <int n>
Vector<n> &Vector<n>::operator=(const Vector<n> &inVector)
{
    // Create a deep copy of the arrays
    for (int i = 0; i < n; i++)
    {
        arr[i] = inVector[i];
    }

    // Return this
    return *this;
}

template <int n>
Vector<n> Vector<n>::operator+(const Vector<n> vectorToAdd) const
{
    // Create the new vector
    Vector<n> vectorToReturn = *this;

    // Add the corresponding values and insert them into the vector
    for (int i = 0; i < this->getN(); i++)
    {
        vectorToReturn[i] += vectorToAdd[i];
    }

    // Return the new vector
    return vectorToReturn;
}

template <int n>
Vector<n> Vector<n>::operator-(const Vector<n> vectorToRemove) const
{
    // Create the new vector
    Vector<n> vectorToReturn = *this;

    // Subtract the corresponding values and insert them into the vector
    for (int i = 0; i < this->getN(); i++)
    {
        vectorToReturn[i] -= vectorToRemove[i];
    }

    // Return the new vector
    return vectorToReturn;
}

template <int n>
Vector<n> Vector<n>::operator*(const float scalar) const
{
    // Create the new vector to return from this
    Vector<n> vectorToReturn = *this;

    // Scale each vector element
    for (int i = 0; i < n; i++)
    {
        vectorToReturn[i] = vectorToReturn[i] * scalar;
    }

    // Return scaled vector
    return vectorToReturn;
}

template <int n>
float Vector<n>::operator*(const Vector<n> otherVector) const
{
    // Create a sum variable
    float dotProductValue = 0;

    // Perform dot product
    for (int i = 0; i < n; i++)
    {
        dotProductValue += arr[i] * otherVector[i];
    }

    // Return final value
    return dotProductValue;
}

template <int n>
float Vector<n>::magnitude() const
{
    // Create a sum variable
    float magnitudeToReturn = 0;

    // Sum the squares of each vector element
    for (int i = 0; i < n; i++)
    {
        magnitudeToReturn += this->arr[i] * this->arr[i];
    }

    // Square root the squared sum
    magnitudeToReturn = sqrt(magnitudeToReturn);

    // Return the magnitude of the vector
    return magnitudeToReturn;
}

template <int n>
Vector<n>::operator Matrix<n, 1>() const
{
    // Create a new matrix
    Matrix<n, 1> convertedMatrix;

    // Copy values into the n row
    for (int i = 0; i < n; i++)
    {
        convertedMatrix[i][0] = this->arr[i];
    }

    // Return newly created array
    return convertedMatrix;
}

template <int n>
Vector<3> Vector<n>::crossProduct(const Vector<3> otherVector) const
{
    // Create a vector to store the cross product
    Vector<3> crossProductVector;

    // Compute each element of the cross product
    crossProductVector[0] = (this->arr[1] * otherVector[2]) - (this->arr[2] * otherVector[1]);
    crossProductVector[1] = (this->arr[2] * otherVector[0]) - (this->arr[0] * otherVector[2]);
    crossProductVector[2] = (this->arr[0] * otherVector[1]) - (this->arr[1] * otherVector[0]);

    // Return the cross product
    return crossProductVector;
}

template <int n>
Vector<n> Vector<n>::unitVector() const
{
    // Create a vector to store the unit vector
    Vector<n> unitVectorToReturn = *this;

    // Store the magnitude of the vector in a variable
    float vectorMagnitude = unitVectorToReturn.magnitude();

    // Check if the unit vector can be calculated
    if (vectorMagnitude < 0.00001f)
    {
        // throw std::invalid_argument("Invalid unit vector");
        throw "Invalid unit vector";
    }

    // Calculate the elements of the unit vector
    for (int i = 0; i < n; i++)
    {
        unitVectorToReturn[i] = unitVectorToReturn[i] / vectorMagnitude;
    }

    // Return the unit vector
    return unitVectorToReturn;
}

template <int n>
int Vector<n>::getN() const
{
    // Return template variable n
    return n;
}