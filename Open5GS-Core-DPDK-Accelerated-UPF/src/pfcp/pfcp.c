#include "pfcp/pfcp.h"
#include <stdatomic.h>

static atomic_uint g_seq = 1;

uint32_t pfcp_next_seq(void){
  return atomic_fetch_add(&g_seq, 1);
}
