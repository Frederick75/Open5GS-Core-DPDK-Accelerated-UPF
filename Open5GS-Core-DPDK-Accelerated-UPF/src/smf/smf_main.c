#include "smf/smf.h"
#include "common/config.h"
#include "common/log.h"
#include <stdio.h>
#include <string.h>

static log_level_t parse_lvl(const char* s){
  if (!s) return LOG_INFO;
  if (strcmp(s,"error")==0) return LOG_ERROR;
  if (strcmp(s,"warn")==0)  return LOG_WARN;
  if (strcmp(s,"info")==0)  return LOG_INFO;
  if (strcmp(s,"debug")==0) return LOG_DEBUG;
  return LOG_INFO;
}

int main(int argc, char** argv){
  const char* cfg_path = "configs/smf.yaml";
  int demo_pdu = 0;

  for (int i=1;i<argc;i++){
    if (strcmp(argv[i],"-c")==0 && i+1<argc) cfg_path = argv[++i];
    else if (strcmp(argv[i],"--demo-pdu")==0) demo_pdu = 1;
  }

  app_config_t cfg;
  if (!config_load_yaml_subset(cfg_path, &cfg)) return 2;
  log_init(parse_lvl(cfg.log_level));

  LOGI("SMF starting (N4 local=%s, UPF peer=%s)", cfg.smf_n4_local, cfg.smf_upf_peer);
  return smf_run(&cfg, demo_pdu);
}
