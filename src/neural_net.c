#include "neural_net.h"

#include <math.h>
#include <stdlib.h>


double sigmoid(double x)
{
    return 1 / (1 + exp(-x));
}

double activate(
    double previous_layer[], size_t previous_layer_length,
    double weight)
{
    double s = 0.0;
    for (size_t j = 0; j < previous_layer_length; ++j)
        s += previous_layer[j] * weight;
    return sigmoid(s);
}

void apply(
    double previous_layer[], size_t previous_layer_length,
    double weights_layer[MAX_NEURALS_PER_LAYER],
    double output[], size_t layer_length)
{
    for (size_t i = 0; i < layer_length; ++i)
        output[i] = activate(previous_layer, previous_layer_length, weights_layer[i]);
}

void classify(double input[], size_t input_length,
    size_t hidden_layers, double weights[MAX_LAYERS][MAX_NEURALS_PER_LAYER],
    size_t neurons_count[MAX_LAYERS], double output[])
{
    size_t previous_layer_length = input_length;
    double* previous_layer = malloc(sizeof(double) * previous_layer_length);
    for (size_t i = 0; i < input_length; ++i)
        previous_layer[i] = input[i];

    for (int k = 0; k < hidden_layers; ++k) {
        double current_layer[neurons_count[k]];

        apply(
            previous_layer, previous_layer_length,
            weights[k], current_layer, neurons_count[k]);

        free(previous_layer);

        previous_layer_length = neurons_count[k];
        previous_layer = malloc(sizeof(double) * previous_layer_length);
        for (size_t i = 0; i < previous_layer_length; ++i)
            previous_layer[i] = current_layer[i];
    }

    for (size_t i = 0; i < neurons_count[hidden_layers]; ++i)
        output[i] = previous_layer[i];
    free(previous_layer);
}
