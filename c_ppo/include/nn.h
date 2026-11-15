#ifndef NN_H
#define NN_H

#include "layers.h"
#include "matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int num_layers;
    Layer **layers;   // Ordered array of pointers to independent Layer structs
} NeuralNetwork;

// Lifecycle & Wiring
NeuralNetwork* nn_create(int num_layers);
void           nn_set_layer(NeuralNetwork *nn, int index, Layer *layer);
void           nn_free(NeuralNetwork *nn);

// Forward & Backward Relays across all layers
Matrix* nn_forward(NeuralNetwork *nn, const Matrix *input);
void    nn_backward(NeuralNetwork *nn, const Matrix *grad_output);

#ifdef __cplusplus
}
#endif

#endif // NN_H