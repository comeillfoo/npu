#include <cmath>
#include "cpu.h"
#include "config.h"
#include "helpers.h"


static double sigmoid(double x)
{
    return 1 / (1 + exp(-x));
}


CPU::CPU(sc_module_name nm, size_t _shiftout) :
    sc_module(nm),
    DEFINE_PORT(cpu_clk_i),
    DEFINE_PORT(cpu_rst_i),
    DEFINE_MEM_MASTER_PORTS(cpu, CONFIG_BUS_WIDTH),
    DEFINE_PORT(cpu_ready_o),
    shiftout(_shiftout),
    weights{{0.0}},
    previous_output{0.0},
    partial_products{{0.0}},
    sum{0.0},
    output{0.0}
{
    cpu_ready_o.initialize(false);
    cpu_rd_o.initialize(false);
    cpu_wr_o.initialize(false);
    cpu_addr_bo.initialize(0);
    for (size_t i = 0; i < cpu_data_bo.size(); ++i)
        cpu_data_bo[i].initialize(0.0);

    SC_CTHREAD(cpu_routine, cpu_clk_i.pos());
}

CPU::~CPU()
{
}

void CPU::bus_write(size_t memory_row, double data[CONFIG_BUS_WIDTH])
{
    for (size_t i = 0; i < cpu_data_bo.size(); ++i)
        cpu_data_bo[i].write(data[i]);
    cpu_addr_bo.write(memory_row << 9);

    cpu_wr_o.write(true);
    wait();
    cpu_wr_o.write(false);
}

void CPU::bus_read(size_t memory_row, double data[CONFIG_BUS_WIDTH])
{
    cpu_addr_bo.write(memory_row << 9);

    cpu_rd_o.write(true);
    wait();
    cpu_rd_o.write(false);
    wait();

    for (size_t i = 0; i < cpu_data_bi.size(); ++i)
        data[i] = cpu_data_bi[i].read();
}

size_t CPU::lea(size_t addr)
{
    return addr + shiftout;
}

// TODO: add while loop 'cause of cthread
void CPU::cpu_routine()
{
    while (true) {
        if (!cpu_rst_i.read()) {
            cpu_ready_o.write(false);
            // read previous outputs
            bus_read(0, previous_output);
            // read weights
            for (size_t i = 0; i < CPU_OUTPUT_LENGTH; ++i)
                bus_read((i + 2) << 9, weights[i]);

            // calculate partial products
            for (size_t i = 0; i < CPU_OUTPUT_LENGTH; ++i)
                for (size_t j = 0; j < CONFIG_BUS_WIDTH; ++j)
                    partial_products[i][j] = weights[i][j] * previous_output[j];

            // calculate output
            for (size_t i = 0; i < CPU_OUTPUT_LENGTH; ++i) {
                sum[i] = 0.0;
                for (size_t j = 0; j < CONFIG_BUS_WIDTH; ++j)
                    sum[i] += partial_products[i][j];
                output[lea(i)] = sigmoid(sum[i]);
            }

            // store output
            bus_write(1 << 9, output);
            cpu_ready_o.write(true);
            wait();
        }

        // reset
        if (cpu_rst_i.read()) {
            // zero outputs
            cpu_ready_o.write(false);
            cpu_rd_o.write(false);
            cpu_wr_o.write(false);
            cpu_addr_bo.write(0);
            for (size_t i = 0; i < cpu_data_bo.size(); ++i)
                cpu_data_bo[i].write(0.0);
            wait();
        }
    }
}
