#pragma once
#include <stdbool.h>

typedef struct {
  char log_level[16];

  // SMF
  char smf_n4_local[64];
  char smf_upf_peer[64];
  int  smf_pfcp_tx_timeout_ms;
  int  smf_pfcp_max_retries;

  // UPF
  char upf_n4_bind[64];
  char upf_n3_bind[64];
  char upf_n6_bind[64];
  char upf_n6_peer[64];

  // DPDK
  bool dpdk_enabled;
  char dpdk_eal_args[256];
  int  dpdk_port_id;
  int  dpdk_rx_queues;
  int  dpdk_tx_queues;
  int  dpdk_mbuf_cache;

  // Demo
  bool demo_enable;
  char demo_imsi[32];
  char demo_dnn[64];
  int  demo_sst;
  char demo_sd[16];
} app_config_t;

bool config_load_yaml_subset(const char* path, app_config_t* out);
