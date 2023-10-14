#include "mem.h"
#include "bus.h"

int sc_main(int argc, char* argv[])
{
    // provide weights and images from paths
    if (argc < 3) {
        return(1);
    }

    Mem memory("memory");

    sc_clock clk("clk", sc_time(10, SC_NS));

    // bus <-> memory signals
    sc_signal<sc_uint<SYS_MEMADDR_WIDTH>> bus_addr;
    sc_vector<sc_signal<double>> bus_data_bi;
    sc_vector<sc_signal<double>> bus_data_bo;
    sc_signal<bool> bus_wr;
    sc_signal<bool> bus_rd;

    memory.mem_clk_i(clk);
    memory.mem_addr_bi(bus_addr);
    memory.mem_data_bi(bus_data_bo);
    memory.mem_data_bo(bus_data_bi);
    memory.mem_wr_i(bus_wr);
    memory.mem_rd_i(bus_rd);

    Bus bus("bus");

    bus.bus_clk_i(clk);
    bus.bus_addr_bo(bus_addr);
    bus.bus_data_bi(bus_data_bi);
    bus.bus_data_bo(bus_data_bo);
    bus.bus_wr_o(bus_wr);
    bus.bus_rd_o(bus_rd);

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
