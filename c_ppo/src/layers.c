#include "layers.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#define PI_F 3.14159265358979323846f

static float rand_normal(void) {
    float u1 = ((float)rand() + 1.0f) / ((float)RAND_MAX + 1.0f);
    float u2 = ((float)rand() + 1.0f) / ((float)RAND_MAX + 1.0f);
    return sqrtf(-2.0f * logf(u1)) * cosf(2.0f * PI_F * u2);
}

static void init_orthogonal_weights(Matrix *W, float gain) {
    int rows = W->rows;
    int cols = W->cols;
    float *data = W->data;

    for (int i = 0; i < rows * cols; i++) {
        data[i] = rand_normal();
    }

    if (rows <= cols) {
        for (int i = 0; i < rows; i++) {
            float *row_i = &data[i * cols];
            for (int k = 0; k < i; k++) {
                float *row_k = &data[k * cols];
                float dot = 0.0f;
                for (int j = 0; j < cols; j++) dot += row_i[j] * row_k[j];
                for (int j = 0; j < cols; j++) row_i[j] -= dot * row_k[j];
            }
            float norm = 0.0f;
            for (int j = 0; j < cols; j++) norm += row_i[j] * row_i[j];
            norm = sqrtf(norm) + 1e-8f;
            for (int j = 0; j < cols; j++) row_i[j] /= norm;
        }
    } else {
        for (int j = 0; j < cols; j++) {
            for (int k = 0; k < j; k++) {
                float dot = 0.0f;
                for (int i = 0; i < rows; i++) dot += data[i * cols + j] * data[i * cols + k];
                for (int i = 0; i < rows; i++) data[i * cols + j] -= dot * data[i * cols + k];
            }
            float norm = 0.0f;
            for (int i = 0; i < rows; i++) norm += data[i * cols + j] * data[i * cols + j];
            norm = sqrtf(norm) + 1e-8f;
            for (int i = 0; i < rows; i++) data[i * cols + j] /= norm;
        }
    }

    for (int i = 0; i < rows * cols; i++) {
        data[i] *= gain;
    }
}

Layer* layer_create(
    int in_features,
    int out_features,
    ActivationForwardFn act_forward,
    ActivationBackwardFn act_backward,
    float init_gain
) {
    assert(in_features > 0 && out_features > 0);
    assert(act_forward != NULL && act_backward != NULL);

    Layer *layer = (Layer *)malloc(sizeof(Layer));
    assert(layer != NULL);

    layer->in_features  = in_features;
    layer->out_features = out_features;
    layer->act_forward  = act_forward;
    layer->act_backward = act_backward;

    layer->weights      = create_matrix(out_features, in_features);
    layer->biases       = create_matrix(out_features, 1);
    layer->grad_weights = create_matrix(out_features, in_features);
    layer->grad_biases  = create_matrix(out_features, 1);
    layer->last_input   = NULL;
    layer->last_output  = NULL;

    init_orthogonal_weights(layer->weights, init_gain);
    return layer;
}

void layer_free(Layer *layer) {
    if (!layer) return;
    free_matrix(layer->weights);
    free_matrix(layer->biases);
    free_matrix(layer->grad_weights);
    free_matrix(layer->grad_biases);
    free_matrix(layer->last_input);
    free_matrix(layer->last_output);
    free(layer);
}

Matrix* layer_forward(Layer *layer, const Matrix *input) {
    assert(layer != NULL && input != NULL);
    assert(input->rows == layer->in_features);

    // 1. Replace cached input X for backpropagation
    free_matrix(layer->last_input);
    layer->last_input = matrix_copy(input);

    // 2. Z = W * X + b
    Matrix *wx = dot_product(layer->weights, input);
    Matrix *z  = matrix_add_bias(wx, layer->biases);
    free_matrix(wx);

    // 3. A = f(Z) and cache A
    Matrix *a = matrix_map(z, layer->act_forward);
    free_matrix(z);

    free_matrix(layer->last_output);
    layer->last_output = matrix_copy(a);

    return a;
}

Matrix* layer_backward(Layer *layer, const Matrix *grad_output) {
    assert(layer != NULL && grad_output != NULL);
    assert(layer->last_input != NULL && layer->last_output != NULL);

    // 1. dz = dL/dA (hadamard) f'(A)
    Matrix *act_deriv = matrix_map(layer->last_output, layer->act_backward);
    Matrix *dz        = hadamard_product(grad_output, act_deriv);
    free_matrix(act_deriv);

    // 2. Accumulate bias gradients: db += sum_columns(dz)
    Matrix *db = matrix_sum_columns(dz);
    matrix_add_inplace(layer->grad_biases, db);
    free_matrix(db);

    // 3. Accumulate weight gradients: dW += dz * X^T
    Matrix *x_t = transpose(layer->last_input);
    Matrix *dw  = dot_product(dz, x_t);
    matrix_add_inplace(layer->grad_weights, dw);
    free_matrix(x_t);
    free_matrix(dw);

    // 4. Propagate gradient to previous layer: dX = W^T * dz
    Matrix *w_t = transpose(layer->weights);
    Matrix *dx  = dot_product(w_t, dz);
    free_matrix(w_t);
    free_matrix(dz);

    return dx;
}