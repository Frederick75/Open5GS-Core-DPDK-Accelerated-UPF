#include "smf/smf_sessions.h"
#include <string.h>

void smf_sess_init(smf_session_t* s, const char* imsi, const char* dnn, uint64_t seid){
  memset(s, 0, sizeof(*s));
  strncpy(s->imsi, imsi, sizeof(s->imsi)-1);
  strncpy(s->dnn, dnn, sizeof(s->dnn)-1);
  s->seid = seid;
  s->state = SMF_SESS_IDLE;
}
