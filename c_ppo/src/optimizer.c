#include "optimizer.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

AdamOptimizer* adam_create(const NeuralNetwork *nn, float lr) {
    assert(nn != NULL && nn->num_layers > 0);
    assert(lr > 0.0f);

    AdamOptimizer *opt = (AdamOptimizer *)malloc(sizeof(AdamOptimizer));
    assert(opt != NULL);

    opt->lr         = lr;
    opt->beta1      = 0.9f;
    opt->beta2      = 0.999f;
    opt->eps        = 1e-5f;   // Matches Stable-Baselines3 PPO default
    opt->t          = 0;
    opt->num_layers = nn->num_layers;

    opt->states = (LayerAdamState *)calloc((size_t)nn->num_layers, sizeof(LayerAdamState));
    assert(opt->states != NULL);

    for (int i = 0; i < nn->num_layers; i++) {
        const Layer *layer = nn->layers[i];
        assert(layer != NULL);

        int rows = layer->out_features;
        int cols = layer->in_features;

        opt->states[i].m_weights = create_matrix(rows, cols);
        opt->states[i].v_weights = create_matrix(rows, cols);
        opt->states[i].m_biases  = create_matrix(rows, 1);
        opt->states[i].v_biases  = create_matrix(rows, 1);
    }

    return opt;
}

void adam_free(AdamOptimizer *opt) {
    if (opt == NULL) return;

    for (int i = 0; i < opt->num_layers; i++) {
        free_matrix(opt->states[i].m_weights);
        free_matrix(opt->states[i].v_weights);
        free_matrix(opt->states[i].m_biases);
        free_matrix(opt->states[i].v_biases);
    }
    free(opt->states);
    free(opt);
}

void adam_zero_grad(NeuralNetwork *nn) {
    assert(nn != NULL);

    for (int i = 0; i < nn->num_layers; i++) {
        Layer *layer = nn->layers[i];
        assert(layer != NULL);
        matrix_zero(layer->grad_weights);
        matrix_zero(layer->grad_biases);
    }
}

float adam_grad_norm_sq(const NeuralNetwork *nn) {
    assert(nn != NULL);

    float norm_sq = 0.0f;
    for (int i = 0; i < nn->num_layers; i++) {
        const Layer *layer = nn->layers[i];
        assert(layer != NULL);

        int w_count = layer->grad_weights->rows * layer->grad_weights->cols;
        for (int j = 0; j < w_count; j++) {
            float g = layer->grad_weights->data[j];
            norm_sq += g * g;
        }

        int b_count = layer->grad_biases->rows;
        for (int j = 0; j < b_count; j++) {
            float g = layer->grad_biases->data[j];
            norm_sq += g * g;
        }
    }
    return norm_sq;
}

void adam_scale_grads(NeuralNetwork *nn, float scale) {
    assert(nn != NULL);

    for (int i = 0; i < nn->num_layers; i++) {
        Layer *layer = nn->layers[i];
        assert(layer != NULL);

        int w_count = layer->grad_weights->rows * layer->grad_weights->cols;
        for (int j = 0; j < w_count; j++) {
            layer->grad_weights->data[j] *= scale;
        }

        int b_count = layer->grad_biases->rows;
        for (int j = 0; j < b_count; j++) {
            layer->grad_biases->data[j] *= scale;
        }
    }
}

static void update_matrix_adam(
    Matrix *param,
    const Matrix *grad,
    Matrix *m,
    Matrix *v,
    float lr,
    float beta1,
    float beta2,
    float eps,
    float bias_corr1,
    float bias_corr2
) {
    int total = param->rows * param->cols;
    for (int i = 0; i < total; i++) {
        float g = grad->data[i];

        // Update biased 1st and 2nd moment estimates
        m->data[i] = beta1 * m->data[i] + (1.0f - beta1) * g;
        v->data[i] = beta2 * v->data[i] + (1.0f - beta2) * (g * g);

        // Bias-corrected estimates
        float m_hat = m->data[i] / bias_corr1;
        float v_hat = v->data[i] / bias_corr2;

        // Parameter update
        param->data[i] -= lr * m_hat / (sqrtf(v_hat) + eps);
    }
}

void adam_step(AdamOptimizer *opt, NeuralNetwork *nn) {
    assert(opt != NULL && nn != NULL);
    assert(opt->num_layers == nn->num_layers);

    opt->t += 1;
    float bias_corr1 = 1.0f - powf(opt->beta1, (float)opt->t);
    float bias_corr2 = 1.0f - powf(opt->beta2, (float)opt->t);

    for (int i = 0; i < nn->num_layers; i++) {
        Layer *layer          = nn->layers[i];
        LayerAdamState *state = &opt->states[i];

        update_matrix_adam(
            layer->weights, layer->grad_weights,
            state->m_weights, state->v_weights,
            opt->lr, opt->beta1, opt->beta2, opt->eps,
            bias_corr1, bias_corr2
        );

        update_matrix_adam(
            layer->biases, layer->grad_biases,
            state->m_biases, state->v_biases,
            opt->lr, opt->beta1, opt->beta2, opt->eps,
            bias_corr1, bias_corr2
        );
    }
}