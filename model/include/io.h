#ifndef _IO_H_
#define _IO_H_

#include "systemc.h"
#include "config.h"
#include <linux/limits.h>

enum io_states {
    IOS_STORE_WEIGHTS = 0,
    IOS_STORE_IMAGE,
    IOS_WAIT_RESULT,
    IOS_LOAD_RESULT,
    IOS_MEMTEST,
    IOS_ERROR
};


SC_MODULE(Io)
{
    sc_in<bool> io_clk_i;
    sc_out<bool> io_rst_o;
    sc_out<bool> io_brq_o;
    sc_in<bool> io_bgt_i;
    sc_in<bool> io_valid_i;
    sc_out<bool> io_ready_o;
    sc_out<sc_uint<CONFIG_MEMADDR_WIDTH>> io_addr_bo;
    sc_vector<sc_in<double>> io_data_bi;
    sc_vector<sc_out<double>> io_data_bo;
    sc_out<bool> io_wr_o;
    sc_out<bool> io_rd_o;

    SC_HAS_PROCESS(Io);

    Io(sc_module_name nm,
        char* _weights_path,
        char** _image_paths,
        size_t _image_count);
    ~Io();

    void io_routine();

private:
    enum io_states state;
    bool should_stop;

    char weights_path[PATH_MAX];
    char image_paths[CONFIG_MAX_IMAGES][PATH_MAX];
    size_t image_count;

    void bus_capture();
    void bus_write(size_t memory_row, double data[CONFIG_BUS_WIDTH]);
    void bus_read(size_t memory_row, double data[CONFIG_BUS_WIDTH]);
    void bus_release();

    void store_weights();
    void store_image();
    void load_result();
    void wait_result();
    bool memtest();
};

#endif // _IO_H_
