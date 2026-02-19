#pragma once
#include "common/config.h"
#include "smf/smf_sessions.h"
#include <stdbool.h>

bool smf_pfcp_heartbeat(const app_config_t* cfg);
bool smf_pfcp_session_establish(const app_config_t* cfg, smf_session_t* sess);
bool smf_pfcp_session_modify(const app_config_t* cfg, smf_session_t* sess);
bool smf_pfcp_session_delete(const app_config_t* cfg, smf_session_t* sess);
