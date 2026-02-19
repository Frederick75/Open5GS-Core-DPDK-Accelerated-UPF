#pragma once
#include "common/config.h"
#include "common/net.h"
#include "pfcp/pfcp_rules.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  app_config_t cfg;

  // PFCP server socket
  int n4_fd;

  // GTP-U (N3)
  int n3_fd;

  // N6 demo forward socket
  int n6_fd;

  // Session table (carrier-grade multi-session)
  struct upf_sess_table* sess_table;

  // PFCP peer (SMF) learned from last request
  udp_endpoint_t last_smf_peer;
  bool have_smf_peer;

  // DPDK
  bool dpdk_active;
} upf_ctx_t;
