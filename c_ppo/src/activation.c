// c_ppo/src/activation.c
#include "activation.h"

float act_tanh_forward(float z) {
    // Explicitly clamp the flat saturation regions
    if (z >= 4.97f) return 1.0f;
    if (z <= -4.97f) return -1.0f;

    // 7th-order Pade rational approximation for the S-curve
    float z2 = z * z;
    float num = z * (135135.0f + z2 * (17325.0f + z2 * (378.0f + z2)));
    float den = 135135.0f + z2 * (62370.0f + z2 * (3150.0f + 28.0f * z2));
    return num / den;
}

float act_tanh_backward(float a) {
    return 1.0f - (a * a);
}

float act_linear_forward(float z) {
    return z;
}

float act_linear_backward(float a) {
    (void)a;
    return 1.0f;
}