#ifndef _NEURAL_NET_H_
#define _NEURAL_NET_H_

#include <stddef.h>
#include "dataset.h"

#define MAX_LAYERS 20
#define MAX_NEURALS_PER_LAYER 20

double sigmoid(double x);

double activate(
    double previous_layer[], size_t previous_layer_length,
    double input_weights[MAX_NEURALS_PER_LAYER]);

void apply(
    double previous_layer[], size_t previous_layer_length,
    double weights_layer[MAX_NEURALS_PER_LAYER][MAX_NEURALS_PER_LAYER],
    double output[], size_t layer_length);

void classify(double input[], size_t input_length,
    size_t hidden_layers, double weights[MAX_LAYERS][MAX_NEURALS_PER_LAYER][MAX_NEURALS_PER_LAYER],
    size_t neurons_count[MAX_LAYERS], double output[MAX_LAYERS][MAX_NEURALS_PER_LAYER]);

void class2probabilities(uint8_t target, size_t classes, double targets[]);

double layer_error(double targets[], size_t classes, double outputs[MAX_NEURALS_PER_LAYER]);

double run_epoch(struct dataset_record dataset[], size_t dataset_size,
    double weights[MAX_LAYERS][MAX_NEURALS_PER_LAYER][MAX_NEURALS_PER_LAYER], size_t hidden_layers,
    size_t neurons_count[MAX_LAYERS], double tune_limit, double alpha);

#endif // _NEURAL_NET_H_
