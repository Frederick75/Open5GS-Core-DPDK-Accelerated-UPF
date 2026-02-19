#include "upf/upf_dataplane.h"
#include "common/log.h"
#include <string.h>

#ifdef USE_DPDK
#include <rte_eal.h>
#include <rte_ethdev.h>
#endif

#ifdef USE_DPDK
static int split_args(const char* s, char* argv[], int max){
  // Very small splitter; respects simple spaces (no full shell parsing).
  static char buf[512];
  strncpy(buf, s ? s : "", sizeof(buf)-1);
  buf[sizeof(buf)-1]='\0';

  int argc=0;
  char* p = buf;
  while(*p && argc < max){
    while(*p==' ') p++;
    if (!*p) break;
    argv[argc++] = p;
    while(*p && *p!=' ') p++;
    if (*p){ *p='\0'; p++; }
  }
  return argc;
}

#endif

bool upf_dataplane_init(upf_ctx_t* ctx){
  if (!ctx) return false;

#ifdef USE_DPDK
  if (ctx->cfg.dpdk_enabled){
    char* argv[32];
    int argc = split_args(ctx->cfg.dpdk_eal_args, argv, 32);
    // prepend a fake argv[0]
    char* eal_argv[33];
    eal_argv[0] = "upf";
    for(int i=0;i<argc;i++) eal_argv[i+1]=argv[i];
    int eal_argc = argc+1;

    int rc = rte_eal_init(eal_argc, eal_argv);
    if (rc < 0){
      LOGE("DPDK EAL init failed");
      ctx->dpdk_active = false;
      return false;
    }
    LOGI("DPDK enabled (EAL ok). port_id=%d", ctx->cfg.dpdk_port_id);
    ctx->dpdk_active = true;

    // Production: configure rte_eth_dev, queues, mempools, RSS, offloads.
    // This reference keeps initialization minimal and focuses on the project structure.
    return true;
  }
#endif

  ctx->dpdk_active = false;
  LOGI("DPDK disabled -> using socket dataplane");
  return true;
}

void upf_dataplane_poll(upf_ctx_t* ctx){
  if (!ctx || !ctx->dpdk_active) return;
#ifdef USE_DPDK
  // Production: burst RX, classify via PDR, apply FAR/QER, encaps/decaps GTP-U, burst TX.
  // Here: placeholder tick.
  (void)ctx;
#endif
}
