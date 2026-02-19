#include "upf/upf_n6.h"
#include "common/net.h"
#include "common/log.h"
#include "upf/session_table.h"
#include <string.h>
#include <arpa/inet.h>

// Downlink demo: receive UDP from N6 and wrap into GTP-U to N3 peer (learned from last packet if desired).
typedef struct {
  uint8_t flags;
  uint8_t msgtype;
  uint16_t length;
  uint32_t teid;
} __attribute__((packed)) gtpu_hdr_t;

void upf_n6_handle(upf_ctx_t* ctx){
  uint8_t payload[1400];
  udp_endpoint_t peer;
  ssize_t n = net_udp_recvfrom(ctx->n6_fd, &peer, payload, sizeof(payload));
  if (n <= 0) return;


  // For demo: send downlink to whoever last sent N3 traffic is not tracked; this is a placeholder.
  // In production: track gNB/ULCL, UE address mapping, PDR lookup, etc.
  // We'll just log it.
  // Downlink classification is environment-specific; for demo we attribute DL bytes to all sessions aggregate.
  // In production, classify using PDR (UE IP, QFI, TEID-DL, etc.).
  (void)peer;
  uint64_t ul=0, dl=0;
  upf_sess_table_agg_urr(ctx->sess_table, &ul, &dl);
  LOGD("N6 received %zd bytes (downlink demo). Current agg UL=%lu DL=%lu", n, (unsigned long)ul, (unsigned long)dl);

}
