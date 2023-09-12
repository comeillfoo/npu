#!/usr/bin/bash

# @brief script path
AT="${0%/*}"

# @brief path to primitives
SOURCE="${1:-assets}"

# @brief number of binaries generated per primitive
BIN_PER_IMG="${2:-20}"

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
    binin="${image/.bmp/.bin}"
    python3 "${BMP2BIN}" "${image}" -o "${binin}" # convert to binary
    # TODO: noisify
    for ((i=1; i <= BIN_PER_IMG; i++)); do
        binout="${DATA}/${i}_${binin##*/}"
        # python3 "${NOISE}" "${binin}" --invert="${i}" -o "${binout}"
        cp "${binin}" "${binout}"
    done
    rm -f "${binin}"
done
