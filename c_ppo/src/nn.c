#include "nn.h"
#include <assert.h>
#include <stdlib.h>

NeuralNetwork* nn_create(int num_layers) {
    assert(num_layers > 0);

    NeuralNetwork *nn = (NeuralNetwork *)malloc(sizeof(NeuralNetwork));
    assert(nn != NULL);

    nn->num_layers = num_layers;
    nn->layers = (Layer **)calloc((size_t)num_layers, sizeof(Layer *));
    assert(nn->layers != NULL);

    return nn;
}

void nn_set_layer(NeuralNetwork *nn, int index, Layer *layer) {
    assert(nn != NULL);
    assert(index >= 0 && index < nn->num_layers);
    assert(layer != NULL);

    // Verify adjacent layer dimensions match when wiring sequentially
    if (index > 0 && nn->layers[index - 1] != NULL) {
        assert(nn->layers[index - 1]->out_features == layer->in_features);
    }
    if (index + 1 < nn->num_layers && nn->layers[index + 1] != NULL) {
        assert(layer->out_features == nn->layers[index + 1]->in_features);
    }

    nn->layers[index] = layer;
}

void nn_free(NeuralNetwork *nn) {
    if (nn == NULL) return;

    for (int i = 0; i < nn->num_layers; i++) {
        layer_free(nn->layers[i]);
    }
    free(nn->layers);
    free(nn);
}

Matrix* nn_forward(NeuralNetwork *nn, const Matrix *input) {
    assert(nn != NULL && input != NULL);
    assert(nn->layers[0] != NULL);
    assert(input->rows == nn->layers[0]->in_features);

    // Layer 0 reads directly from caller's input without modifying or freeing it
    Matrix *current = layer_forward(nn->layers[0], input);

    // Relay output from layer i-1 into layer i, freeing intermediate matrices
    for (int i = 1; i < nn->num_layers; i++) {
        assert(nn->layers[i] != NULL);
        Matrix *next = layer_forward(nn->layers[i], current);
        free_matrix(current);
        current = next;
    }

    // Caller owns the final output Matrix* and must free it
    return current;
}

void nn_backward(NeuralNetwork *nn, const Matrix *grad_output) {
    assert(nn != NULL && grad_output != NULL);

    int last_idx = nn->num_layers - 1;
    assert(nn->layers[last_idx] != NULL);
    assert(grad_output->rows == nn->layers[last_idx]->out_features);

    // Copy initial loss gradient so caller's grad_output stays intact
    Matrix *grad = matrix_copy(grad_output);

    // Step backward from output head to input layer
    for (int i = last_idx; i >= 0; i--) {
        assert(nn->layers[i] != NULL);
        Matrix *prev_grad = layer_backward(nn->layers[i], grad);
        free_matrix(grad);
        grad = prev_grad;
    }

    // Free the final dL/dX propagated past layer 0
    free_matrix(grad);
}