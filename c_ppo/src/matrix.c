#include "matrix.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

Matrix* create_matrix(int rows, int cols) {
    assert(rows > 0 && cols > 0);

    Matrix *m = (Matrix *)malloc(sizeof(Matrix));
    assert(m != NULL);

    m->rows = rows;
    m->cols = cols;
    m->data = (float *)calloc((size_t)rows * (size_t)cols, sizeof(float));
    assert(m->data != NULL);

    return m;
}

void free_matrix(Matrix *m) {
    if (m != NULL) {
        free(m->data);
        free(m);
    }
}

Matrix* matrix_copy(const Matrix *m) {
    assert(m != NULL);

    Matrix *copy = create_matrix(m->rows, m->cols);
    size_t bytes = (size_t)m->rows * (size_t)m->cols * sizeof(float);
    memcpy(copy->data, m->data, bytes);
    return copy;
}

void matrix_zero(Matrix *m) {
    assert(m != NULL);
    size_t bytes = (size_t)m->rows * (size_t)m->cols * sizeof(float);
    memset(m->data, 0, bytes);
}

Matrix* dot_product(const Matrix *a, const Matrix *b) {
    assert(a != NULL && b != NULL);
    assert(a->cols == b->rows);

    Matrix *c = create_matrix(a->rows, b->cols);

    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            float sum = 0.0f;
            for (int k = 0; k < a->cols; k++) {
                sum += a->data[i * a->cols + k] * b->data[k * b->cols + j];
            }
            c->data[i * c->cols + j] = sum;
        }
    }
    return c;
}

Matrix* transpose(const Matrix *m) {
    assert(m != NULL);

    Matrix *t = create_matrix(m->cols, m->rows);
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            t->data[j * t->cols + i] = m->data[i * m->cols + j];
        }
    }
    return t;
}

Matrix* hadamard_product(const Matrix *a, const Matrix *b) {
    assert(a != NULL && b != NULL);
    assert(a->rows == b->rows && a->cols == b->cols);

    Matrix *c = create_matrix(a->rows, a->cols);
    int total_elements = a->rows * a->cols;
    for (int i = 0; i < total_elements; i++) {
        c->data[i] = a->data[i] * b->data[i];
    }
    return c;
}

Matrix* matrix_add(const Matrix *a, const Matrix *b) {
    assert(a != NULL && b != NULL);
    assert(a->rows == b->rows && a->cols == b->cols);

    Matrix *c = create_matrix(a->rows, a->cols);
    int total_elements = a->rows * a->cols;
    for (int i = 0; i < total_elements; i++) {
        c->data[i] = a->data[i] + b->data[i];
    }
    return c;
}

void matrix_add_inplace(Matrix *target, const Matrix *source) {
    assert(target != NULL && source != NULL);
    assert(target->rows == source->rows && target->cols == source->cols);

    int total_elements = target->rows * target->cols;
    for (int i = 0; i < total_elements; i++) {
        target->data[i] += source->data[i];
    }
}

Matrix* matrix_subtract(const Matrix *a, const Matrix *b) {
    assert(a != NULL && b != NULL);
    assert(a->rows == b->rows && a->cols == b->cols);

    Matrix *c = create_matrix(a->rows, a->cols);
    int total_elements = a->rows * a->cols;
    for (int i = 0; i < total_elements; i++) {
        c->data[i] = a->data[i] - b->data[i];
    }
    return c;
}

Matrix* matrix_multiply_scalar(const Matrix *m, float scalar) {
    assert(m != NULL);

    Matrix *result = create_matrix(m->rows, m->cols);
    int total_elements = m->rows * m->cols;
    for (int i = 0; i < total_elements; i++) {
        result->data[i] = m->data[i] * scalar;
    }
    return result;
}

Matrix* matrix_map(const Matrix *m, float (*func)(float)) {
    assert(m != NULL && func != NULL);

    Matrix *result = create_matrix(m->rows, m->cols);
    int total_elements = m->rows * m->cols;
    for (int i = 0; i < total_elements; i++) {
        result->data[i] = func(m->data[i]);
    }
    return result;
}

Matrix* matrix_add_bias(const Matrix *wx, const Matrix *bias) {
    assert(wx != NULL && bias != NULL);
    assert(wx->rows == bias->rows && bias->cols == 1);

    Matrix *results = create_matrix(wx->rows, wx->cols);
    for (int i = 0; i < wx->rows; i++) {
        float bias_value = bias->data[i];
        for (int j = 0; j < wx->cols; j++) {
            results->data[i * wx->cols + j] = wx->data[i * wx->cols + j] + bias_value;
        }
    }
    return results;
}

Matrix* matrix_sum_columns(const Matrix *m) {
    assert(m != NULL);

    Matrix *result = create_matrix(m->rows, 1);
    for (int i = 0; i < m->rows; i++) {
        float row_sum = 0.0f;
        for (int j = 0; j < m->cols; j++) {
            row_sum += m->data[i * m->cols + j];
        }
        result->data[i] = row_sum;
    }
    return result;
}