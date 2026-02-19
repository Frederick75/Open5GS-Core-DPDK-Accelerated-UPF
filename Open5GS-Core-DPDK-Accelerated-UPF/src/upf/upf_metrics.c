#include "upf/upf.h"
#include "upf/session_table.h"
#include "common/log.h"
#include <inttypes.h>

void upf_metrics_dump(const upf_ctx_t* ctx){
  if (!ctx || !ctx->sess_table) return;
  uint64_t ul=0, dl=0;
  upf_sess_table_agg_urr(ctx->sess_table, &ul, &dl);
  LOGI("URR aggregate: UL=%" PRIu64 " bytes DL=%" PRIu64 " bytes", ul, dl);
}
