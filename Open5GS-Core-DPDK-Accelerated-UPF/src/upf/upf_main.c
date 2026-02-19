#include "upf/upf.h"
#include "common/config.h"
#include "common/log.h"
#include "common/net.h"
#include "upf/upf_pfcp_srv.h"
#include "upf/upf_gtpu.h"
#include "upf/upf_n6.h"
#include "upf/upf_dataplane.h"
#include "upf/session_table.h"
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdio.h>

static log_level_t parse_lvl(const char* s){
  if (!s) return LOG_INFO;
  if (strcmp(s,"error")==0) return LOG_ERROR;
  if (strcmp(s,"warn")==0)  return LOG_WARN;
  if (strcmp(s,"info")==0)  return LOG_INFO;
  if (strcmp(s,"debug")==0) return LOG_DEBUG;
  return LOG_INFO;
}

int main(int argc, char** argv){
  const char* cfg_path = "configs/upf.yaml";
  for (int i=1;i<argc;i++){
    if (strcmp(argv[i],"-c")==0 && i+1<argc) cfg_path = argv[++i];
  }

  upf_ctx_t ctx; memset(&ctx, 0, sizeof(ctx));
  if (!config_load_yaml_subset(cfg_path, &ctx.cfg)) return 2;
  log_init(parse_lvl(ctx.cfg.log_level));

  udp_endpoint_t n4, n3, n6;
  if (!net_parse_ip_port(ctx.cfg.upf_n4_bind, &n4)) return 2;
  if (!net_parse_ip_port(ctx.cfg.upf_n3_bind, &n3)) return 2;
  if (!net_parse_ip_port(ctx.cfg.upf_n6_bind, &n6)) return 2;

  ctx.n4_fd = net_udp_bind(&n4);
  ctx.n3_fd = net_udp_bind(&n3);
  ctx.n6_fd = net_udp_bind(&n6);
  if (ctx.n4_fd < 0 || ctx.n3_fd < 0 || ctx.n6_fd < 0) return 3;

  // Create session table (default capacity 4096)
  ctx.sess_table = upf_sess_table_create(4096);
  if (!ctx.sess_table){ LOGE("session table alloc failed"); return 4; }

  // Initialize dataplane
  if (!upf_dataplane_init(&ctx)){
    LOGE("Dataplane init failed");
    return 4;
  }

  LOGI("UPF started: N4=%s N3=%s N6=%s (dpdk=%s)",
       ctx.cfg.upf_n4_bind, ctx.cfg.upf_n3_bind, ctx.cfg.upf_n6_bind,
       ctx.dpdk_active ? "on" : "off");

  // Main select loop for control + socket dataplane.
  // If DPDK is active, dataplane polling happens inside upf_dataplane_poll().
  while(1){
    fd_set rfds; FD_ZERO(&rfds);
    int maxfd = 0;
    FD_SET(ctx.n4_fd, &rfds); if (ctx.n4_fd > maxfd) maxfd = ctx.n4_fd;
    if (!ctx.dpdk_active){
      FD_SET(ctx.n3_fd, &rfds); if (ctx.n3_fd > maxfd) maxfd = ctx.n3_fd;
      FD_SET(ctx.n6_fd, &rfds); if (ctx.n6_fd > maxfd) maxfd = ctx.n6_fd;
    }

    struct timeval tv = { .tv_sec = 0, .tv_usec = 10000 };
    int rc = select(maxfd+1, &rfds, NULL, NULL, &tv);
    if (rc < 0) continue;

    if (FD_ISSET(ctx.n4_fd, &rfds)) upf_pfcp_handle(&ctx);
    if (!ctx.dpdk_active){
      if (FD_ISSET(ctx.n3_fd, &rfds)) upf_gtpu_handle_n3(&ctx);
      if (FD_ISSET(ctx.n6_fd, &rfds)) upf_n6_handle(&ctx);
    } else {
      upf_dataplane_poll(&ctx);
    }
  }

  return 0;
}
