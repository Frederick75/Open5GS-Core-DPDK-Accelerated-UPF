#include "pfcp/pfcp.h"
#include <string.h>
#include <arpa/inet.h>

static uint64_t htonll_u64(uint64_t x){
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return ((uint64_t)htonl((uint32_t)(x & 0xFFFFFFFFULL)) << 32) | htonl((uint32_t)(x >> 32));
#else
  return x;
#endif
}
static uint64_t ntohll_u64(uint64_t x){ return htonll_u64(x); }

bool pfcp_encode(const pfcp_msg_t* m, uint8_t* out, size_t* inout_len){
  if (!m || !out || !inout_len) return false;
  size_t need = sizeof(pfcp_hdr_t) + m->payload_len;
  if (*inout_len < need) return false;

  pfcp_hdr_t h = m->hdr;
  h.version = 1;
  h.msg_len = htons((uint16_t)m->payload_len);
  h.msg_type = htons((uint16_t)h.msg_type);
  h.seid_present = htons((uint16_t)h.seid_present);
  h.seid = htonll_u64(h.seid);
  h.seq  = htonl(h.seq);

  memcpy(out, &h, sizeof(h));
  memcpy(out + sizeof(h), m->payload, m->payload_len);
  *inout_len = need;
  return true;
}

bool pfcp_decode(pfcp_msg_t* m, const uint8_t* buf, size_t len){
  if (!m || !buf || len < sizeof(pfcp_hdr_t)) return false;
  pfcp_hdr_t h; memcpy(&h, buf, sizeof(h));

  m->hdr = h;
  m->hdr.msg_type = ntohs(h.msg_type);
  m->hdr.msg_len  = ntohs(h.msg_len);
  m->hdr.seid_present = ntohs(h.seid_present);
  m->hdr.seid = ntohll_u64(h.seid);
  m->hdr.seq  = ntohl(h.seq);

  size_t pay = m->hdr.msg_len;
  if (sizeof(pfcp_hdr_t) + pay > len) return false;
  if (pay > sizeof(m->payload)) return false;

  memcpy(m->payload, buf + sizeof(pfcp_hdr_t), pay);
  m->payload_len = pay;
  return true;
}

// TLV format: type(2) len(2) value(n)
size_t pfcp_tlv_put_u32(uint8_t* b, size_t cap, uint16_t t, uint32_t v){
  uint32_t nv = htonl(v);
  return pfcp_tlv_put_buf(b, cap, t, &nv, sizeof(nv));
}
size_t pfcp_tlv_put_u64(uint8_t* b, size_t cap, uint16_t t, uint64_t v){
  uint64_t nv = htonll_u64(v);
  return pfcp_tlv_put_buf(b, cap, t, &nv, sizeof(nv));
}
size_t pfcp_tlv_put_buf(uint8_t* b, size_t cap, uint16_t t, const void* p, uint16_t n){
  if (!b || !p) return 0;
  if (cap < (size_t)(4 + n)) return 0;
  uint16_t nt = htons(t);
  uint16_t nl = htons(n);
  memcpy(b, &nt, 2);
  memcpy(b+2, &nl, 2);
  memcpy(b+4, p, n);
  return 4 + n;
}

static bool tlv_find(const uint8_t* b, size_t len, uint16_t t, const uint8_t** val, uint16_t* vlen){
  size_t off = 0;
  while(off + 4 <= len){
    uint16_t rt; memcpy(&rt, b+off, 2); rt = ntohs(rt);
    uint16_t rl; memcpy(&rl, b+off+2, 2); rl = ntohs(rl);
    if (off + 4 + rl > len) return false;
    if (rt == t){
      *val = b + off + 4;
      *vlen = rl;
      return true;
    }
    off += 4 + rl;
  }
  return false;
}

bool pfcp_tlv_get_u32(const uint8_t* b, size_t len, uint16_t t, uint32_t* out){
  const uint8_t* v; uint16_t vl;
  if (!tlv_find(b,len,t,&v,&vl) || vl != 4) return false;
  uint32_t nv; memcpy(&nv, v, 4); *out = ntohl(nv); return true;
}
bool pfcp_tlv_get_u64(const uint8_t* b, size_t len, uint16_t t, uint64_t* out){
  const uint8_t* v; uint16_t vl;
  if (!tlv_find(b,len,t,&v,&vl) || vl != 8) return false;
  uint64_t nv; memcpy(&nv, v, 8); *out = ntohll_u64(nv); return true;
}
