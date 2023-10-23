#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>

#include <errno.h>
#include <limits.h>
#include <string.h>

size_t original_size = 49;
size_t target_size = 64;
char image[PATH_MAX] = {0};
const char* out = "out.bin";


int main(int argc, char** argv)
{
    struct option longopts[] = {
        { "original-size", required_argument, NULL, 'o' },
        { "target-size", required_argument, NULL, 't' },
        { "file", required_argument, NULL, 'f' },
        { 0, 0, 0, 0 }
    };

    char c = -1;
    int longind = 0;
    while ((c = getopt_long(argc, argv, "o:t:f:", longopts, &longind)) != -1) {
        switch (c) {
            case 'o':
                original_size = strtoul(optarg, NULL, 10);
                break;
            case 't':
                target_size = strtoul(optarg, NULL, 10);
            case 'f':
                strncpy(image, optarg, PATH_MAX);
                break;
            default:
                return EINVAL;
        }
    }

    if (original_size > target_size) {
        fprintf(stderr, "Fatal: threat of data loss detected (%zu > %zu)\n", original_size, target_size);
        return EINVAL;
    }

    uint8_t input[original_size];
    memset(input, 0, sizeof(uint8_t) * original_size);

    int ret = 0;
    // input file
    FILE* f = fopen(image, "rb");
    if (!f) {
        perror("Cannot open input file");
        return -1;
    }
    if (fread(input, sizeof(uint8_t), original_size, f) < original_size) {
        perror("Fatal: file corrupted");
        ret = -1;
    }
    if (fclose(f)) {
        perror("Cannot close input file");
        ret = -1;
    }
    if (ret != 0) return ret;

    f = fopen(out, "wb");
    if (!f) {
        perror("Cannot open output file");
        return -1;
    }
    double output[target_size];
    size_t i = 0;
    for (; i < original_size; ++i) output[i] = (double) input[i];
    while (i < target_size) {
        output[i] = 0.0;
        ++i;
    }
    if (fwrite(output, sizeof(double), target_size, f) < target_size) {
        perror("Fatal: write failed");
        ret = -1;
    }
    if (fclose(f)) {
        perror("Cannot close output file");
        ret = -1;
    }

    return ret;
}