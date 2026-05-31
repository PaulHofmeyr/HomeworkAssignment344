#include "Matrix.h"

template <int n, int m>
Matrix<n, m>::Matrix()
{
    // Ininitalizing the array to a size of n x m
    this->arr = new float *[n];

    for (int i = 0; i < n; i++)
    {
        this->arr[i] = new float[m];
    }

    // Populating with default values
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            this->arr[i][j] = 0.0f;
        }
    }
}

template <int n, int m>
Matrix<n, m>::Matrix(float **inArr)
{
    // Create a shallow copy of the passed in parameter
    this->arr = inArr;
}

template <int n, int m>
Matrix<n, m>::Matrix(const Matrix<n, m> &inMatrix)
{
    // Ininitalizing the array to a size of n x m
    this->arr = new float *[n];

    for (int i = 0; i < n; i++)
    {
        this->arr[i] = new float[m];
    }

    // Creating a deep copy of inMatrix's data
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            this->arr[i][j] = inMatrix[i][j];
        }
    }
}

template <int n, int m>
Matrix<n, m>::~Matrix()
{
    // Delete dynamically allocated memory
    for (int i = 0; i < n; i++)
    {
        delete[] this->arr[i];
    }

    delete[] this->arr;
    this->arr = nullptr;
}

template <int n, int m>
Matrix<n, m> &Matrix<n, m>::operator=(const Matrix<n, m> &otherMatrix)
{
    // Allocate memory
    this->arr = new float *[n];
    for (int i = 0; i < n; i++)
    {
        this->arr[i] = new float[m];
    }

    // Create a deep copy of the arrays
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            arr[i][j] = otherMatrix[i][j];
        }
    }

    // Return this
    return *this;
}

template <int n, int m>
template <int a>
Matrix<n, a> Matrix<n, m>::operator*(const Matrix<m, a> otherMatrix) const
{
    // Create the matrix to return
    Matrix<n, a> matrixToReturn;

    // Iterate over n
    for (int i = 0; i < n; i++)
    {
        // Iterate over a
        for (int j = 0; j < a; j++)
        {
            // Create a sum variable
            float multiplicationValue = 0;

            // Iterate over m
            for (int k = 0; k < m; k++)
            {
                // Calculate the dot products
                multiplicationValue += this->arr[i][k] * otherMatrix[k][j];
            }

            // Store the sum value of the dot products in n x a
            matrixToReturn[i][j] = multiplicationValue;
        }
    }

    // Return the final matrix
    return matrixToReturn;
}

template <int n, int m>
Matrix<n, m> Matrix<n, m>::operator*(const float scalar) const
{
    // Create a matrix that can be scaled
    Matrix<n, m> matrixToReturn = *this;

    // Scale the matrix
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            matrixToReturn[i][j] = matrixToReturn[i][j] * scalar;
        }
    }

    // Return scaled matrix
    return matrixToReturn;
}

template <int n, int m>
Matrix<n, m> Matrix<n, m>::operator+(const Matrix<n, m> otherMatrix) const
{
    // Create a matrix to return
    Matrix<n, m> matrixToReturn = *this;

    // Iterate throught the matrices and insert the added values into the new matrix
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            matrixToReturn[i][j] = this->arr[i][j] + otherMatrix[i][j];
        }
    }

    // Return the summed matrix
    return matrixToReturn;
}

template <int n, int m>
Matrix<m, n> Matrix<n, m>::operator~() const
{
    // Create a new matrix
    Matrix<m, n> matrixToReturn;

    // Transpose the matrix
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            matrixToReturn[j][i] = arr[i][j];
        }
    }

    // Return the transposed matrix
    return matrixToReturn;
}

template <int n, int m>
int Matrix<n, m>::getM() const
{
    // Return template variable m
    return m;
}

template <int n, int m>
int Matrix<n, m>::getN() const
{
    // Return template variable n
    return n;
}

template <int n, int m>
float Matrix<n, m>::determinant() const
{
    // Create a temporary matrix that can be altered
    Matrix<n, m> tempMatrix = *this;

    // Check if the matrix is square
    if (tempMatrix.getM() != tempMatrix.getN())
    {
        throw "Matrix is not square";
    }

    // Create a counter to track the number of swaps
    int numSwaps = 0;

    // Iterate through the columns
    for (int i = 0; i < n; i++)
    {
        // Create variables to store the max and the row to swap
        int max = tempMatrix[i][i];
        int rowToSwap = i;

        // Determine row and max
        for (int j = i; j < n; j++)
        {
            if (abs(tempMatrix[j][i]) > abs(max))
            {
                max = tempMatrix[j][i];
                rowToSwap = j;
            }
        }

        // Check if a swap is necessary
        if (rowToSwap != i)
        {
            // Perform the swap
            for (int k = 0; k < m; k++)
            {
                float swap = tempMatrix[i][k];
                tempMatrix[i][k] = tempMatrix[rowToSwap][k];
                tempMatrix[rowToSwap][k] = swap;
            }

            // Record a swap occurred
            ++numSwaps;
        }

        // Check if the matrix is singular
        if (abs(tempMatrix[i][i]) < 0.0000001f)
        {
            return 0;
        }

        // Eliminate the rows below the current one
        for (int p = i + 1; p < n; p++)
        {
            // Calculate the factor
            float factor = tempMatrix[p][i] / tempMatrix[i][i];

            // Apply factor to the entire row
            for (int f = i; f < m; f++)
            {
                tempMatrix[p][f] -= factor * tempMatrix[i][f];
            }
        }
    }

    // Create a variable to store the determinant
    float determinant = 1.0f;

    // Multiply the diagonal to calculate the determinant
    for (int r = 0; r < n; r++)
    {
        determinant *= tempMatrix[r][r];
    }

    // Perform the sign flip if an odd number of swaps occurred
    if ((numSwaps % 2) == 1)
    {
        determinant = -1 * determinant;
    }

    // Return the final determinant
    return determinant;
}