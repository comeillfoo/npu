#!/usr/bin/bash

usage()
{
    echo -e "usage: ${0} CONFIG [NEURONS ...]
Options:
    CONFIG  - path to configuration with environment variables
    NEURONS - number of neurons per layer, total HIDDEN_LAYERS values"
}

# @brief path to config
CONFIG=${1}; shift

# @brief name of executable
EXEC='./npu'


if [ -z "${CONFIG}" ]; then
    usage
    exit 22
fi

set -eo pipefail

if [ ! -f "${EXEC}" ]; then
    echo "SKIP: ${EXEC} not found, run make first"
    exit 2
fi

if [ ! -f "${CONFIG}" ]; then
    echo "SKIP: ${CONFIG} not found, use configs/config.sample as example"
    exit 2
fi

source "${CONFIG}"
export ACCURACY_LIMIT TUNE_LIMIT ALPHA DATASET_PATH EPOCH_LIMIT IN_LAYER_SIZE HIDDEN_LAYERS

if [ -n "${DEBUG}" ]; then
    valgrind --tool=massif "${EXEC}" "${@}"
else
    "${EXEC}" "${@}"
fi
