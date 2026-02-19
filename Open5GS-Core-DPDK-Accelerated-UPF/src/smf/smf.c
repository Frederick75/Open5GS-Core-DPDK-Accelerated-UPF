#include "smf/smf.h"
#include "smf/smf_pfcp_cli.h"
#include "smf/smf_sessions.h"
#include "common/log.h"
#include <unistd.h>

int smf_run(const app_config_t* cfg, int demo_pdu){
  if (!cfg) return 2;

  if (!smf_pfcp_heartbeat(cfg)){
    LOGE("UPF heartbeat failed");
    return 3;
  }

  if (demo_pdu || cfg->demo_enable){
    smf_session_t s;
    smf_sess_init(&s, cfg->demo_imsi, cfg->demo_dnn, 0xABCDEF0011223344ULL);

    LOGI("Demo: UE-initiated PDU session establish (IMSI=%s, DNN=%s)", s.imsi, s.dnn);
    if (!smf_pfcp_session_establish(cfg, &s)) return 4;
    s.state = SMF_SESS_ESTABLISHED;

    LOGI("Demo: PDU session modify");
    if (!smf_pfcp_session_modify(cfg, &s)) return 5;

    LOGI("Demo: PDU session delete");
    if (!smf_pfcp_session_delete(cfg, &s)) return 6;

    LOGI("Demo complete");
    return 0;
  }

  LOGI("SMF running (no demo). Press Ctrl+C to exit.");
  while(1){ sleep(1); }
  return 0;
}
