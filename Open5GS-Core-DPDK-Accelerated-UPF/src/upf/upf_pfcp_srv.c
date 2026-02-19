#include "upf/upf_pfcp_srv.h"
#include "upf/session_table.h"
#include "pfcp/pfcp.h"
#include "pfcp/pfcp_rules.h"
#include "common/net.h"
#include "common/log.h"
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

static void send_rsp(upf_ctx_t* ctx, const udp_endpoint_t* peer, uint16_t type, uint32_t seq, uint64_t seid, uint32_t cause){
  pfcp_msg_t rsp={0};
  rsp.hdr.msg_type = type;
  rsp.hdr.seid_present = 1;
  rsp.hdr.seid = seid;
  rsp.hdr.seq  = seq;
  rsp.payload_len = 0;

  size_t cap = sizeof(rsp.payload);
  size_t n = pfcp_tlv_put_u32(rsp.payload, cap, IE_CAUSE, cause);
  rsp.payload_len += n;

  uint8_t buf[PFCP_MAX_MSG];
  size_t bl = sizeof(buf);
  if (!pfcp_encode(&rsp, buf, &bl)) return;
  net_udp_sendto(ctx->n4_fd, peer, buf, bl);
}

void upf_pfcp_handle(upf_ctx_t* ctx){
  uint8_t buf[PFCP_MAX_MSG];
  udp_endpoint_t peer;
  ssize_t n = net_udp_recvfrom(ctx->n4_fd, &peer, buf, sizeof(buf));
  if (n <= 0) return;

  pfcp_msg_t req={0};
  if (!pfcp_decode(&req, buf, (size_t)n)) return;

  ctx->last_smf_peer = peer;
  ctx->have_smf_peer = true;

  switch(req.hdr.msg_type){
    case PFCP_HEARTBEAT_REQ:
      LOGI("PFCP heartbeat req seq=%u", req.hdr.seq);
      send_rsp(ctx, &peer, PFCP_HEARTBEAT_RSP, req.hdr.seq, 0, 1);
      break;

    case PFCP_SESS_EST_REQ: {
      LOGI("PFCP sess est req seid=0x%lx seq=%u", (unsigned long)req.hdr.seid, req.hdr.seq);
      pfcp_session_rules_t rules; rules_init(&rules, req.hdr.seid);

      uint32_t ue=0, teid_ul=0, teid_dl=0;
      (void)pfcp_tlv_get_u32(req.payload, req.payload_len, IE_UE_IPV4, &ue);
      (void)pfcp_tlv_get_u32(req.payload, req.payload_len, IE_TEID, &teid_ul);
      (void)pfcp_tlv_get_u32(req.payload, req.payload_len, IE_TEID+1, &teid_dl);

      rules.pdr.ue_ipv4 = ue;
      rules.pdr.teid_ul = teid_ul;
      rules.pdr.teid_dl = teid_dl;

      // Demo FAR: forward to N6 peer
      rules.far.dst_ipv4 = inet_addr("127.0.0.1");
      rules.far.dst_port = 6001;

      if (!upf_sess_table_put(ctx->sess_table, &rules)){
        send_rsp(ctx, &peer, PFCP_SESS_EST_RSP, req.hdr.seq, req.hdr.seid, 64);
        break;
      }
      send_rsp(ctx, &peer, PFCP_SESS_EST_RSP, req.hdr.seq, req.hdr.seid, 1);
      break;
    }

    case PFCP_SESS_MOD_REQ:
      LOGI("PFCP sess mod req seid=0x%lx seq=%u", (unsigned long)req.hdr.seid, req.hdr.seq);
      if (!upf_sess_table_get(ctx->sess_table, req.hdr.seid, &(pfcp_session_rules_t){0})){
        send_rsp(ctx, &peer, PFCP_SESS_MOD_RSP, req.hdr.seq, req.hdr.seid, 64 /* not found */);
        break;
      }
      // Demo: accept update (placeholder for FAR/QER updates)
      send_rsp(ctx, &peer, PFCP_SESS_MOD_RSP, req.hdr.seq, req.hdr.seid, 1);
      break;

    case PFCP_SESS_DEL_REQ:
      LOGI("PFCP sess del req seid=0x%lx seq=%u", (unsigned long)req.hdr.seid, req.hdr.seq);
      (void)upf_sess_table_del(ctx->sess_table, req.hdr.seid);
      send_rsp(ctx, &peer, PFCP_SESS_DEL_RSP, req.hdr.seq, req.hdr.seid, 1);
      break;

    default:
      LOGW("Unhandled PFCP msg type=%u", req.hdr.msg_type);
      break;
  }
}
