#include "io.h"
#include "mem.h"
#include "bus.h"

int sc_main(int argc, char* argv[])
{
    // provide weights and images from paths
    size_t images = argc - 2;
    if (argc < 3 || images > CONFIG_MAX_IMAGES) {
        return(1);
    }

    sc_clock clk("clk", sc_time(10, SC_NS));

    Mem memory("memory");
    // bus <-> memory signals
    sc_signal<sc_uint<CONFIG_MEMADDR_WIDTH>> bus_addr;
    sc_vector<sc_signal<double>> bus_data_bi, bus_data_bo;
    sc_signal<bool> bus_wr, bus_rd;

    memory.mem_clk_i(clk);
    memory.mem_addr_bi(bus_addr);
    memory.mem_data_bi(bus_data_bo);
    memory.mem_data_bo(bus_data_bi);
    memory.mem_wr_i(bus_wr);
    memory.mem_rd_i(bus_rd);

    Io io("io", argv[1], argv + 2, argc - 2);
    // bus <-> io signals
    sc_signal<bool> io_brq, io_bgt;
    sc_signal<sc_uint<CONFIG_MEMADDR_WIDTH>> io_addr;
    sc_vector<sc_signal<double>> io_data_bi, io_data_bo;
    sc_signal<bool> io_wr, io_rd;

    io.io_brq_o(io_brq);
    io.io_bgt_i(io_bgt);
    io.io_addr_bo(io_addr);
    io.io_data_bi(io_data_bi);
    io.io_data_bo(io_data_bo);
    io.io_wr_o(io_wr);
    io.io_rd_o(io_rd);

    Bus bus("bus");

    // connect mem
    bus.bus_clk_i(clk);
    bus.bus_addr_bo(bus_addr);
    bus.bus_data_bi(bus_data_bi);
    bus.bus_data_bo(bus_data_bo);
    bus.bus_wr_o(bus_wr);
    bus.bus_rd_o(bus_rd);

    // connect io
    bus.io_brq_i(io_brq);
    bus.io_bgt_o(io_bgt);
    bus.io_addr_bi(io_addr);
    bus.io_data_bi(io_data_bo);
    bus.io_data_bo(io_data_bi);
    bus.io_wr_i(io_wr);
    bus.io_rd_i(io_rd);

    sc_trace_file *wf = sc_create_vcd_trace_file("wave");
    sc_trace(wf, clk, "clk");
    sc_trace(wf, bus_addr, "bus_addr");
    // sc_trace(wf, bus_data_bi, "bus_data_bi");
    // sc_trace(wf, bus_data_bo, "bus_data_bo");
    sc_trace(wf, bus_wr, "bus_wr");
    sc_trace(wf, bus_rd, "bus_rd");

    sc_start();

    sc_close_vcd_trace_file(wf);

    return(0);
}
