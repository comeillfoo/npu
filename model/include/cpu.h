#ifndef _CPU_H_
#define _CPU_H_

#include "systemc.h"
#include "config.h"
#include "helpers.h"

#define CPU_OUTPUT_LENGTH (CONFIG_MAX_WEIGHTS_PER_LAYER / 4)


SC_MODULE(CPU)
{

    sc_in<bool> cpu_clk_i;
    sc_in<bool> cpu_rst_i;
    DECLARE_MEM_MASTER_PORTS(cpu, double, CONFIG_LOCAL_MEMADDR_WIDTH);
    sc_out<bool> cpu_ready_o;

    SC_HAS_PROCESS(CPU);

    CPU(sc_module_name nm,
        size_t _shiftout);
    ~CPU();

    void cpu_routine();
private:
    size_t shiftout;
    double weights[CPU_OUTPUT_LENGTH][CONFIG_BUS_WIDTH];
    double previous_output[CONFIG_BUS_WIDTH];
    double sum[CPU_OUTPUT_LENGTH];
    double output[CONFIG_BUS_WIDTH];

    size_t lea(size_t addr);

    void bus_write(size_t memory_row, double data[CONFIG_BUS_WIDTH]);
    void bus_read(size_t memory_row, double data[CONFIG_BUS_WIDTH]);
};

#endif // _CPU_H_
