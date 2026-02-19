#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
  SMF_SESS_IDLE = 0,
  SMF_SESS_ESTABLISHED = 1,
} smf_sess_state_t;

typedef struct {
  char imsi[32];
  char dnn[64];
  uint64_t seid;
  uint32_t teid_ul;
  uint32_t teid_dl;
  uint32_t ue_ipv4; // network byte order
  smf_sess_state_t state;
} smf_session_t;

void smf_sess_init(smf_session_t* s, const char* imsi, const char* dnn, uint64_t seid);
