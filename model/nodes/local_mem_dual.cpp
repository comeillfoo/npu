#include "local_mem_dual.h"

LocalMemDual::LocalMemDual(sc_module_name nm) :
    sc_module(nm),
    DEFINE_PORT(lmd_clk_i),
    DEFINE_MEM_PORTS(lmd_0, CONFIG_BUS_WIDTH),
    DEFINE_MEM_PORTS(lmd_1, CONFIG_BUS_WIDTH)
{
    for (size_t i = 0; i < lmd_0_data_bo.size(); ++i)
        lmd_0_data_bo[i].initialize(0.0);
    for (size_t i = 0; i < lmd_1_data_bo.size(); ++i)
        lmd_1_data_bo[i].initialize(0.0);

    SC_METHOD(bus_write);
    sensitive << lmd_clk_i.pos();

    SC_METHOD(bus_read);
    sensitive << lmd_clk_i.pos();

}

LocalMemDual::~LocalMemDual()
{
}

size_t LocalMemDual::row(size_t addr)
{
    return addr >> 9; // 3 for double bytes; 6 for columns number
}

void LocalMemDual::port_read(
    sc_in<sc_uint<CONFIG_LOCAL_MEMADDR_WIDTH>> &addr_bi,
    sc_vector<sc_in<double>> &data_bi)
{
    const size_t r = row(addr_bi.read());
    for (size_t c = 0; c < CONFIG_MEMORY_COLS; ++c)
        mem[r][c] = data_bi[c].read();
}

void LocalMemDual::bus_read()
{
    if (lmd_0_rd_i.read())
        port_read(lmd_0_addr_bi, lmd_0_data_bi);

    if (lmd_1_rd_i.read())
        port_read(lmd_1_addr_bi, lmd_1_data_bi);
}

void LocalMemDual::port_write(
    sc_in<sc_uint<CONFIG_LOCAL_MEMADDR_WIDTH>> &addr_bi,
    sc_vector<sc_out<double>> &data_bo)
{
    const size_t r = row(addr_bi.read());
    for (size_t c = 0; c < CONFIG_MEMORY_COLS; ++c)
        data_bo[c].write(mem[r][c]);
}

void LocalMemDual::bus_write()
{
    if (lmd_0_wr_i.read())
        port_write(lmd_0_addr_bi, lmd_0_data_bo);

    if (lmd_1_wr_i.read())
        port_write(lmd_1_addr_bi, lmd_1_data_bo);
}
