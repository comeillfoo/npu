#ifdef DEBUG
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <math.h>

#include <sys/types.h>
#include <dirent.h>

#include <stdint.h>

#include <time.h>

#include "neural_net.h"
#include "dataset.h"


enum network_parameters {
    NP_ACCURACY_LIMIT = 0,
    NP_TUNE_LIMIT,
    NP_ALPHA,
    NP_DATASET_PATH,
    NP_EPOCH_LIMIT,
    NP_HIDDEN_LAYERS
};

enum data_classes {
    DC_CIRCLES = 0,
    DC_SQUARES,
    DC_TRIANGLES,
    DC_CLASSES
};


static size_t neurons_count[MAX_LAYERS] = {0};
static double weights[MAX_LAYERS][MAX_NEURALS_PER_LAYER][MAX_NEURALS_PER_LAYER] = {0};

static const char* param_names[] = {
    [NP_ACCURACY_LIMIT] = "ACCURACY_LIMIT",
    [NP_TUNE_LIMIT] = "TUNE_LIMIT",
    [NP_ALPHA] = "ALPHA",
    [NP_DATASET_PATH] = "DATASET_PATH",
    [NP_EPOCH_LIMIT] = "EPOCH_LIMIT",
    [NP_HIDDEN_LAYERS] = "HIDDEN_LAYERS"
};

static size_t epoch_limit = 0, hidden_layers = 0;
static double accuracy_limit = 0.0, tune_limit = 0.0, alpha = 0.25;
static char* dataset_path = NULL;

#ifdef DEBUG
// record learning time, epoch number, time of output calculation
long output_delay = 0;
#endif


static double rand_range(double min, double max)
{
    const double range = (max - min);
    const double div = RAND_MAX / range;
    return min + rand() / div;
}


static bool set_double_param(enum network_parameters param, double* value_p)
{
    const char* raw_param = getenv(param_names[param]);
    // fprintf(stderr, "%i: %s\n", param, raw_param);
    if (raw_param == NULL) return false;
    errno = 0;
    const double value = strtod(raw_param, NULL);

    if (value == 0.0) return false; // TODO: ccmp, no conversion is performed
    if (errno == ERANGE) return false; // underflow or overflow

    *value_p = value;
    return true;
}


static bool parse_size(const char* raw_size, size_t* value_p)
{
    // fprintf(stderr, "sz: %s\n", raw_size);
    errno = 0;
    const size_t value = strtoul(raw_size, NULL, 10);

    if (value == 0) return false; // no conversion performed
    if (errno == ERANGE) return false; // overflow

    *value_p = value;
    return true;
}


static bool set_size_param(enum network_parameters param, size_t* value_p)
{
    const char* raw_param = getenv(param_names[param]);
    if (raw_param == NULL) return false;
    return parse_size(raw_param, value_p);
}


static bool parse_args(int argc, char** argv)
{
    bool ret = true;
    // parse doubles
    ret &= set_double_param(NP_ACCURACY_LIMIT, &accuracy_limit);
    ret &= set_double_param(NP_TUNE_LIMIT, &tune_limit);
    ret &= set_double_param(NP_ALPHA, &alpha);

    // parse sizes
    ret &= set_size_param(NP_EPOCH_LIMIT, &epoch_limit);

    ret &= set_size_param(NP_HIDDEN_LAYERS, &hidden_layers);
    if (hidden_layers > MAX_LAYERS - 1) return false;

    if (!ret) return false; // all params parsing SUCCEED

    dataset_path = getenv(param_names[NP_DATASET_PATH]);
    // fprintf(stderr, "dataset: %s\n", dataset_path);
    if (dataset_path == NULL) return false;

    if (argc - 1 < hidden_layers) return false;
    for (size_t i = 1; i <= hidden_layers; ++i) {
        if (!parse_size(argv[i], &neurons_count[i - 1]) || neurons_count[i - 1] > MAX_NEURALS_PER_LAYER)
            return false;
    }
    return true;
}

typedef double (*produce_action)(void);

static double rand_weight(void)
{
    return rand_range(-1.0, 1.0); // TODO: generate using normal distribution
}

static void map_weights(size_t input_length, produce_action mapper)
{
    size_t k = 0;
    while (neurons_count[k] > 0 && k < MAX_LAYERS) {
        for (size_t i = 0; i < neurons_count[k]; ++i) {
            size_t previous_layer_length = k == 0? input_length : neurons_count[k - 1];
            for (size_t j = 0; j < previous_layer_length; ++j)
                weights[k][i][j] = mapper();
        }
        k++;
    }
}

typedef void (*consume_action)(double);

static void print_weight(double weight)
{
    printf("%lf ", weight);
}

static void foreach_weights(size_t input_length, consume_action action)
{
    size_t k = 0;
    while (neurons_count[k] > 0 && k < MAX_LAYERS) {
        for (size_t i = 0; i < neurons_count[k]; ++i) {
            size_t previous_layer_length = k == 0? input_length : neurons_count[k - 1];
            for (size_t j = 0; j < previous_layer_length; ++j)
                action(weights[k][i][j]);
        }
        k++;
    }
}


// @param [in] 1: training accuracy limit  - ACCURACY_LIMIT
// @param [in] 2: correction limit         - TUNE_LIMIT
// @param [in] 3: alpha                    - ALPHA
// @param [in] 4: path to dataset          - DATASET_PATH
// @param [in] 5: max epoch limit          - EPOCH_LIMIT
// @param [in] 6: number of hidden layers  - HIDDEN_LAYERS
// @param [in] *: number of neurons in every hidden layer - argv[i], i = 1..=HIDDEN_LAYERS
int main(int argc, char** argv)
{
    if (!parse_args(argc, argv)) return EINVAL;

    srand(time(NULL)); // random seed
    // init weights and sizes
    neurons_count[hidden_layers] = NP_OUT_LAYER_SIZE;
    map_weights(NP_IN_LAYER_SIZE, &rand_weight);

    // build path to classes
    char class_paths[][PATH_MAX] = {
        [DC_CIRCLES] = {0},
        [DC_SQUARES] = {0},
        [DC_TRIANGLES] = {0}
    };
    const size_t dataset_path_len = strlen(dataset_path);
    strncpy(class_paths[DC_CIRCLES], dataset_path, PATH_MAX);
    strncat(class_paths[DC_CIRCLES], "/circles", PATH_MAX - dataset_path_len);

    strncpy(class_paths[DC_SQUARES], dataset_path, PATH_MAX);
    strncat(class_paths[DC_SQUARES], "/squares", PATH_MAX - dataset_path_len);

    strncpy(class_paths[DC_TRIANGLES], dataset_path, PATH_MAX);
    strncat(class_paths[DC_TRIANGLES], "/triangles", PATH_MAX - dataset_path_len);

    // count dataset size
    size_t dataset_size = 0;
    for (size_t i = 0; i < DC_CLASSES; ++i) {
        DIR* dp = opendir(class_paths[i]);
        if (dp != NULL) {
            while (readdir(dp)) ++dataset_size;
            closedir(dp);
        } else return ENOENT;
    }

    // read dataset
    struct dataset_record dataset[dataset_size];
    bool isset[dataset_size];
    memset(isset, false, dataset_size);
    uint8_t imm_pixels[NP_IN_LAYER_SIZE] = {0};
    for (size_t i = 0; i < DC_CLASSES; ++i) {
        const size_t class_path_len = strlen(class_paths[i]);
        DIR* dp = opendir(class_paths[i]);
        if (dp != NULL) {
            struct dirent* ep;
            while ((ep = readdir(dp))) {
                // build path to image
                char image_path[PATH_MAX] = {0};
                strncpy(image_path, class_paths[i], PATH_MAX);
                strncat(image_path, "/", PATH_MAX - class_path_len);
                strncat(image_path, ep->d_name, PATH_MAX - class_path_len - 1);

                // compute idx
                size_t idx = 0;
                do {
                    idx = rand() % dataset_size;
                } while (isset[idx]);
                isset[idx] = true;

                // read image
                FILE* image_file = fopen(image_path, "rb");
                fread(imm_pixels, sizeof(uint8_t), NP_IN_LAYER_SIZE, image_file);
                fclose(image_file);

                for (size_t j = 0; j < NP_IN_LAYER_SIZE; ++j)
                    dataset[idx].image[j] = (double) imm_pixels[j];
                dataset[idx].target = i; // set class
            }
            closedir(dp);
        } else return ENOENT;
    }

    printf("----  PARAMS  ----\n");
    printf("%s: %.2lf\n", param_names[NP_ACCURACY_LIMIT], accuracy_limit);
    printf("%s: %.2lf\n", param_names[NP_TUNE_LIMIT], tune_limit);
    printf("dataset_size: %zu\n", dataset_size);
    printf("%s: %zu\n", param_names[NP_EPOCH_LIMIT], epoch_limit);
    printf("%s: %zu\n", param_names[NP_HIDDEN_LAYERS], hidden_layers);
    printf("classes: [ %s", class_paths[0]);
    for (size_t i = 1; i < DC_CLASSES; ++i)
        printf(", %s", class_paths[i]);
    printf(" ]\n");

    printf("---- INITIAL WEIGHTS ----\n");
    foreach_weights(NP_IN_LAYER_SIZE, &print_weight);
    printf("\n");

    // train
    double accuracy = 0.0;
    size_t epoch = 0;
    #ifdef DEBUG
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    struct timeval learn_start = r.ru_utime;
    #endif
    do {
        accuracy = run_epoch(dataset, dataset_size,
            weights, hidden_layers, neurons_count,
            tune_limit, alpha);
        epoch++;
    } while (accuracy < accuracy_limit
        && epoch < epoch_limit); // TODO: ccmp
    #ifdef DEBUG
    getrusage(RUSAGE_SELF, &r);
    struct timeval learn_end = r.ru_utime;
    printf("learn time: %ldus; avg. output computation delay: %ldus; ",
        ((learn_end.tv_sec - learn_start.tv_sec) * 1000000L) + learn_end.tv_usec - learn_start.tv_usec, output_delay / epoch);
    #endif
    printf("epoch: %zu\n", epoch);
    printf("---- TUNED WEIGHTS ----\n");
    foreach_weights(NP_IN_LAYER_SIZE, &print_weight);
    printf("\n");
    return 0;
}
