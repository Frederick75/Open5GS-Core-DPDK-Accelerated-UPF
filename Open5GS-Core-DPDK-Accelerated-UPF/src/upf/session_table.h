#pragma once
#include "pfcp/pfcp_rules.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct upf_sess_table upf_sess_table_t;

upf_sess_table_t* upf_sess_table_create(size_t capacity_pow2);
void upf_sess_table_destroy(upf_sess_table_t* t);

// Create or replace session rules for SEID.
bool upf_sess_table_put(upf_sess_table_t* t, const pfcp_session_rules_t* rules);

// Lookup session rules by SEID. Returns a copy into *out.
bool upf_sess_table_get(upf_sess_table_t* t, uint64_t seid, pfcp_session_rules_t* out);

// Update session rules in-place using a callback; returns false if not found.
typedef void (*upf_sess_update_fn)(pfcp_session_rules_t* rules, void* user);
bool upf_sess_table_update(upf_sess_table_t* t, uint64_t seid, upf_sess_update_fn fn, void* user);

// Delete session.
bool upf_sess_table_del(upf_sess_table_t* t, uint64_t seid);

// Aggregate counters for quick metrics.
void upf_sess_table_agg_urr(upf_sess_table_t* t, uint64_t* ul_bytes, uint64_t* dl_bytes);

// Lookup by UL TEID (used for N3 uplink classification). O(cap) scan in this reference.
bool upf_sess_table_find_by_teid_ul(upf_sess_table_t* t, uint32_t teid_ul, pfcp_session_rules_t* out);
