#include <iostream>
#include "io.h"
#include "helpers.h"


Io::Io(sc_module_name nm,
    char* _weights_path,
    char** _image_paths,
    size_t _image_count) :
    sc_module(nm),
    DEFINE_PORT(io_clk_i),
    DEFINE_PORT(io_brq_o),
    DEFINE_PORT(io_bgt_i),
    DEFINE_PORT(io_addr_bo),
    DEFINE_PORTVEC(io_data_bi, SYS_RQ_BUS_WIDTH),
    DEFINE_PORTVEC(io_data_bo, SYS_RQ_BUS_WIDTH),
    DEFINE_PORT(io_wr_o),
    DEFINE_PORT(io_rd_o),
    should_stop(false),
    image_count(_image_count)
{
    strncpy(weights_path, _weights_path, PATH_MAX);
    for (size_t i = 0; i < image_count; ++i)
        strncpy(image_paths[i], _image_paths[i], PATH_MAX);

    io_wr_o.initialize(false);
    io_rd_o.initialize(false);
    io_addr_bo.initialize(0);
    io_brq_o.initialize(false);
    for (size_t i = 0; i < io_data_bo.size(); ++i)
        io_data_bo[i].initialize(0);

    state = IOS_STORE_WEIGHTS;

    SC_CTHREAD(io_routine, io_clk_i.pos());
}

Io::~Io()
{
}

void Io::bus_capture()
{
    io_brq_o.write(true);
    while (!io_bgt_i.read()) wait();
}

void Io::bus_release()
{
    io_brq_o.write(false);
    while (io_bgt_i.read()) wait();
}

void Io::bus_write(size_t memory_row, double data[SYS_RQ_BUS_WIDTH])
{
    bus_capture();

    for (size_t i = 0; i < io_data_bo.size(); ++i)
        io_data_bo[i].write(data[i]);
    io_addr_bo.write(memory_row << 9);

    io_wr_o.write(true);
    wait();
    io_wr_o.write(false);
    bus_release();
}

void Io::bus_read(size_t memory_row, double data[SYS_RQ_BUS_WIDTH])
{
    bus_capture();

    io_addr_bo.write(memory_row << 9);

    io_rd_o.write(true);
    wait();
    io_rd_o.write(false);
    wait();

    for (size_t i = 0; i < io_data_bi.size(); ++i)
        data[i] = io_data_bi[i].read();

    bus_release();
}


void Io::store_weights()
{
    std::cout << "$ store_weights " << weights_path << std::endl;

    FILE* fweights = fopen(weights_path, "rb");
    if (!fweights) {
        state = IOS_ERROR;
        return;
    }

    const size_t count = SYS_RQ_LAYER_COUNT * SYS_RQ_MAX_IMAGE_SIZE;
    for (size_t i = 0; i < count; ++i) {
        double weights_per_input[SYS_RQ_MAX_IMAGE_SIZE] = {0.0};

        size_t n = fread(weights_per_input, sizeof(double),
            (sizeof(weights_per_input) / sizeof(double)), fweights);

        if (n < SYS_RQ_MAX_IMAGE_SIZE) {
            state = IOS_ERROR;
            goto err;
        }
        bus_write(i + SYS_RQ_LAYER_COUNT, weights_per_input);
    }

    state = IOS_STORE_IMAGE;
err:
    fclose(fweights);
}

void Io::store_image()
{
    if (image_count == 0) {
        should_stop = true;
        return;
    }
    size_t img = image_count - 1;
    std::cout << "$ store_image " << image_paths[img];

    FILE* fimage = fopen(image_paths[img], "rb");
    if (!fimage) {
        state = IOS_ERROR;
        return;
    }

    double image[SYS_RQ_MAX_IMAGE_SIZE] = {0.0};
    size_t n = fread(image, sizeof(double),
        (sizeof(image) / sizeof(double)), fimage);
    if (n < SYS_RQ_MAX_IMAGE_SIZE) {
        state = IOS_ERROR;
        goto err;
    }
    bus_write(0, image);

    state = IOS_WAIT_RESULT;
err:
    fclose(fimage);
}

void Io::load_result()
{
    for (size_t layer = 1; layer < SYS_RQ_LAYER_COUNT; ++layer) {
        double output[SYS_RQ_BUS_WIDTH] = {0.0};
        bus_read(layer, output);
        std::cout << "y[" << layer << "]: {";
        for (size_t i = 0; i < SYS_RQ_BUS_WIDTH; ++i)
            std::cout << output[i] << ", ";
        std::cout << "}" << std::endl;
    }
}

void Io::io_routine()
{
    while (!should_stop) {
        switch (state) {
            case IOS_STORE_WEIGHTS:
                store_weights(); break;
            case IOS_STORE_IMAGE:
                store_image(); break;
            case IOS_WAIT_RESULT:
                break;
            case IOS_LOAD_RESULT:
                load_result(); break;
            case IOS_ERROR:
            default:
                should_stop = true;
                break;
        }
    }
    sc_stop();
}
