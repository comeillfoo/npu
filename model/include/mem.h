#ifndef _MEM_H
#define _MEM_H

#include "systemc.h"
#include "sysreqs.h"

SC_MODULE(Mem)
{

    sc_in<bool>  mem_clk_i;
    sc_in<sc_uint<SYS_MEMADDR_WIDTH>> mem_addr_bi;
    sc_vector<sc_in<double>>  mem_data_bi;
    sc_vector<sc_out<double>> mem_data_bo;
    sc_in<bool> mem_wr_i;
    sc_in<bool> mem_rd_i;

    SC_HAS_PROCESS(Mem);

    Mem(sc_module_name nm);
    ~Mem();

    void bus_write();
    void bus_read();

private:
    double mem[SYS_MEMORY_ROWS][SYS_MEMORY_COLS];

    size_t row(size_t addr);
};

#endif
