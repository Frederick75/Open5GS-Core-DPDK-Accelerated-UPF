#include "upf/upf_gtpu.h"
#include "upf/session_table.h"
#include "common/log.h"
#include "common/net.h"
#include <string.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <inttypes.h>

static void upf_add_ul_bytes(pfcp_session_rules_t* r, void* user){
  uint64_t add = (uint64_t)(uintptr_t)user;
  r->urr.uplink_bytes += add;
}


// Minimal GTP-U header (simplified)
typedef struct {
  uint8_t flags;
  uint8_t msgtype;
  uint16_t length;
  uint32_t teid;
} __attribute__((packed)) gtpu_hdr_t;

void upf_gtpu_handle_n3(upf_ctx_t* ctx){
  uint8_t pkt[2048];
  udp_endpoint_t peer;
  ssize_t n = net_udp_recvfrom(ctx->n3_fd, &peer, pkt, sizeof(pkt));
  if (n <= (ssize_t)sizeof(gtpu_hdr_t)) return;

  
  gtpu_hdr_t h; memcpy(&h, pkt, sizeof(h));
  uint32_t teid = ntohl(h.teid);

  pfcp_session_rules_t rules;
  if (!upf_sess_table_find_by_teid_ul(ctx->sess_table, teid, &rules)){
    LOGW("N3 packet dropped (no session for TEID=0x%x)", teid);
    return;
  }

  // Forward inner payload to N6 peer (demo UDP forward)
  const uint8_t* inner = pkt + sizeof(gtpu_hdr_t);
  size_t inner_len = (size_t)n - sizeof(gtpu_hdr_t);

  struct sockaddr_in dst = {0};
  dst.sin_family = AF_INET;
  dst.sin_addr.s_addr = rules.far.dst_ipv4;
  dst.sin_port = htons(rules.far.dst_port);
  sendto(ctx->n6_fd, inner, inner_len, 0, (struct sockaddr*)&dst, sizeof(dst));

  // Update URR for this session
  upf_sess_table_update(ctx->sess_table, rules.seid, 
    upf_add_ul_bytes,
    (void*)(uintptr_t)inner_len);

  LOGD("N3->N6 forwarded %zu bytes (teid=0x%x)", inner_len, teid);
}
