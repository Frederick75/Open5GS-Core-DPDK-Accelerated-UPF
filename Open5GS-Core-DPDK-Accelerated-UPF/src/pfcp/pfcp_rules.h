#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint32_t teid_ul;
  uint32_t teid_dl;
  uint32_t ue_ipv4;  // network byte order
} pdr_t;

typedef struct {
  uint32_t dst_ipv4; // network byte order
  uint16_t dst_port;
} far_t;

typedef struct {
  uint32_t qfi;
  uint64_t max_bitrate_ul;
  uint64_t max_bitrate_dl;
} qer_t;

typedef struct {
  uint64_t uplink_bytes;
  uint64_t downlink_bytes;
} urr_t;

typedef struct {
  uint64_t seid;
  pdr_t pdr;
  far_t far;
  qer_t qer;
  urr_t urr;
} pfcp_session_rules_t;

void rules_init(pfcp_session_rules_t* r, uint64_t seid);
