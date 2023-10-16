#ifndef _CPU_H_
#define _CPU_H_

#include "systemc.h"
#include "sysreqs.h"
#include "helpers.h"

#define CPU_OUTPUT_LENGTH (SYS_RQ_BUS_WIDTH / 4)


SC_MODULE(CPU)
{

    sc_in<bool> cpu_clk_i;
    sc_in<bool> cpu_rst_i;
    DECLARE_MEM_MASTER_PORTS(cpu, double, SYS_RQ_LOCAL_MEMADDR_WIDTH);
    sc_out<bool> cpu_ready_o;

    SC_HAS_PROCESS(CPU);

    CPU(sc_module_name nm,
        size_t _shiftout);
    ~CPU();

    void cpu_routine();
private:
    size_t shiftout;
    double weights[CPU_OUTPUT_LENGTH][SYS_RQ_BUS_WIDTH];
    double previous_output[SYS_RQ_BUS_WIDTH];
    double partial_products[CPU_OUTPUT_LENGTH][SYS_RQ_BUS_WIDTH];
    double sum[CPU_OUTPUT_LENGTH];
    double output[SYS_RQ_BUS_WIDTH];

    void bus_write(size_t memory_row, double data[SYS_RQ_BUS_WIDTH]);
    void bus_read(size_t memory_row, double data[SYS_RQ_BUS_WIDTH]);
};

#endif // _CPU_H_