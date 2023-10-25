#include <iostream>
#include <random>
#include "io.h"
#include "helpers.h"


Io::Io(sc_module_name nm,
    char* _weights_path,
    char** _image_paths,
    size_t _image_count) :
    sc_module(nm),
    DEFINE_PORT(io_clk_i),
    DEFINE_PORT(io_rst_o),
    DEFINE_PORT(io_brq_o),
    DEFINE_PORT(io_bgt_i),
    DEFINE_PORT(io_valid_i),
    DEFINE_PORT(io_ready_o),
    DEFINE_PORT(io_addr_bo),
    DEFINE_PORTVEC(io_data_bi, CONFIG_BUS_WIDTH),
    DEFINE_PORTVEC(io_data_bo, CONFIG_BUS_WIDTH),
    DEFINE_PORT(io_wr_o),
    DEFINE_PORT(io_rd_o),
    should_stop(false),
    image_count(_image_count)
{
    std::cout << "IO: provided " << image_count << " images" << std::endl;
    strncpy(weights_path, _weights_path, PATH_MAX);
    for (size_t i = 0; i < image_count; ++i)
        strncpy(image_paths[i], _image_paths[i], PATH_MAX);
    std::cout << "IO: internal fields initialized" << std::endl;

    io_rst_o.initialize(true);
    io_ready_o.initialize(false);
    io_brq_o.initialize(false);
    io_wr_o.initialize(false);
    io_rd_o.initialize(false);
    io_addr_bo.initialize(0);
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

void Io::bus_write(size_t memory_row, double data[CONFIG_BUS_WIDTH])
{
    bus_capture();

    for (size_t i = 0; i < io_data_bo.size(); ++i)
        io_data_bo[i].write(data[i]);
    io_addr_bo.write(memory_row << 9);
    // std::cout << "IO: write to " << io_addr_bo.read() << std::endl;

    io_wr_o.write(true);
    wait();
    io_wr_o.write(false);
    // wait();
    bus_release();
}

void Io::bus_read(size_t memory_row, double data[CONFIG_BUS_WIDTH])
{
    bus_capture();

    io_addr_bo.write(memory_row << 9);

    io_rd_o.write(true);
    wait();
    io_rd_o.write(false);
    wait();

    for (size_t i = 0; i < io_data_bi.size(); ++i) {
        data[i] = io_data_bi[i].read();
    }
    // std::cout << name() << ": bus_read; data[0] = " << data[0] << std::endl;

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

    const size_t count = CONFIG_LAYER_COUNT * CONFIG_MAX_IMAGE_SIZE;
    for (size_t i = 0; i < count; ++i) {
        double weights_per_input[CONFIG_MAX_IMAGE_SIZE] = {0.0};

        size_t n = fread(weights_per_input, sizeof(double),
            (sizeof(weights_per_input) / sizeof(double)), fweights);

        if (n < CONFIG_MAX_IMAGE_SIZE) {
            state = IOS_ERROR;
            goto err;
        }
        bus_write(i + CONFIG_LAYER_COUNT, weights_per_input);
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
    --image_count;
    std::cout << "$ store_image " << image_paths[image_count] << std::endl;

    FILE* fimage = fopen(image_paths[image_count], "rb");
    if (!fimage) {
        state = IOS_ERROR;
        return;
    }

    double image[CONFIG_MAX_IMAGE_SIZE] = {0.0};
    size_t n = fread(image, sizeof(double),
        (sizeof(image) / sizeof(double)), fimage);

    // std::cout << "# image[0] = " << image[0] << std::endl;
    if (n < CONFIG_MAX_IMAGE_SIZE) {
        state = IOS_ERROR;
        goto err;
    }
    bus_write(0, image);

    state = IOS_WAIT_RESULT;
err:
    fclose(fimage);
}

void Io::wait_result()
{
    io_rst_o.write(false);
    for (size_t layer = 1; layer < CONFIG_LAYER_COUNT; ++layer) {
        // std::cout << "$ wait_result " << layer << " STARTED" << std::endl;
        io_ready_o.write(true);
        wait();
        io_ready_o.write(false);
        while (!io_valid_i.read()) wait();
        std::cout << "$ wait_result " << layer << " FINISHED" << std::endl;
    }
    io_rst_o.write(true);

    state = IOS_LOAD_RESULT;
}

void Io::load_result()
{
    std::cout << "$ load_result" << std::endl;
    for (size_t layer = 0; layer < CONFIG_LAYER_COUNT; ++layer) {
        double output[CONFIG_BUS_WIDTH] = {0.0};
        bus_read(layer, output);
        std::cout << "y[" << layer << "]: {";
        for (size_t i = 0; i < CONFIG_BUS_WIDTH; ++i)
            std::cout << output[i] << ", ";
        std::cout << "}" << std::endl;
    }

    state = IOS_STORE_IMAGE;
}

void Io::memtest()
{
    double a[(CONFIG_MEMORY_ROWS >> 4)][CONFIG_MEMORY_COLS] = {{0.0}};
    // write to the memory
    for (size_t i = 0; i < (CONFIG_MEMORY_ROWS >> 4); ++i) {
        for (size_t j = 0; j < CONFIG_MEMORY_COLS; ++j)
            a[i][j] = i + j;
        bus_write(i, a[i]);
    }

    std::cout << "IO: memtest: ";
    double b[CONFIG_MEMORY_COLS] = {0.0};
    // read from the memory
    for (size_t i = 0; i < (CONFIG_MEMORY_ROWS >> 4); ++i) {
        bus_read(i, b);
        for (size_t j = 0; j < CONFIG_MEMORY_COLS; ++j)
            if (b[j] != a[i][j]) {
                std::cout << "FAILED: expected " << a[i][j] << ", found " << b[j] << std::endl;
                return;
            }
    }
    std::cout << "SUCCEED" << std::endl;
}

void Io::io_routine()
{
    std::cout << "IO: connected" << std::endl;
    while (!should_stop) {
        switch (state) {
            case IOS_STORE_WEIGHTS:
                store_weights(); break;
            case IOS_STORE_IMAGE:
                store_image(); break;
            case IOS_WAIT_RESULT:
                wait_result(); break;
            case IOS_LOAD_RESULT:
                load_result(); break;
            case IOS_MEMTEST:
                memtest();
            case IOS_ERROR:
            default:
                should_stop = true;
                break;
        }
    }
    std::cout << "IO: disconnected" << std::endl;
    sc_stop();
}
