#!/usr/bin/bash

# @brief script path
AT="${0%/*}"

# @brief number of binaries generated for every number of inverted pixels
BIN_PER_INVERTS="${1:-20}"

# @brief maximum number of inverted pixels in image
MAX_INVERTS="${2:-13}"

# @brief path to primitives
SOURCE="${3:-assets}"

# @brief path to tools
TOOLS="${4:-$(realpath "${AT}/..")}"

BMP2BIN="${TOOLS}/bmp2bin.py"
NOISE="${TOOLS}/noise.py"

# @brief target dir for generated binaries
DATA='data'

if [ -d "${DATA}" ]; then
    echo "${DATA} dir already exists"
    exit 1
fi

if [ -z "$(ls "${SOURCE}")" ]; then
    echo "No generation sources found"
    exit 1
fi
mkdir "${DATA}"

for image in "${SOURCE}"/*; do
    echo "${image}"
    binin="${image/.bmp/.bin}"
    python3 "${BMP2BIN}" -o "${binin}" "${image}"
    for ((i=0; i < BIN_PER_INVERTS; i++)); do
        for ((j=1; j <= MAX_INVERTS; j++)); do
            binout="${DATA}/${i}_${j}_${binin##*/}"
            python3 "${NOISE}" -i "${j}" -o "${binout}" "${binin}"
        done
    done
    rm -f "${binin}"
done
