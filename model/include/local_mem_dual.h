#ifndef _LOCAL_MEM_DUAL_H_
#define _LOCAL_MEM_DUAL_H_

#include "systemc.h"
#include "sysreqs.h"
#include "helpers.h"


SC_MODULE(LocalMemDual)
{
    sc_in<bool> lmd_clk_i;
    DECLARE_MEM_PORTS(lmd_0, double, SYS_RQ_LOCAL_MEMADDR_WIDTH);
    DECLARE_MEM_PORTS(lmd_1, double, SYS_RQ_LOCAL_MEMADDR_WIDTH);

    SC_HAS_PROCESS(LocalMemDual);

    LocalMemDual(sc_module_name nm);
    ~LocalMemDual();

    void bus_write();
    void bus_read();

private:
    double mem[SYS_RQ_LOCAL_MEM_ROWS][SYS_RQ_MEMORY_COLS];

    size_t row(size_t addr);

    void port_write(
        sc_in<sc_uint<SYS_RQ_LOCAL_MEMADDR_WIDTH>> &addr_bi,
        sc_vector<sc_out<double>> &data_bo);
    void port_read(
        sc_in<sc_uint<SYS_RQ_LOCAL_MEMADDR_WIDTH>> &addr_bi,
        sc_vector<sc_in<double>> &data_bi);
};

#endif // _LOCAL_MEM_DUAL_H_
