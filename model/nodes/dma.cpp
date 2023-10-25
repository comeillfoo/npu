#include "dma.h"

#define DMA_WEIGHTS_ROWS (CONFIG_MAX_WEIGHTS_PER_LAYER / 4)
#define SHMEM_OUTPUT_SHIFT (32ULL)

DMA::DMA(sc_module_name nm,
    size_t _shift) :
    sc_module(nm),
    DEFINE_PORT(dma_clk_i),
    DEFINE_PORT(dma_rst_i),
    DEFINE_PORT(dma_shmem_bgt_i),
    DEFINE_PORT(dma_valid_i),
    DEFINE_PORT(dma_cpu_ready_i),
    DEFINE_MEM_MASTER_PORTS(dma_lcmem, CONFIG_BUS_WIDTH),
    DEFINE_MEM_MASTER_PORTS(dma_shmem, CONFIG_BUS_WIDTH),
    DEFINE_PORT(dma_ready_o),
    DEFINE_PORT(dma_shmem_brq_o),
    DEFINE_PORT(dma_cpu_rst_o),
    buf{0.0},
    buf1{0.0},
    layer(1),
    shift(_shift)
{
    dma_cpu_rst_o.initialize(true);
    dma_shmem_brq_o.initialize(false);
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
    dma_shmem_brq_o.write(true);
    while (!dma_shmem_bgt_i.read()) wait();
}

void DMA::shmem_release()
{
    dma_shmem_brq_o.write(false);
    while (dma_shmem_bgt_i.read()) wait();
}

void DMA::lcmem_write(size_t memory_row, double data[CONFIG_BUS_WIDTH])
{
    for (size_t i = 0; i < dma_lcmem_data_bo.size(); ++i)
        dma_lcmem_data_bo[i].write(data[i]);
    dma_lcmem_addr_bo.write(memory_row << 9);

    dma_lcmem_wr_o.write(true);
    wait();
    dma_lcmem_wr_o.write(false);
    wait();
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
    wait();
    dma_shmem_rd_o.write(true);
    wait(); wait(); wait(); wait();

    for (size_t i = 0; i < dma_shmem_data_bi.size(); ++i)
        data[i] = dma_shmem_data_bi[i].read();
    dma_shmem_rd_o.write(false);
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
    while (true) {
        if (!dma_rst_i.read() && dma_valid_i.read()) {
            std::cout << name() << ": process layer: " << layer << std::endl;
            dma_cpu_rst_o.write(true); // reset linked cpu
            dma_ready_o.write(false); // reset ready flag

            shmem_read(layer - 1, buf); // read previous output from shared memory
            lcmem_write(0, buf); // write to local memory

            for (size_t i = 0; i < DMA_WEIGHTS_ROWS; ++i) {
                shmem_read(lea(i), buf); // read layer weights
                lcmem_write(2 + i, buf); // store layer weights
            }

            dma_cpu_rst_o.write(false); // start cpu
            while (!dma_cpu_ready_i.read()) wait(); // wait for end of calculations
            dma_cpu_rst_o.write(true); // stop cpu

            // merge previous output with the calculated
            shmem_read(layer, buf);
            lcmem_read(1, buf1);
            for (size_t i = 0; i < CONFIG_BUS_WIDTH; ++i)
                buf[i] += buf1[i];
            shmem_write(layer, buf);
            layer = (layer + 1) % CONFIG_LAYER_COUNT;
            dma_ready_o.write(true); // notify next block in chain
            wait();
            dma_ready_o.write(false);
        }

        // no reset and no valid data
        if (!dma_rst_i.read() && !dma_valid_i.read()) wait();

        // reset
        if (dma_rst_i.read()) {
            layer = 1;
            for (size_t i = 0; i < CONFIG_BUS_WIDTH; ++i) {
                buf[i] = 0.0;
                buf1[i] = 0.0;
            }
            dma_cpu_rst_o.write(true);
            dma_shmem_brq_o.write(false);
            dma_ready_o.write(false);
            dma_lcmem_rd_o.write(false);
            dma_lcmem_wr_o.write(false);
            dma_lcmem_addr_bo.write(0);
            for (size_t i = 0; i < dma_lcmem_data_bo.size(); ++i)
                dma_lcmem_data_bo[i].write(0.0);

            dma_shmem_rd_o.write(false);
            dma_shmem_wr_o.write(false);
            dma_shmem_addr_bo.write(0);
            for (size_t i = 0; i < dma_shmem_data_bo.size(); ++i)
                dma_shmem_data_bo[i].write(0.0);
            wait();
        }
    }
}

#undef DMA_WEIGHTS_ROWS
#undef SHMEM_OUTPUT_SHIFT
