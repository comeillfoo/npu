#include "bus.h"
#include "helpers.h"

#define DEFINE_DEVICE_PORTS(dev) \
    DEFINE_PORT(dev##_wr_i), \
    DEFINE_PORT(dev##_rd_i), \
    DEFINE_PORT(dev##_addr_bi), \
    DEFINE_PORT(dev##_brq_i), \
    DEFINE_PORTVEC(dev##_data_bi, CONFIG_BUS_WIDTH), \
    DEFINE_PORT(dev##_bgt_o), \
    DEFINE_PORTVEC(dev##_data_bo, CONFIG_BUS_WIDTH)

#define INIT_DEVICE_PORTS(dev) \
    for (size_t i = 0; i < dev##_data_bo.size(); ++i) \
        dev##_data_bo[i].initialize(0.0); \
    dev##_bgt_o.initialize(false)

Bus::Bus(sc_module_name nm) :
    sc_module(nm),
    DEFINE_PORT(bus_clk_i),
    DEFINE_DEVICE_PORTS(io),
    DEFINE_DEVICE_PORTS(dma0),
    DEFINE_DEVICE_PORTS(dma1),
    DEFINE_DEVICE_PORTS(dma2),
    DEFINE_DEVICE_PORTS(dma3),
    DEFINE_PORT(bus_addr_bo),
    DEFINE_PORTVEC(bus_data_bi, CONFIG_BUS_WIDTH),
    DEFINE_PORTVEC(bus_data_bo, CONFIG_BUS_WIDTH),
    DEFINE_PORT(bus_wr_o),
    DEFINE_PORT(bus_rd_o)
{
    INIT_DEVICE_PORTS(io);
    INIT_DEVICE_PORTS(dma0);
    INIT_DEVICE_PORTS(dma1);
    INIT_DEVICE_PORTS(dma2);
    INIT_DEVICE_PORTS(dma3);

    bus_addr_bo.initialize(0);
    bus_wr_o.initialize(false);
    bus_rd_o.initialize(false);
    for (size_t i = 0; i < bus_data_bo.size(); ++i)
        bus_data_bo[i].initialize(0.0);

    state = BS_IDLE;

    SC_METHOD(bus_routine);
    sensitive << bus_clk_i.pos();
}

Bus::~Bus()
{
}

void Bus::bus_arbiter()
{
    if (io_brq_i.read()) {
        io_bgt_o.write(true);
        state = BS_BUSY_IO;
        // std::cout << name() << ": io granted" << std::endl;
    } else if (dma0_brq_i.read()) {
        dma0_bgt_o.write(true);
        state = BS_BUSY_DMA0;
        // std::cout << name() << ": dma0 granted" << std::endl;
    } else if (dma1_brq_i.read()) {
        dma1_bgt_o.write(true);
        state = BS_BUSY_DMA1;
        // std::cout << name() << ": dma1 granted" << std::endl;
    } else if (dma2_brq_i.read()) {
        dma2_bgt_o.write(true);
        state = BS_BUSY_DMA2;
        // std::cout << name() << ": dma2 granted" << std::endl;
    } else if (dma3_brq_i.read()) {
        dma3_bgt_o.write(true);
        state = BS_BUSY_DMA3;
        // std::cout << name() << ": dma3 granted" << std::endl;
    }
}

void Bus::bus_service(
    sc_in<bool> &dev_brq_i,
    sc_in<bool> &dev_wr_i,
    sc_in<bool> &dev_rd_i,
    sc_in<sc_uint<CONFIG_MEMADDR_WIDTH>> &dev_addr_bi,
    sc_vector<sc_in<double>> &dev_data_bi,
    sc_out<bool> &dev_bgt_o,
    sc_vector<sc_out<double>> &dev_data_bo)
{
    if (dev_brq_i.read() && dev_bgt_o.read()) {
        bus_wr_o.write(dev_wr_i.read());
        bus_rd_o.write(dev_rd_i.read());
        bus_addr_bo.write(dev_addr_bi.read());
        // copy memory input
        for (size_t i = 0; i < bus_data_bo.size(); ++i)
            bus_data_bo[i].write(dev_data_bi[i].read());
        // copy memory output
        for (size_t i = 0; i < bus_data_bi.size(); ++i)
            dev_data_bo[i].write(bus_data_bi[i].read());

    } else if (!dev_brq_i.read()) {
        bus_wr_o.write(false);
        bus_rd_o.write(false);
        dev_bgt_o.write(false);
        state = BS_IDLE;
    }
}

#define bus_service_device(dev) \
    bus_service(dev##_brq_i, dev##_wr_i, dev##_rd_i, \
        dev##_addr_bi, dev##_data_bi, dev##_bgt_o, dev##_data_bo)


void Bus::bus_routine()
{
    switch (state) {
        case BS_IDLE: bus_arbiter(); break;
        case BS_BUSY_IO:   bus_service_device(io); break;
        case BS_BUSY_DMA0: bus_service_device(dma0); break;
        case BS_BUSY_DMA1: bus_service_device(dma1); break;
        case BS_BUSY_DMA2: bus_service_device(dma2); break;
        case BS_BUSY_DMA3: bus_service_device(dma3); break;
        default: break;
    }
}

#undef DEFINE_DEVICE_PORT
#undef DEFINE_DEVICE_PORTS
