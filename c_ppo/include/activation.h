#ifndef ACTIVATION_H
#define ACTIVATION_H

#ifdef __cplusplus
extern "C" {
#endif

// Function pointer types for forward activation and its derivative
typedef float (*ActivationForwardFn)(float z);
typedef float (*ActivationBackwardFn)(float a); // Takes post-activation output a = f(z)

// Hyperbolic Tangent (used in hidden layers)
float act_tanh_forward(float z);
float act_tanh_backward(float a);

// Linear / Identity (used in Actor & Critic output heads)
float act_linear_forward(float z);
float act_linear_backward(float a);

#ifdef __cplusplus
}
#endif

#endif // ACTIVATION_H