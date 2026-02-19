#include "common/net.h"
#include "common/log.h"
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

bool net_parse_ip_port(const char* s, udp_endpoint_t* out){
  if (!s || !out) return false;
  char tmp[128]; strncpy(tmp, s, sizeof(tmp)-1); tmp[sizeof(tmp)-1]='\0';
  char* colon = strrchr(tmp, ':');
  if (!colon) return false;
  *colon = '\0';
  const char* ip = tmp;
  const char* port_s = colon+1;
  int port = atoi(port_s);
  if (port <= 0 || port > 65535) return false;

  memset(out, 0, sizeof(*out));
  out->addr.sin_family = AF_INET;
  out->addr.sin_port = htons((uint16_t)port);
  if (inet_pton(AF_INET, ip, &out->addr.sin_addr) != 1) return false;
  return true;
}

int net_udp_bind(const udp_endpoint_t* ep){
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0){ LOGE("socket() failed: %s", strerror(errno)); return -1; }
  int on=1; setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if (bind(fd, (const struct sockaddr*)&ep->addr, sizeof(ep->addr)) < 0){
    LOGE("bind() failed: %s", strerror(errno)); close(fd); return -1;
  }
  return fd;
}

int net_udp_connect(const udp_endpoint_t* peer){
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0){ LOGE("socket() failed: %s", strerror(errno)); return -1; }
  if (connect(fd, (const struct sockaddr*)&peer->addr, sizeof(peer->addr)) < 0){
    LOGE("connect() failed: %s", strerror(errno)); close(fd); return -1;
  }
  return fd;
}

ssize_t net_udp_sendto(int fd, const udp_endpoint_t* peer, const void* buf, size_t len){
  return sendto(fd, buf, len, 0, (const struct sockaddr*)&peer->addr, sizeof(peer->addr));
}

ssize_t net_udp_recvfrom(int fd, udp_endpoint_t* peer, void* buf, size_t len){
  socklen_t sl = sizeof(peer->addr);
  ssize_t n = recvfrom(fd, buf, len, 0, (struct sockaddr*)&peer->addr, &sl);
  return n;
}
