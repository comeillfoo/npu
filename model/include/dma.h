#ifndef _DMA_H_
#define _DMA_H_

#include "systemc.h"
#include "config.h"
#include "helpers.h"

SC_MODULE(DMA)
{
    sc_in<bool> dma_clk_i;
    sc_in<bool> dma_rst_i;
    sc_in<bool> dma_shmem_bgt_i;
    sc_in<bool> dma_valid_i;
    sc_in<bool> dma_cpu_ready_i;
    DECLARE_MEM_MASTER_PORTS(dma_lcmem, double, CONFIG_LOCAL_MEMADDR_WIDTH);
    DECLARE_MEM_MASTER_PORTS(dma_shmem, double, CONFIG_MEMADDR_WIDTH);
    sc_out<bool> dma_ready_o;
    sc_out<bool> dma_shmem_brq_o;
    sc_out<bool> dma_cpu_rst_o;

    SC_HAS_PROCESS(DMA);

    DMA(sc_module_name nm, size_t _shift);
    ~DMA();

    void dma_routine();
private:
    double buf[CONFIG_BUS_WIDTH];
    double buf1[CONFIG_BUS_WIDTH];
    size_t layer;
    size_t shift;

    size_t lea(size_t addr);

    void lcmem_write(size_t memory_row, double data[CONFIG_BUS_WIDTH]);
    void lcmem_read(size_t memory_row, double data[CONFIG_BUS_WIDTH]);
    void shmem_capture();
    void shmem_write(size_t memory_row, double data[CONFIG_BUS_WIDTH]);
    void shmem_read(size_t memory_row, double data[CONFIG_BUS_WIDTH]);
    void shmem_release();
};

#endif // _DMA_H_