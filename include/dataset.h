#ifndef _DATASET_H_
#define _DATASET_H_

#include <stdint.h>

enum network_constants {
    NP_IN_LAYER_SIZE = 49,
    NP_OUT_LAYER_SIZE = 3
};

struct dataset_record {
    double image[NP_IN_LAYER_SIZE];
    uint8_t target;
};

#endif // _DATASET_H_