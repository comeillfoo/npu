#ifndef _NEURAL_NET_H_
#define _NEURAL_NET_H_

#include <stddef.h>

#define MAX_LAYERS 20
#define MAX_NEURALS_PER_LAYER 20

double sigmoid(double x);

double activate(
    double previous_layer[], size_t previous_layer_length,
    double weight);

void apply(
    double previous_layer[], size_t previous_layer_length,
    double weights_layer[MAX_NEURALS_PER_LAYER],
    double output[], size_t layer_length);

void classify(double input[], size_t input_length,
    size_t hidden_layers, double weights[MAX_LAYERS][MAX_NEURALS_PER_LAYER],
    size_t neurons_count[MAX_LAYERS], double output[]);

#endif // _NEURAL_NET_H_
