#ifndef _BUS_H_
#define _BUS_H_

#include "systemc.h"
#include "sysreqs.h"

#define DECLARE_DEVICE_PORTS(dev) \
    sc_in<bool> dev##_wr_i; \
    sc_in<bool> dev##_rd_i; \
    sc_in<sc_uint<SYS_RQ_MEMADDR_WIDTH>> dev##_addr_bi; \
    sc_in<bool> dev##_brq_i; \
    sc_vector<sc_in<double>> dev##_data_bi; \
    sc_out<bool> dev##_bgt_o; \
    sc_vector<sc_out<double>> dev##_data_bo


enum bus_states {
    BS_IDLE = 0,
    BS_BUSY_IO,
    BS_BUSY_DMA1,
    BS_BUSY_DMA2,
    BS_BUSY_DMA3,
    BS_BUSY_DMA4
};


SC_MODULE(Bus)
{
    sc_in<bool> bus_clk_i;
    DECLARE_DEVICE_PORTS(io);
    DECLARE_DEVICE_PORTS(dma1);
    DECLARE_DEVICE_PORTS(dma2);
    DECLARE_DEVICE_PORTS(dma3);
    DECLARE_DEVICE_PORTS(dma4);
    sc_out<sc_uint<SYS_RQ_MEMADDR_WIDTH>> bus_addr_bo;
    sc_vector<sc_in<double>> bus_data_bi;
    sc_vector<sc_out<double>> bus_data_bo;
    sc_out<bool> bus_wr_o;
    sc_out<bool> bus_rd_o;

    SC_HAS_PROCESS(Bus);

    Bus(sc_module_name nm);
    ~Bus();

    void bus_arbiter();
    void bus_service(
        sc_in<bool> &dev_brq_i,
        sc_in<bool> &dev_wr_i,
        sc_in<bool> &dev_rd_i,
        sc_in<sc_uint<SYS_RQ_MEMADDR_WIDTH>> &dev_addr_bi,
        sc_vector<sc_in<double>> &dev_data_bi,
        sc_out<bool> &dev_bgt_o,
        sc_vector<sc_out<double>> &dev_data_bo);
    void bus_routine();

private:
    enum bus_states state;

};

#undef DECLARE_DEVICE_PORTS

#endif // _BUS_H_
