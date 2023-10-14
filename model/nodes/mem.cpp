#include "mem.h"
#include "helpers.h"

Mem::Mem(sc_module_name nm) :
    sc_module(nm),
    DEFINE_PORT(mem_clk_i),
    DEFINE_PORT(mem_addr_bi),
    DEFINE_PORTVEC(mem_data_bi, SYS_BUS_WIDTH),
    DEFINE_PORTVEC(mem_data_bo, SYS_BUS_WIDTH),
    DEFINE_PORT(mem_wr_i),
    DEFINE_PORT(mem_rd_i)

{
    for (size_t i = 0; i < mem_data_bo.size(); ++i)
        mem_data_bo[i].initialize(0.0);

    SC_METHOD(bus_write);
    sensitive << mem_clk_i.pos();

    SC_METHOD(bus_read);
    sensitive << mem_clk_i.pos();

}

Mem::~Mem()
{
}

size_t Mem::row(size_t addr)
{
    return addr >> 9;
}

void Mem::bus_read()
{
    size_t r = row(mem_addr_bi.read());
    if (mem_wr_i.read()) {
        for (size_t c = 0; c < SYS_MEMORY_COLS; ++c)
            mem[r][c] = mem_data_bi[c].read();
    }
}

void Mem::bus_write()
{
    size_t r = row(mem_addr_bi.read());
    if(mem_rd_i.read())
        for (size_t c = 0; c < SYS_MEMORY_COLS; ++c)
            mem_data_bo[c].write(mem[r][c]);
}
