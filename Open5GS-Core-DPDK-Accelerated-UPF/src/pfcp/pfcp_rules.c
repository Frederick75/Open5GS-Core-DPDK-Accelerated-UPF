#include "pfcp/pfcp_rules.h"
#include <string.h>

void rules_init(pfcp_session_rules_t* r, uint64_t seid){
  memset(r, 0, sizeof(*r));
  r->seid = seid;
  r->qer.qfi = 9;
}
