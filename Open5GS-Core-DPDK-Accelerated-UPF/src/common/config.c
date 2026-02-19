#include "common/config.h"
#include "common/log.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static void safe_strcpy(char* dst, size_t cap, const char* src){
  if (!dst || cap==0) return;
  if (!src) { dst[0]='\0'; return; }
  size_t n = strnlen(src, cap-1);
  memcpy(dst, src, n);
  dst[n] = '\0';
}

static void cfg_defaults(app_config_t* c){
  memset(c, 0, sizeof(*c));
  strncpy(c->log_level, "info", sizeof(c->log_level)-1);

  strncpy(c->smf_n4_local, "0.0.0.0:8805", sizeof(c->smf_n4_local)-1);
  strncpy(c->smf_upf_peer, "127.0.0.1:8806", sizeof(c->smf_upf_peer)-1);
  c->smf_pfcp_tx_timeout_ms = 500;
  c->smf_pfcp_max_retries = 3;

  strncpy(c->upf_n4_bind, "0.0.0.0:8806", sizeof(c->upf_n4_bind)-1);
  strncpy(c->upf_n3_bind, "0.0.0.0:2152", sizeof(c->upf_n3_bind)-1);
  strncpy(c->upf_n6_bind, "0.0.0.0:6000", sizeof(c->upf_n6_bind)-1);
  strncpy(c->upf_n6_peer, "127.0.0.1:6001", sizeof(c->upf_n6_peer)-1);

  c->dpdk_enabled = false;
  strncpy(c->dpdk_eal_args, "-l 2-5 -n 4 --proc-type=auto", sizeof(c->dpdk_eal_args)-1);
  c->dpdk_port_id = 0;
  c->dpdk_rx_queues = 2;
  c->dpdk_tx_queues = 2;
  c->dpdk_mbuf_cache = 256;

  c->demo_enable = false;
  strncpy(c->demo_imsi, "001010123456789", sizeof(c->demo_imsi)-1);
  strncpy(c->demo_dnn, "internet", sizeof(c->demo_dnn)-1);
  c->demo_sst = 1;
  strncpy(c->demo_sd, "010203", sizeof(c->demo_sd)-1);
}

static char* ltrim(char* s){ while(*s && isspace((unsigned char)*s)) s++; return s; }
static void rtrim(char* s){
  size_t n = strlen(s);
  while(n>0 && isspace((unsigned char)s[n-1])){ s[n-1]='\0'; n--; }
}

static bool streq(const char* a, const char* b){ return a && b && strcmp(a,b)==0; }

static void set_kv(app_config_t* c, const char* key, const char* val){
  if (streq(key,"smf.n4_local")) strncpy(c->smf_n4_local, val, sizeof(c->smf_n4_local)-1);
  else if (streq(key,"smf.upf_n4_peer")) strncpy(c->smf_upf_peer, val, sizeof(c->smf_upf_peer)-1);
  else if (streq(key,"smf.pfcp.tx_timeout_ms")) c->smf_pfcp_tx_timeout_ms = atoi(val);
  else if (streq(key,"smf.pfcp.max_retries")) c->smf_pfcp_max_retries = atoi(val);
  else if (streq(key,"smf.log_level")) strncpy(c->log_level, val, sizeof(c->log_level)-1);

  else if (streq(key,"upf.n4_bind")) strncpy(c->upf_n4_bind, val, sizeof(c->upf_n4_bind)-1);
  else if (streq(key,"upf.n3_bind")) strncpy(c->upf_n3_bind, val, sizeof(c->upf_n3_bind)-1);
  else if (streq(key,"upf.n6_bind")) strncpy(c->upf_n6_bind, val, sizeof(c->upf_n6_bind)-1);
  else if (streq(key,"upf.n6_peer")) strncpy(c->upf_n6_peer, val, sizeof(c->upf_n6_peer)-1);
  else if (streq(key,"upf.log_level")) strncpy(c->log_level, val, sizeof(c->log_level)-1);

  else if (streq(key,"upf.dpdk.enabled")) c->dpdk_enabled = (strcmp(val,"true")==0 || strcmp(val,"1")==0);
  else if (streq(key,"upf.dpdk.eal_args")) strncpy(c->dpdk_eal_args, val, sizeof(c->dpdk_eal_args)-1);
  else if (streq(key,"upf.dpdk.port_id")) c->dpdk_port_id = atoi(val);
  else if (streq(key,"upf.dpdk.rx_queues")) c->dpdk_rx_queues = atoi(val);
  else if (streq(key,"upf.dpdk.tx_queues")) c->dpdk_tx_queues = atoi(val);
  else if (streq(key,"upf.dpdk.mbuf_cache")) c->dpdk_mbuf_cache = atoi(val);

  else if (streq(key,"demo.enable")) c->demo_enable = (strcmp(val,"true")==0 || strcmp(val,"1")==0);
  else if (streq(key,"demo.ue_imsi")) strncpy(c->demo_imsi, val, sizeof(c->demo_imsi)-1);
  else if (streq(key,"demo.dnn")) strncpy(c->demo_dnn, val, sizeof(c->demo_dnn)-1);
  else if (streq(key,"demo.snssai.sst")) c->demo_sst = atoi(val);
  else if (streq(key,"demo.snssai.sd")) strncpy(c->demo_sd, val, sizeof(c->demo_sd)-1);
}

static void unquote(char* s){
  size_t n = strlen(s);
  if (n >= 2 && ((s[0]=='"' && s[n-1]=='"') || (s[0]=='\'' && s[n-1]=='\''))){
    memmove(s, s+1, n-2);
    s[n-2]='\0';
  }
}

bool config_load_yaml_subset(const char* path, app_config_t* out){
  if (!path || !out) return false;
  cfg_defaults(out);

  FILE* f = fopen(path, "r");
  if (!f){ LOGE("Cannot open config: %s", path); return false; }

  // Minimal YAML subset parser:
  // - supports nested keys via indentation of 2 spaces
  // - key: value lines (scalars only)
  // - ignores lists and complex YAML
  char line[512];
  char ctx1[64]={0}, ctx2[64]={0}, ctx3[64]={0};
  while(fgets(line, sizeof(line), f)){
    char* s = ltrim(line);
    rtrim(s);
    if (*s=='\0' || *s=='#') continue;

    int indent = (int)(s - line);
    // Determine context based on indentation (2 spaces per level)
    char* colon = strchr(s, ':');
    if (!colon) continue;

    *colon='\0';
    char key[128]; strncpy(key, s, sizeof(key)-1); key[sizeof(key)-1]='\0';
    char* val = ltrim(colon+1);
    if (*val=='#') *val='\0';
    rtrim(val);
    unquote(val);

    if (*val=='\0'){
      // This is a map key (context)
      if (indent < 2){ safe_strcpy(ctx1, sizeof(ctx1), key); ctx2[0]=ctx3[0]='\0'; }
      else if (indent < 4){ safe_strcpy(ctx2, sizeof(ctx2), key); ctx3[0]='\0'; }
      else if (indent < 6){ safe_strcpy(ctx3, sizeof(ctx3), key); }
      continue;
    }

    // Build dotted key
    char dotted[256]={0};
    if (indent < 2){
      snprintf(dotted, sizeof(dotted), "%s", key);
    } else if (indent < 4){
      snprintf(dotted, sizeof(dotted), "%s.%s", ctx1, key);
    } else if (indent < 6){
      snprintf(dotted, sizeof(dotted), "%s.%s.%s", ctx1, ctx2, key);
    } else {
      snprintf(dotted, sizeof(dotted), "%s.%s.%s.%s", ctx1, ctx2, ctx3, key);
    }
    set_kv(out, dotted, val);
  }

  fclose(f);
  return true;
}
