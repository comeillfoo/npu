#include "neural_net.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#ifdef DEBUG
#include <sys/time.h>
#include <sys/resource.h>
extern long output_delay;
#endif

double sigmoid(double x)
{
    return 1 / (1 + exp(-x));
}

double activate(
    double previous_layer[], size_t previous_layer_length,
    double input_weights[MAX_NEURALS_PER_LAYER])
{
    double s = 0.0;
    for (size_t j = 0; j < previous_layer_length; ++j)
        s += previous_layer[j] * input_weights[j];
    return sigmoid(s);
}

void apply(
    double previous_layer[], size_t previous_layer_length,
    double weights_layer[MAX_NEURALS_PER_LAYER][MAX_NEURALS_PER_LAYER],
    double output[], size_t layer_length)
{
    for (size_t i = 0; i < layer_length; ++i)
        output[i] = activate(previous_layer, previous_layer_length, weights_layer[i]);
}

void classify(double input[], size_t input_length,
    size_t hidden_layers, double weights[MAX_LAYERS][MAX_NEURALS_PER_LAYER][MAX_NEURALS_PER_LAYER],
    size_t neurons_count[MAX_LAYERS], double output[MAX_LAYERS][MAX_NEURALS_PER_LAYER])
{
    #ifdef DEBUG
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    struct timeval output_start = r.ru_utime;
    #endif
    for (size_t k = 0; k <= hidden_layers; ++k) {
        double* previous_layer = k == 0? input : output[k - 1];
        size_t previous_layer_length = k == 0? input_length : neurons_count[k - 1];
        apply(
            previous_layer, previous_layer_length,
            weights[k], output[k], neurons_count[k]);
    }
    #ifdef DEBUG
    getrusage(RUSAGE_SELF, &r);
    struct timeval output_end = r.ru_utime;
    output_delay += ((output_end.tv_sec - output_start.tv_sec) * 1000000L) + output_end.tv_usec - output_start.tv_usec;
    #endif
}

void class2probabilities(uint8_t target, size_t classes, double targets[])
{
    for (size_t i = 0; i < classes; ++i) targets[i] = 0.0;
    targets[target] = 1.0;
}

double layer_error(double targets[], size_t classes, double output[MAX_NEURALS_PER_LAYER])
{
    double s = 0.0;
    for (size_t i = 0; i < classes; ++i)
        s += targets[i] - output[i];
    return s / 2;
}

void tune_weights(double input[],
    double outputs[MAX_LAYERS][MAX_NEURALS_PER_LAYER],
    double weights[MAX_LAYERS][MAX_NEURALS_PER_LAYER][MAX_NEURALS_PER_LAYER],
    size_t neurons_count[MAX_LAYERS], double targets[],
    size_t input_length, size_t N, size_t classes, double alpha)
{
    // https://ai.stackexchange.com/questions/26136/why-are-the-weights-of-the-previous-layers-updated-only-considering-the-old-valu
    double new_weights[MAX_LAYERS][MAX_NEURALS_PER_LAYER][MAX_NEURALS_PER_LAYER] = {{{0.0}}};

    size_t previous_deltas_length = neurons_count[N];
    double* previous_deltas = malloc(sizeof(double) * previous_deltas_length);

    for (ssize_t k = N; k >= 0; --k) {
        double current_deltas[neurons_count[k]];

        // y^{k - 1}_j
        const size_t y_prev_length = k == 0? input_length : neurons_count[k - 1];
        double* y_prev = k == 0? input : outputs[k - 1];
        for (size_t i = 0; i < neurons_count[k]; ++i) {
            const double y_k = outputs[k][i];

            // handle output layer
            if (k == N) {
                current_deltas[i] = y_k * (1 - y_k) * (targets[i] - y_k);
                // compute new weights
                for (size_t j = 0; j < y_prev_length; ++j) {
                    const double delta_weight = alpha * current_deltas[i] * y_prev[j];
                    new_weights[k][i][j] = weights[k][i][j] + delta_weight;
                }
                continue;
            }

            // handle hidden layers
            double s = 0.0;
            for (size_t j = 0; j < neurons_count[k + 1]; ++j)
                s += previous_deltas[j] * weights[k + 1][i][j];

            current_deltas[i] = y_k * (1 - y_k) * s;
            // compute new weights
            for (size_t j = 0; j < y_prev_length; ++j) {
                const double delta_weight = alpha * current_deltas[i] * y_prev[j];
                new_weights[k][i][j] = weights[k][i][j] + delta_weight;
            }
        }

        // realloc deltas
        free(previous_deltas);
        previous_deltas_length = neurons_count[k];
        previous_deltas = malloc(sizeof(double) * previous_deltas_length);
        memcpy(previous_deltas, current_deltas, previous_deltas_length);
    }
    free(previous_deltas);

    // update weights
    for (size_t k = 0; k < MAX_LAYERS; ++k)
        for (size_t i = 0; i < MAX_NEURALS_PER_LAYER; ++i)
            for (size_t j = 0; j < MAX_NEURALS_PER_LAYER; ++j)
                weights[k][i][j] = new_weights[k][i][j];
}

// @return accuracy
double run_epoch(struct dataset_record dataset[], size_t dataset_size,
    double weights[MAX_LAYERS][MAX_NEURALS_PER_LAYER][MAX_NEURALS_PER_LAYER], size_t hidden_layers,
    size_t neurons_count[MAX_LAYERS], double tune_limit, double alpha)
{
    double accuracy = DBL_MAX;
    size_t classes = neurons_count[hidden_layers];
    for (size_t i = 0; i < dataset_size; ++i) {
        double outputs[MAX_LAYERS][MAX_NEURALS_PER_LAYER] = {{0.0}};
        double targets[classes];
        const size_t image_size = sizeof(dataset[i].image);

        classify(dataset[i].image, image_size,
            hidden_layers, weights, neurons_count, outputs);

        class2probabilities(dataset[i].target, classes, targets);
        double e = layer_error(targets, classes, outputs[hidden_layers]);
        e = e < 0.0? -e: e; // TODO: ccmp
        if (e > tune_limit) { // TODO: ccmp
            // printf("tuning weights\n");
            tune_weights(dataset[i].image, outputs, weights, neurons_count, targets,
                image_size, hidden_layers, classes, alpha);
        }

        if (1.0 - e < accuracy) { accuracy = 1.0 - e; } // TODO: ccmp
    }
    return accuracy;
}
