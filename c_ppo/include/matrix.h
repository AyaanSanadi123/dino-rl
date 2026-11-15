#ifndef MATRIX_H
#define MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int rows;
    int cols;
    float *data;
} Matrix;

// Lifecycle
Matrix* create_matrix(int rows, int cols);
void    free_matrix(Matrix *m);
Matrix* matrix_copy(const Matrix *m);
void    matrix_zero(Matrix *m);

// Core Matrix Operations
Matrix* dot_product(const Matrix *a, const Matrix *b);
Matrix* transpose(const Matrix *m);
Matrix* hadamard_product(const Matrix *a, const Matrix *b);

// Element-wise & Scalar Operations
Matrix* matrix_add(const Matrix *a, const Matrix *b);
void    matrix_add_inplace(Matrix *target, const Matrix *source);
Matrix* matrix_subtract(const Matrix *a, const Matrix *b);
Matrix* matrix_multiply_scalar(const Matrix *m, float scalar);
Matrix* matrix_map(const Matrix *m, float (*func)(float));

// Neural Network Broadcast & Reduction Helpers
Matrix* matrix_add_bias(const Matrix *wx, const Matrix *bias);
Matrix* matrix_sum_columns(const Matrix *m);

#ifdef __cplusplus
}
#endif

#endif // MATRIX_H