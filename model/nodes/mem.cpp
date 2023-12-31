#include "mem.h"


Mem::Mem(sc_module_name nm) :
    sc_module(nm),
    DEFINE_PORT(mem_clk_i),
    DEFINE_MEM_PORTS(mem, CONFIG_BUS_WIDTH)
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
        // std::cout << name() << ": write at " << r << ": { ";
        for (size_t c = 0; c < CONFIG_MEMORY_COLS; ++c) {
            mem[r][c] = mem_data_bi[c].read();
            // std::cout << mem[r][c] << " ";
        }
        // std::cout << "}" << std::endl;
    }
}

void Mem::bus_write()
{
    size_t r = row(mem_addr_bi.read());
    if(mem_rd_i.read()) {
        // std::cout << name() << ": read at " << r << ": { ";
        for (size_t c = 0; c < CONFIG_MEMORY_COLS; ++c) {
            mem_data_bo[c].write(mem[r][c]);
            // std::cout << mem[r][c] << " ";
        }
        // std::cout << "}" << std::endl;
    }
}
