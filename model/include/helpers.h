#ifndef _HELPERS_H_
#define _HELPERS_H_

#define STRINGIFY_PORT(port) #port
#define DEFINE_PORT(port) port(STRINGIFY_PORT(port))
#define DEFINE_PORTVEC(port, size) port(STRINGIFY_PORT(port), (size))

#endif // _HELPERS_H_