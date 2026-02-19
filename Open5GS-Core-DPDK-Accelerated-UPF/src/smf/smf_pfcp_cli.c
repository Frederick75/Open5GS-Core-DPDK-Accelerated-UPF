#include "smf/smf_pfcp_cli.h"
#include "pfcp/pfcp.h"
#include "common/net.h"
#include "common/log.h"
#include "common/timeutil.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

static bool xact(const app_config_t* cfg, const udp_endpoint_t* peer,
                 pfcp_msg_t* req, pfcp_msg_t* rsp){
  uint8_t buf[PFCP_MAX_MSG];
  size_t bl = sizeof(buf);
  if (!pfcp_encode(req, buf, &bl)) return false;

  int fd = net_udp_connect(peer);
  if (fd < 0) return false;

  for (int attempt=0; attempt<=cfg->smf_pfcp_max_retries; attempt++){
    ssize_t n = send(fd, buf, bl, 0);
    if (n < 0){ LOGE("PFCP send failed: %s", strerror(errno)); close(fd); return false; }

    uint64_t start = time_now_ms();
    while(time_now_ms() - start < (uint64_t)cfg->smf_pfcp_tx_timeout_ms){
      uint8_t rbuf[PFCP_MAX_MSG];
      ssize_t rn = recv(fd, rbuf, sizeof(rbuf), MSG_DONTWAIT);
      if (rn < 0){
        if (errno==EAGAIN || errno==EWOULDBLOCK){ usleep(2000); continue; }
        LOGE("PFCP recv failed: %s", strerror(errno)); close(fd); return false;
      }
      if (!pfcp_decode(rsp, rbuf, (size_t)rn)) continue;
      if (rsp->hdr.seq == req->hdr.seq) { close(fd); return true; }
    }
    LOGW("PFCP timeout (attempt %d)", attempt+1);
  }
  close(fd);
  return false;
}

bool smf_pfcp_heartbeat(const app_config_t* cfg){
  udp_endpoint_t peer; if (!net_parse_ip_port(cfg->smf_upf_peer, &peer)) return false;
  pfcp_msg_t req={0}, rsp={0};
  req.hdr.msg_type = PFCP_HEARTBEAT_REQ;
  req.hdr.seid_present = 0;
  req.hdr.seid = 0;
  req.hdr.seq  = pfcp_next_seq();

  req.payload_len = 0;
  bool ok = xact(cfg, &peer, &req, &rsp);
  if (!ok) return false;
  LOGI("PFCP heartbeat rsp type=%u", rsp.hdr.msg_type);
  return (rsp.hdr.msg_type == PFCP_HEARTBEAT_RSP);
}

static void put_u32(pfcp_msg_t* m, uint16_t ie, uint32_t v){
  size_t cap = sizeof(m->payload) - m->payload_len;
  size_t n = pfcp_tlv_put_u32(m->payload + m->payload_len, cap, ie, v);
  m->payload_len += n;
}
static void put_u64(pfcp_msg_t* m, uint16_t ie, uint64_t v){
  size_t cap = sizeof(m->payload) - m->payload_len;
  size_t n = pfcp_tlv_put_u64(m->payload + m->payload_len, cap, ie, v);
  m->payload_len += n;
}

bool smf_pfcp_session_establish(const app_config_t* cfg, smf_session_t* sess){
  udp_endpoint_t peer; if (!net_parse_ip_port(cfg->smf_upf_peer, &peer)) return false;
  pfcp_msg_t req={0}, rsp={0};
  req.hdr.msg_type = PFCP_SESS_EST_REQ;
  req.hdr.seid_present = 1;
  req.hdr.seid = sess->seid;
  req.hdr.seq  = pfcp_next_seq();
  req.payload_len = 0;

  // Demo IEs: UE IPv4, TEIDs
  sess->ue_ipv4 = inet_addr("10.45.0.2");
  sess->teid_ul = 0x1001;
  sess->teid_dl = 0x2001;

  put_u32(&req, IE_UE_IPV4, sess->ue_ipv4);
  put_u32(&req, IE_TEID, sess->teid_ul);
  put_u32(&req, IE_TEID+1, sess->teid_dl);

  bool ok = xact(cfg, &peer, &req, &rsp);
  if (!ok) return false;

  uint32_t cause=0;
  pfcp_tlv_get_u32(rsp.payload, rsp.payload_len, IE_CAUSE, &cause);
  LOGI("PFCP sess est rsp cause=%u", cause);
  return (rsp.hdr.msg_type == PFCP_SESS_EST_RSP && cause == 1);
}

bool smf_pfcp_session_modify(const app_config_t* cfg, smf_session_t* sess){
  udp_endpoint_t peer; if (!net_parse_ip_port(cfg->smf_upf_peer, &peer)) return false;
  pfcp_msg_t req={0}, rsp={0};
  req.hdr.msg_type = PFCP_SESS_MOD_REQ;
  req.hdr.seid_present = 1;
  req.hdr.seid = sess->seid;
  req.hdr.seq  = pfcp_next_seq();
  req.payload_len = 0;

  // Demo: update QER/QFI
  put_u64(&req, IE_QER, 9);

  bool ok = xact(cfg, &peer, &req, &rsp);
  if (!ok) return false;

  uint32_t cause=0;
  pfcp_tlv_get_u32(rsp.payload, rsp.payload_len, IE_CAUSE, &cause);
  LOGI("PFCP sess mod rsp cause=%u", cause);
  return (rsp.hdr.msg_type == PFCP_SESS_MOD_RSP && cause == 1);
}

bool smf_pfcp_session_delete(const app_config_t* cfg, smf_session_t* sess){
  udp_endpoint_t peer; if (!net_parse_ip_port(cfg->smf_upf_peer, &peer)) return false;
  pfcp_msg_t req={0}, rsp={0};
  req.hdr.msg_type = PFCP_SESS_DEL_REQ;
  req.hdr.seid_present = 1;
  req.hdr.seid = sess->seid;
  req.hdr.seq  = pfcp_next_seq();
  req.payload_len = 0;

  bool ok = xact(cfg, &peer, &req, &rsp);
  if (!ok) return false;

  uint32_t cause=0;
  pfcp_tlv_get_u32(rsp.payload, rsp.payload_len, IE_CAUSE, &cause);
  LOGI("PFCP sess del rsp cause=%u", cause);
  return (rsp.hdr.msg_type == PFCP_SESS_DEL_RSP && cause == 1);
}
