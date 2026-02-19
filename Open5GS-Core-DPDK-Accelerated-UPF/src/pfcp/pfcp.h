#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define PFCP_PORT 8805
#define PFCP_MAX_MSG 2048

typedef enum {
  PFCP_HEARTBEAT_REQ = 1,
  PFCP_HEARTBEAT_RSP = 2,
  PFCP_SESS_EST_REQ  = 50,
  PFCP_SESS_EST_RSP  = 51,
  PFCP_SESS_MOD_REQ  = 52,
  PFCP_SESS_MOD_RSP  = 53,
  PFCP_SESS_DEL_REQ  = 54,
  PFCP_SESS_DEL_RSP  = 55,
} pfcp_msg_type_t;

typedef struct {
  uint8_t  version;
  uint8_t  spare;
  uint16_t msg_type;
  uint16_t msg_len;
  uint16_t seid_present;
  uint64_t seid;       // simplified
  uint32_t seq;        // simplified
} __attribute__((packed)) pfcp_hdr_t;

typedef struct {
  pfcp_hdr_t hdr;
  uint8_t payload[PFCP_MAX_MSG - sizeof(pfcp_hdr_t)];
  size_t payload_len;
} pfcp_msg_t;

uint32_t pfcp_next_seq(void);

bool pfcp_encode(const pfcp_msg_t* m, uint8_t* out, size_t* inout_len);
bool pfcp_decode(pfcp_msg_t* m, const uint8_t* buf, size_t len);

// Minimal TLV helpers for demo rule conveyance
typedef enum {
  IE_NODE_ID    = 60,
  IE_CAUSE      = 19,
  IE_FSEID      = 57,
  IE_PDR        = 1,
  IE_FAR        = 2,
  IE_QER        = 3,
  IE_URR        = 4,
  IE_TEID       = 1001, // demo-only
  IE_UE_IPV4    = 1002, // demo-only
} pfcp_ie_type_t;

size_t pfcp_tlv_put_u32(uint8_t* b, size_t cap, uint16_t t, uint32_t v);
size_t pfcp_tlv_put_u64(uint8_t* b, size_t cap, uint16_t t, uint64_t v);
size_t pfcp_tlv_put_buf(uint8_t* b, size_t cap, uint16_t t, const void* p, uint16_t n);

bool   pfcp_tlv_get_u32(const uint8_t* b, size_t len, uint16_t t, uint32_t* out);
bool   pfcp_tlv_get_u64(const uint8_t* b, size_t len, uint16_t t, uint64_t* out);
