#include "dma.h"

#define DMA_WEIGHTS_ROWS (CONFIG_MAX_WEIGHTS_PER_LAYER / 4)
#define SHMEM_OUTPUT_SHIFT (32ULL)

DMA::DMA(sc_module_name nm,
    size_t _shift) :
    sc_module(nm),
    DEFINE_PORT(dma_clk_i),
    DEFINE_PORT(dma_bgt_i),
    DEFINE_PORT(dma_valid_i),
    DEFINE_PORT(dma_cpu_ready_i),
    DEFINE_MEM_MASTER_PORTS(dma_lcmem, CONFIG_BUS_WIDTH),
    DEFINE_MEM_MASTER_PORTS(dma_shmem, CONFIG_BUS_WIDTH),
    DEFINE_PORT(dma_ready_o),
    DEFINE_PORT(dma_brq_o),
    DEFINE_PORT(dma_cpu_rst_o),
    buffer{0.0},
    layer(1),
    shift(_shift)
{
    dma_cpu_rst_o.initialize(true);
    dma_brq_o.initialize(false);
    dma_ready_o.initialize(false);
    dma_lcmem_rd_o.initialize(false);
    dma_lcmem_wr_o.initialize(false);
    dma_lcmem_addr_bo.initialize(0);
    for (size_t i = 0; i < dma_lcmem_data_bo.size(); ++i)
        dma_lcmem_data_bo[i].initialize(0.0);

    dma_shmem_rd_o.initialize(false);
    dma_shmem_wr_o.initialize(false);
    dma_shmem_addr_bo.initialize(0);
    for (size_t i = 0; i < dma_shmem_data_bo.size(); ++i)
        dma_shmem_data_bo[i].initialize(0.0);

    SC_CTHREAD(dma_routine, dma_clk_i.pos());
}

DMA::~DMA()
{
}

void DMA::shmem_capture()
{
    dma_brq_o.write(true);
    while (!dma_bgt_i.read()) wait();
}

void DMA::shmem_release()
{
    dma_brq_o.write(false);
    while (dma_bgt_i.read()) wait();
}

void DMA::lcmem_write(size_t memory_row, double data[CONFIG_BUS_WIDTH])
{
    for (size_t i = 0; i < dma_lcmem_data_bo.size(); ++i)
        dma_lcmem_data_bo[i].write(data[i]);
    dma_lcmem_addr_bo.write(memory_row << 9);

    dma_lcmem_wr_o.write(true);
    wait();
    dma_lcmem_wr_o.write(false);
}

void DMA::lcmem_read(size_t memory_row, double data[CONFIG_BUS_WIDTH])
{
    dma_lcmem_addr_bo.write(memory_row << 9);

    dma_lcmem_rd_o.write(true);
    wait();
    dma_lcmem_rd_o.write(false);
    wait();

    for (size_t i = 0; i < dma_lcmem_data_bi.size(); ++i)
        data[i] = dma_lcmem_data_bi[i].read();
}

void DMA::shmem_write(size_t memory_row, double data[CONFIG_BUS_WIDTH])
{
    shmem_capture();
    for (size_t i = 0; i < dma_shmem_data_bo.size(); ++i)
        dma_shmem_data_bo[i].write(data[i]);
    dma_shmem_addr_bo.write(memory_row << 9);

    dma_shmem_wr_o.write(true);
    wait();
    dma_shmem_wr_o.write(false);
    shmem_release();
}

void DMA::shmem_read(size_t memory_row, double data[CONFIG_BUS_WIDTH])
{
    shmem_capture();
    dma_shmem_addr_bo.write(memory_row << 9);

    dma_shmem_rd_o.write(true);
    wait();
    dma_shmem_rd_o.write(false);
    wait();

    for (size_t i = 0; i < dma_shmem_data_bi.size(); ++i)
        data[i] = dma_shmem_data_bi[i].read();
    shmem_release();
}

size_t DMA::lea(size_t addr)
{
    // 32 + 64 * L + shift + row
    return SHMEM_OUTPUT_SHIFT
        + CONFIG_MAX_WEIGHTS_PER_LAYER * (layer - 1)
        + shift + addr;
}

// TODO: add while loop 'cause of cthread
void DMA::dma_routine()
{
    if (dma_valid_i.read()) {
        dma_cpu_rst_o.write(true); // reset linked cpu
        dma_ready_o.write(false); // reset read

        shmem_read(layer - 1, buffer); // read previous output from shared memory
        lcmem_write(0, buffer); // write to local memory

        for (size_t i = 0; i < DMA_WEIGHTS_ROWS; ++i) {
            shmem_read(lea(i), buffer); // read layer weights
            lcmem_write(2 + i, buffer); // store layer weights
        }

        dma_cpu_rst_o.write(false); // start cpu
        while (!dma_cpu_ready_i.read()) wait(); // wait for end of calculations
        // merge previous output with the calculated

        // store merged output
    }
}

#undef DMA_WEIGHTS_ROWS
#undef SHMEM_OUTPUT_SHIFT
