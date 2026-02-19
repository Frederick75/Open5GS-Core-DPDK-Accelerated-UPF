#pragma once
#include "upf/upf.h"
#include <stdbool.h>

bool upf_dataplane_init(upf_ctx_t* ctx);
void upf_dataplane_poll(upf_ctx_t* ctx);
