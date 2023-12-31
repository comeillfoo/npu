#include "io.h"
#include "cpu.h"
#include "dma.h"
#include "local_mem_dual.h"
#include "mem.h"
#include "bus.h"

int sc_main(int argc, char* argv[])
{
    // provide weights and images from paths
    size_t images = argc - 2;
    if (argc < 3 || images > CONFIG_MAX_IMAGES) {
        return(1);
    }

    sc_clock clk("clk", 10, SC_NS);

    Bus bus("bus");
    // bus <-> memory signals
    sc_signal<sc_uint<CONFIG_MEMADDR_WIDTH>> bus_addr;
    sc_vector<sc_signal<double>> bus_data_bi("bus_data_bi", CONFIG_BUS_WIDTH), bus_data_bo("bus_data_bo", CONFIG_BUS_WIDTH);
    sc_signal<bool> bus_wr, bus_rd;
    bus.bus_clk_i(clk);
    bus.bus_addr_bo(bus_addr);
    bus.bus_data_bi(bus_data_bi);
    bus.bus_data_bo(bus_data_bo);
    bus.bus_wr_o(bus_wr);
    bus.bus_rd_o(bus_rd);

    Io io("io", argv[1], argv + 2, argc - 2);
    io.io_clk_i(clk);
    sc_signal<bool> io_rst; io.io_rst_o(io_rst);
    DECLARE_BUS_MATRIX_CLIENT_SIGNALS(io, io, io, double, CONFIG_MEMADDR_WIDTH, CONFIG_BUS_WIDTH); // bus <-> io signals
    sc_signal<bool> io_ready; io.io_ready_o(io_ready);
    PLUG_DEV_BUS_MATRIX(bus, io, io); // connect io to bus

    DECLARE_DMA_CPU_LMEM(0, double, CONFIG_MEMADDR_WIDTH, double, CONFIG_LOCAL_MEMADDR_WIDTH, clk, CONFIG_BUS_WIDTH);
    dma0.dma_rst_i(io_rst);
    PLUG_DEV_BUS_MATRIX(bus, dma0, dma0_shmem); // connect dma0 to bus

    DECLARE_DMA_CPU_LMEM(1, double, CONFIG_MEMADDR_WIDTH, double, CONFIG_LOCAL_MEMADDR_WIDTH, clk, CONFIG_BUS_WIDTH);
    dma1.dma_rst_i(io_rst);
    PLUG_DEV_BUS_MATRIX(bus, dma1, dma1_shmem); // connect dma1 to bus

    DECLARE_DMA_CPU_LMEM(2, double, CONFIG_MEMADDR_WIDTH, double, CONFIG_LOCAL_MEMADDR_WIDTH, clk, CONFIG_BUS_WIDTH);
    dma2.dma_rst_i(io_rst);
    PLUG_DEV_BUS_MATRIX(bus, dma2, dma2_shmem); // connect dma2 to bus

    DECLARE_DMA_CPU_LMEM(3, double, CONFIG_MEMADDR_WIDTH, double, CONFIG_LOCAL_MEMADDR_WIDTH, clk, CONFIG_BUS_WIDTH);
    dma3.dma_rst_i(io_rst);
    PLUG_DEV_BUS_MATRIX(bus, dma3, dma3_shmem); // connect dma3 to bus

    // make valid/ready chain
    dma0.dma_valid_i(io_ready);
    dma1.dma_valid_i(dma0_ready);
    dma2.dma_valid_i(dma1_ready);
    dma3.dma_valid_i(dma2_ready);
    io.io_valid_i(dma3_ready);

    // connect mem to bus
    Mem memory("memory");
    memory.mem_clk_i(clk);
    memory.mem_addr_bi(bus_addr);
    memory.mem_data_bi(bus_data_bo);
    memory.mem_data_bo(bus_data_bi);
    memory.mem_wr_i(bus_wr);
    memory.mem_rd_i(bus_rd);

    sc_trace_file* wf = sc_create_vcd_trace_file("wave");
    TRACE_SIGNAL(wf, clk);
    // bus matrix requests/grants
    TRACE_SIGNAL(wf, io_bgt);
    TRACE_SIGNAL(wf, io_brq);
    TRACE_SIGNAL(wf, dma0_shmem_bgt);
    TRACE_SIGNAL(wf, dma0_shmem_brq);
    TRACE_SIGNAL(wf, dma1_shmem_bgt);
    TRACE_SIGNAL(wf, dma1_shmem_brq);
    TRACE_SIGNAL(wf, dma2_shmem_bgt);
    TRACE_SIGNAL(wf, dma2_shmem_brq);
    TRACE_SIGNAL(wf, dma3_shmem_bgt);
    TRACE_SIGNAL(wf, dma3_shmem_brq);

    // valid/ready chain
    TRACE_SIGNAL(wf, io_ready);
    TRACE_SIGNAL(wf, dma0_ready);
    TRACE_SIGNAL(wf, dma1_ready);
    TRACE_SIGNAL(wf, dma2_ready);
    TRACE_SIGNAL(wf, dma3_ready);
    // resets
    TRACE_SIGNAL(wf, io_rst);
    TRACE_SIGNAL(wf, dma0_rst);
    TRACE_SIGNAL(wf, dma1_rst);
    TRACE_SIGNAL(wf, dma2_rst);
    TRACE_SIGNAL(wf, dma3_rst);

    TRACE_SIGNAL(wf, dma0_cpu_ready);
    TRACE_SIGNAL(wf, dma0_cpu_rst);
    TRACE_SIGNAL(wf, dma1_cpu_ready);
    TRACE_SIGNAL(wf, dma1_cpu_rst);
    TRACE_SIGNAL(wf, dma2_cpu_ready);
    TRACE_SIGNAL(wf, dma2_cpu_rst);
    TRACE_SIGNAL(wf, dma3_cpu_ready);
    TRACE_SIGNAL(wf, dma3_cpu_rst);

    std::cout << "simulation started" << std::endl;

    sc_start();

    sc_close_vcd_trace_file(wf);

    return(0);
}
