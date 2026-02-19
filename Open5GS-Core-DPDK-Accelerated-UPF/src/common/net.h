#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>

typedef struct {
  struct sockaddr_in addr;
} udp_endpoint_t;

bool net_parse_ip_port(const char* s, udp_endpoint_t* out);
int  net_udp_bind(const udp_endpoint_t* ep);
int  net_udp_connect(const udp_endpoint_t* peer);
ssize_t net_udp_sendto(int fd, const udp_endpoint_t* peer, const void* buf, size_t len);
ssize_t net_udp_recvfrom(int fd, udp_endpoint_t* peer, void* buf, size_t len);
