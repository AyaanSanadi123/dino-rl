#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "nn.h"
#include "matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    Matrix *m_weights;   // 1st moment (mean) for weights
    Matrix *v_weights;   // 2nd moment (uncentered variance) for weights
    Matrix *m_biases;    // 1st moment (mean) for biases
    Matrix *v_biases;    // 2nd moment (uncentered variance) for biases
} LayerAdamState;

typedef struct {
    float lr;
    float beta1;
    float beta2;
    float eps;
    int   t;             // Timestep counter for bias correction

    int num_layers;
    LayerAdamState *states;
} AdamOptimizer;

// Lifecycle
AdamOptimizer* adam_create(const NeuralNetwork *nn, float lr);
void           adam_free(AdamOptimizer *opt);

// Gradient Utilities (Zeroing & Global Norm Clipping)
void  adam_zero_grad(NeuralNetwork *nn);
float adam_grad_norm_sq(const NeuralNetwork *nn);
void  adam_scale_grads(NeuralNetwork *nn, float scale);

// Parameter Update Step
void adam_step(AdamOptimizer *opt, NeuralNetwork *nn);

#ifdef __cplusplus
}
#endif

#endif // OPTIMIZER_H