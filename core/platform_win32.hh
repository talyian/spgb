#pragma once
#include <cstdint>

struct udp_socket {
  udp_socket(const char * host, const char * port);
  void send(void * message, std::size_t len);
};

namespace {
#include <winsock2.h>
#include <windows.h>
}

udp_socket::udp_socket(const char *host, const char *port) {
  fprintf(stderr, "warning: sockets not enabled in windows yet\n");
}

void udp_socket::send(void * message, std::size_t len) {
  fprintf(stderr, "warning: sockets not enabled in windows yet\n");
}
