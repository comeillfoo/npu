#ifndef _DATASET_H_
#define _DATASET_H_

#include <stdint.h>

#define MAX_IN_LAYER_SIZE 60

enum network_constants {
    NP_OUT_LAYER_SIZE=3
};

struct dataset_record {
    double image[MAX_IN_LAYER_SIZE];
    uint8_t target;
};

#endif // _DATASET_H_