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

#endif // _HELPERS_H_
