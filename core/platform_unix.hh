#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>

struct udp_socket {
  int sock = -1;
  udp_socket(const char * host, const char * port);
  udp_socket(const char * host, const char * port, bool listen);
  void send(void * message, std::size_t len);
  size_t read(char * buf, size_t len);
};

namespace {
#include <arpa/inet.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
}

struct addrinfo *getAddrInfo(const char *host, const char *port) {
    struct addrinfo hints, *servinfo=0;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo(host, port, &hints, &servinfo)) exit(2);
    return servinfo;
}

udp_socket::udp_socket(
  const char * host,
  const char * port,
  __attribute__((unused)) bool listen)
{
  int optval = 1;
  auto servinfo = getAddrInfo(host, port);
  sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  printf("udp socket %d\n", sock);
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof (int));

  if (bind(sock, servinfo->ai_addr, servinfo->ai_addrlen)) {
    exit(fprintf(stderr, "could not bind socket"));
  }
  freeaddrinfo(servinfo);
}

udp_socket::udp_socket(const char * host, const char * port) {
  auto servinfo = getAddrInfo(host, port);
  sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  connect(sock, servinfo->ai_addr, servinfo->ai_addrlen);
  freeaddrinfo(servinfo);
}

size_t udp_socket::read(char * buf, size_t len) {
  fd_set read_set;
  FD_ZERO(&read_set);
  FD_SET(sock, &read_set);
  timeval timeout = { 0, 0 }; // no timeout
  if (select(sock + 1, &read_set, 0, 0, &timeout) > 0) {
    return recv(sock, buf, len, 0);
  }
  return 0;
}

void udp_socket::send(void * message, size_t len) {
  ::send(sock, message, len, 0);
}
