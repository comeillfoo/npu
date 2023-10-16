#ifndef _MEM_H
#define _MEM_H

#include "systemc.h"
#include "sysreqs.h"
#include "helpers.h"

SC_MODULE(Mem)
{
    sc_in<bool>  mem_clk_i;
    DECLARE_MEM_PORTS(mem, double, SYS_RQ_MEMADDR_WIDTH);

    SC_HAS_PROCESS(Mem);

    Mem(sc_module_name nm);
    ~Mem();

    void bus_write();
    void bus_read();

private:
    double mem[SYS_RQ_MEMORY_ROWS][SYS_RQ_MEMORY_COLS];

    size_t row(size_t addr);
};

#endif
