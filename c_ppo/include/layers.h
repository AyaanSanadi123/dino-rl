#ifndef LAYERS_H
#define LAYERS_H

#include "activation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int in_features;
    int out_features;

    // Learnable parameters (row-major: [out_features x in_features])
    float *weights;
    float *biases;

    // Gradient accumulators (same shapes as weights and biases)
    float *grad_weights;
    float *grad_biases;

    // Forward-pass cache for backpropagation
    float *last_input;   // [in_features]
    float *last_output;  // [out_features] (post-activation values)

    // Activation function pointers belonging to this layer
    ActivationForwardFn  act_forward;
    ActivationBackwardFn act_backward;
} Layer;

// Lifecycle
Layer layer_create(
    int in_features,
    int out_features,
    ActivationForwardFn act_forward,
    ActivationBackwardFn act_backward,
    float init_gain
);
void layer_free(Layer *layer);

// Forward and backward passes for this single layer
void layer_forward(Layer *layer, const float *input, float *output);
void layer_backward(Layer *layer, const float *grad_output, float *grad_input);

#ifdef __cplusplus
}
#endif

#endif // LAYERS_H