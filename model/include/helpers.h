#ifndef _HELPERS_H_
#define _HELPERS_H_

#define STRINGIFY_PORT(port) #port
#define DEFINE_PORT(port) port(STRINGIFY_PORT(port))
#define DEFINE_PORTVEC(port, size) port(STRINGIFY_PORT(port), (size))

#define DECLARE_MEM_PORTS(prefix, cell_type, addr_width) \
    sc_in<sc_uint<addr_width>> prefix##_addr_bi; \
    sc_vector<sc_in<cell_type>>  prefix##_data_bi; \
    sc_vector<sc_out<cell_type>> prefix##_data_bo; \
    sc_in<bool> prefix##_wr_i; \
    sc_in<bool> prefix##_rd_i

#define DEFINE_MEM_PORTS(prefix, bus_width) \
    DEFINE_PORT(prefix##_addr_bi), \
    DEFINE_PORTVEC(prefix##_data_bi, bus_width), \
    DEFINE_PORTVEC(prefix##_data_bo, bus_width), \
    DEFINE_PORT(prefix##_wr_i), \
    DEFINE_PORT(prefix##_rd_i)

#define DECLARE_MEM_MASTER_PORTS(prefix, cell_type, addr_width) \
    sc_out<sc_uint<addr_width>> prefix##_addr_bo; \
    sc_vector<sc_in<cell_type>>  prefix##_data_bi; \
    sc_vector<sc_out<cell_type>> prefix##_data_bo; \
    sc_out<bool> prefix##_wr_o; \
    sc_out<bool> prefix##_rd_o

#define DEFINE_MEM_MASTER_PORTS(prefix, bus_width) \
    DEFINE_PORT(prefix##_addr_bo), \
    DEFINE_PORTVEC(prefix##_data_bi, bus_width), \
    DEFINE_PORTVEC(prefix##_data_bo, bus_width), \
    DEFINE_PORT(prefix##_wr_o), \
    DEFINE_PORT(prefix##_rd_o)

#define DECLARE_MEM_MASTER_SIGNALS(dev, presig, preport, cell_type, addr_width, bus_width) \
    sc_signal<sc_uint<addr_width>> presig##_addr; \
    sc_vector<sc_signal<cell_type>> presig##_data_bi(#presig "_data_bi", bus_width), presig##_data_bo(#presig "_data_bo", bus_width); \
    sc_signal<bool> presig##_wr, presig##_rd; \
    dev.preport##_addr_bo(presig##_addr); \
    dev.preport##_data_bi(presig##_data_bi); \
    dev.preport##_data_bo(presig##_data_bo); \
    dev.preport##_wr_o(presig##_wr); \
    dev.preport##_rd_o(presig##_rd)

#define DECLARE_BUS_MATRIX_CLIENT_SIGNALS(dev, presig, preport, cell_type, addr_width, bus_width) \
    DECLARE_MEM_MASTER_SIGNALS(dev, presig, preport, cell_type, addr_width, bus_width); \
    sc_signal<bool> presig##_brq, presig##_bgt; \
    dev.preport##_brq_o(presig##_brq); dev.preport##_bgt_i(presig##_bgt)

#define DECLARE_DMA_CPU_LMEM(n, shmem_t, shmem_w, lcmem_w, lcmem_t, clk, bus_width) \
    DMA dma##n("dma" #n, n << 4); \
    dma##n.dma_clk_i(clk); \
    sc_signal<bool> dma##n##_rst; \
    DECLARE_BUS_MATRIX_CLIENT_SIGNALS(dma##n, dma##n##_shmem, dma_shmem, double, CONFIG_MEMADDR_WIDTH, bus_width); \
    DECLARE_MEM_MASTER_SIGNALS(dma##n, dma##n##_lcmem, dma_lcmem, double, CONFIG_LOCAL_MEMADDR_WIDTH, bus_width); \
    sc_signal<bool> dma##n##_ready; dma##n.dma_ready_o(dma##n##_ready); \
    sc_signal<bool> dma##n##_cpu_rst; dma##n.dma_cpu_rst_o(dma##n##_cpu_rst); \
    sc_signal<bool> dma##n##_cpu_ready; dma##n.dma_cpu_ready_i(dma##n##_cpu_ready); \
    CPU cpu##n("cpu" #n, 0 << 4); \
    cpu##n.cpu_clk_i(clk); \
    cpu##n.cpu_rst_i(dma##n##_cpu_rst); \
    cpu##n.cpu_ready_o(dma##n##_cpu_ready); \
    DECLARE_MEM_MASTER_SIGNALS(cpu##n, cpu##n, cpu, double, CONFIG_LOCAL_MEMADDR_WIDTH, bus_width); \
    LocalMemDual lmem##n("lmem" #n); \
    lmem##n.lmd_clk_i(clk); \
    lmem##n.lmd_0_addr_bi(dma##n##_lcmem_addr); \
    lmem##n.lmd_0_data_bi(dma##n##_lcmem_data_bo); \
    lmem##n.lmd_0_data_bo(dma##n##_lcmem_data_bi); \
    lmem##n.lmd_0_rd_i(dma##n##_lcmem_rd); \
    lmem##n.lmd_0_wr_i(dma##n##_lcmem_wr); \
    lmem##n.lmd_1_addr_bi(cpu##n##_addr); \
    lmem##n.lmd_1_data_bi(cpu##n##_data_bo); \
    lmem##n.lmd_1_data_bo(cpu##n##_data_bi); \
    lmem##n.lmd_1_rd_i(cpu##n##_rd); \
    lmem##n.lmd_1_wr_i(cpu##n##_wr)

#define PLUG_DEV_BUS_MATRIX(bus, dev, presig) \
    bus.dev##_brq_i(presig##_brq); \
    bus.dev##_bgt_o(presig##_bgt); \
    bus.dev##_addr_bi(presig##_addr); \
    bus.dev##_data_bi(presig##_data_bo); \
    bus.dev##_data_bo(presig##_data_bi); \
    bus.dev##_wr_i(presig##_wr); \
    bus.dev##_rd_i(presig##_rd)

#define TRACE_SIGNAL(waveform, signal) \
    sc_trace(waveform, signal, #signal)

#endif // _HELPERS_H_
