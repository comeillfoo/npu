#ifndef _MEM_H
#define _MEM_H

#include "systemc.h"
#include "config.h"
#include "helpers.h"

SC_MODULE(Mem)
{
    sc_in<bool>  mem_clk_i;
    DECLARE_MEM_PORTS(mem, double, CONFIG_MEMADDR_WIDTH);

    SC_HAS_PROCESS(Mem);

    Mem(sc_module_name nm);
    ~Mem();

    void bus_write();
    void bus_read();

private:
    double mem[CONFIG_MEMORY_ROWS][CONFIG_MEMORY_COLS];

    size_t row(size_t addr);
};

/*
 * ------------ Memory map ---------------
 * 0000: input image     0x000000-0x0001ff
 * 0001: output l01      0x000200-0x0003ff
 * :::::::::::::::::::::::::::::::::::::::
 * 0031: output l31      0x003e00-0x003fff
 * 0032: weights l01 n00 0x004000-0x0041ff
 * 0033: weights l01 n01 0x004200-0x0043ff
 * :::::::::::::::::::::::::::::::::::::::
 * 0095: weights l01 n63 0x004e00-0x004fff
 * 0096: weights l02 n00 0x005000-0x0051ff
 * 0097: weights l02 n01 0x005200-0x0053ff
 * :::::::::::::::::::::::::::::::::::::::
 * 0159: weights l02 n63 0x005e00-0x005fff
 * :::::::::::::::::::::::::::::::::::::::
 * 2015: weights l31 n63 0x102e00-0x102fff
 * 2016: weights l32 n00 0x103000-0x1031ff -+
 * :::::::::::::::::::::::::::::::::::::::  +- reserved
 * 2079: weights l32 n63 0x103e00-0x103fff -+
 */

#endif
