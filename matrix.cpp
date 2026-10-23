#include "vec3.cpp"
#include <cassert>
#include <iostream>
#include <cmath>

template<int nrows, int ncols> 
struct mat {
    vec<ncols> rows[nrows] = {};

    // Row getter
          vec<ncols>& operator[](const int idx)       { return rows[idx]; }
    const vec<ncols>& operator[](const int idx) const { return rows[idx]; }

    // Create identity matrix
    static mat<nrows, ncols> identity() {
        mat<nrows, ncols> m;
        for (int r = 0; r < nrows; r++) {
            for (int c = 0; c < ncols; c++) {
                m[r][c] = (r == c ? 1.0 : 0.0);
            }
        }
        return m;
    }
};

template<int rows, int cols>
void print_matrix(const mat<rows, cols> mat) {
    for (int i = 0; i < rows; i ++) {
        for (int j = 0; j < cols; j ++) {
            std::cout << mat[i][j] << " ";
        }
        std::cout << "\n";
    }
}

// Get the row of the matrix
template<int rows, int cols>
vec<cols> row(int c, mat<rows, cols> matrix) {
    vec<rows> res;
    for (int i = 0; i < cols; i ++) {
        res[i] = matrix[c][i];
    }
    return res;
}

template<int rows, int cols>
mat<rows, cols> scalar_mul(mat<rows, cols> matrix, double val) {
    mat<rows, cols> res;
    for (int i = 0; i < rows; i ++) {
        for (int j = 0; j < cols; j ++) {
            res[i][j] = matrix[i][j] * val;
        }
    }
    return res;
}

// Alias
typedef mat<3,3> mat3;
typedef mat<4,4> mat4;

template<int R, int C>
vec<R> operator*(const mat<R, C>& m, const vec<C>& v) {
    vec<R> res;
    for (int r = 0; r < R; r ++) {
        res[r] = dot(m[r], v);
    }
    return res;
}

template<int R, int K, int C>
mat<R, C> operator*(const mat<R, K>& left, const mat<K,C>& right) {
    mat<R, C> res;
    float sum;
    // Iterate through left rows
    for (int i = 0; i < R; i ++) {

        // Iterate through right columns
        for (int k = 0; k < C; k ++) {
            sum = 0.0;

            // Iterate through all the rows on left and columns on right
            for (int t = 0; t < K; t ++) {
                sum += left[i][t] * right[t][k];
            }
            res[i][k] = sum;
        }
    }
    return res;
}

template<int row, int col>
mat<col, row> transpose(const mat<row, col> matrix) {
    mat<col, row> res;

    for (int i = 0; i < row; i ++) {
        for (int j = 0; j < col; j ++) {
            res[j][i] = matrix[i][j];
        }
    }
    return res;
}

// ------ Matrix Inversion -----

template<int R, int C>
mat<R - 1, C - 1> submatrix(const mat<R, C>& matrix, int row, int col) {
    mat<R - 1, C - 1> res;
    int current_row = 0;
    int current_col = 0;

    for (int i = 0; i < R - 1; i ++) {
        current_col = 0;

        if (current_row == row) {
            current_row += 1;
        }

        for (int j = 0; j < C - 1; j ++) {
            if (current_col == col) {
                current_col += 1;
            }

            res[i][j] = matrix[current_row][current_col];
            current_col += 1;
        }
        current_row += 1;
    }
    return res;
}


// Recursive determinant
template<int R>
double determinant(const mat<R, R>& matrix) {
    
    double running_sum = 0.0;
    
    for (int j = 0; j < R; j ++) {
        mat<R - 1, R - 1> sub = submatrix(matrix, 0, j);
        
        double sub_det = determinant(sub);
        
        int sign = (j%2 == 0) ? 1.0 : -1.0;
        
        running_sum += sign * matrix[0][j] * sub_det;
    }
    return running_sum;
}

template<>
double determinant(const mat<2,2>& matrix) {
    // Base case (2x2 matrix)
    return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
}
template <int R>
mat<R, R> get_cofactor(const mat<R, R>& matrix) {
    mat<R,R> res;

    for (int i = 0; i < R; i ++) {
        for (int j = 0; j < R; j ++) {
            mat<R - 1, R - 1> sub = submatrix(matrix, i, j);
            double minor = determinant(sub);
            int sign = ((i+j)%2 == 0) ? 1.0 : -1.0;
            res[i][j] = sign * minor;
        }
    }
    return res;
}

template<int R>
mat<R, R> adjoint(const mat<R, R>& matrix) {
    return transpose(get_cofactor(matrix));
}

template<int R>
mat<R, R> inverse(const mat<R, R>& matrix) {
    const double eps = 1e-9;
    double det = determinant(matrix);

    if (std::abs(det) < eps) {
        throw "Inverse does not exist";
    }

    det = 1.0 / determinant(matrix);

    return scalar_mul(adjoint(matrix), det);
}

// ---------------
//      TEST
// ---------------

// For individual doubles (c[0][0], dot products, scalar outputs)
bool approx_equal(double a, double b, double eps = 1e-5) {
    return std::abs(a - b) < eps;
}

// For whole vectors (vec3, vec4, etc.)
template<int n>
bool approx_equal(const vec<n>& a, const vec<n>& b, double eps = 1e-5) {
    for (int i = 0; i < n; i++) {
        if (!approx_equal(a[i], b[i], eps)) return false;
    }
    return true;
}

// For whole matrices
template<int R, int C>
bool approx_equal(const mat<R, C>& a, const mat<R, C>& b, double eps = 1e-5) {
    for (int r = 0; r < R; r++) {
        if (!approx_equal(a[r], b[r], eps)) return false;
    }
    return true;
}

void test_matrix_multiplication() {
    mat<2, 2> a, b;
    mat3 e;
    a[0][0] = 1; a[0][1] = 2;
    a[1][0] = 3; a[1][1] = 4;

    b[0][0] = 2; b[0][1] = 0;
    b[1][0] = 1; b[1][1] = 2;

    e[0][0] = 1; e[0][1] = 3; e[0][2] = 1;
    e[1][0] = 0; e[1][1] = 5; e[1][2] = 7;
    e[2][0] = 4; e[2][1] = 2; e[2][2] = 3;

    mat<2, 2> c = a * b;

    assert(approx_equal(c[0][0], 4.0));
    assert(approx_equal(c[0][1], 4.0));
    assert(approx_equal(c[1][0], 10.0));
    assert(approx_equal(c[1][1], 8.0));
    std::cout << "[PASS] Matrix multiplication\n";

    mat<2,2> d = transpose(a);
    print_matrix(d);

    print_matrix(e);
    e = inverse(e);
    print_matrix(e);
    e = inverse(e);
    print_matrix(e);
}

// int main() {
//     test_matrix_multiplication();
// }