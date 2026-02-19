#include "upf/session_table.h"
#include "common/log.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct {
  uint64_t key;  // SEID
  pfcp_session_rules_t val;
  uint8_t state; // 0=empty, 1=full, 2=tombstone
} bucket_t;

struct upf_sess_table {
  bucket_t* b;
  size_t cap;        // power of 2
  size_t size;
  pthread_rwlock_t rw;
};

static uint64_t mix64(uint64_t x){
  // splitmix64
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

static size_t idx_of(size_t cap, uint64_t key){
  return (size_t)(mix64(key) & (cap - 1));
}

upf_sess_table_t* upf_sess_table_create(size_t capacity_pow2){
  if (capacity_pow2 < 1024) capacity_pow2 = 1024;
  // ensure power of 2
  size_t cap = 1;
  while(cap < capacity_pow2) cap <<= 1;

  upf_sess_table_t* t = calloc(1, sizeof(*t));
  if (!t) return NULL;
  t->b = calloc(cap, sizeof(bucket_t));
  if (!t->b){ free(t); return NULL; }
  t->cap = cap;
  pthread_rwlock_init(&t->rw, NULL);
  return t;
}

void upf_sess_table_destroy(upf_sess_table_t* t){
  if (!t) return;
  pthread_rwlock_destroy(&t->rw);
  free(t->b);
  free(t);
}

static bool put_nolock(upf_sess_table_t* t, const pfcp_session_rules_t* rules){
  if (!t || !rules) return false;
  uint64_t key = rules->seid;
  size_t cap = t->cap;
  size_t i = idx_of(cap, key);
  size_t first_tomb = (size_t)-1;

  for (size_t probe=0; probe<cap; probe++){
    bucket_t* bk = &t->b[(i + probe) & (cap - 1)];
    if (bk->state == 0){
      if (first_tomb != (size_t)-1) bk = &t->b[first_tomb];
      bk->key = key;
      bk->val = *rules;
      bk->state = 1;
      t->size++;
      return true;
    }
    if (bk->state == 2 && first_tomb == (size_t)-1){
      first_tomb = (i + probe) & (cap - 1);
    }
    if (bk->state == 1 && bk->key == key){
      bk->val = *rules;
      return true;
    }
  }
  return false;
}

bool upf_sess_table_put(upf_sess_table_t* t, const pfcp_session_rules_t* rules){
  if (!t || !rules) return false;
  pthread_rwlock_wrlock(&t->rw);
  bool ok = put_nolock(t, rules);
  pthread_rwlock_unlock(&t->rw);
  return ok;
}

bool upf_sess_table_get(upf_sess_table_t* t, uint64_t seid, pfcp_session_rules_t* out){
  if (!t || !out) return false;
  pthread_rwlock_rdlock(&t->rw);
  size_t cap = t->cap;
  size_t i = idx_of(cap, seid);

  for (size_t probe=0; probe<cap; probe++){
    bucket_t* bk = &t->b[(i + probe) & (cap - 1)];
    if (bk->state == 0) break;
    if (bk->state == 1 && bk->key == seid){
      *out = bk->val;
      pthread_rwlock_unlock(&t->rw);
      return true;
    }
  }
  pthread_rwlock_unlock(&t->rw);
  return false;
}

bool upf_sess_table_update(upf_sess_table_t* t, uint64_t seid, upf_sess_update_fn fn, void* user){
  if (!t || !fn) return false;
  pthread_rwlock_wrlock(&t->rw);
  size_t cap = t->cap;
  size_t i = idx_of(cap, seid);

  for (size_t probe=0; probe<cap; probe++){
    bucket_t* bk = &t->b[(i + probe) & (cap - 1)];
    if (bk->state == 0) break;
    if (bk->state == 1 && bk->key == seid){
      fn(&bk->val, user);
      pthread_rwlock_unlock(&t->rw);
      return true;
    }
  }
  pthread_rwlock_unlock(&t->rw);
  return false;
}

bool upf_sess_table_del(upf_sess_table_t* t, uint64_t seid){
  if (!t) return false;
  pthread_rwlock_wrlock(&t->rw);
  size_t cap = t->cap;
  size_t i = idx_of(cap, seid);

  for (size_t probe=0; probe<cap; probe++){
    bucket_t* bk = &t->b[(i + probe) & (cap - 1)];
    if (bk->state == 0) break;
    if (bk->state == 1 && bk->key == seid){
      bk->state = 2; // tombstone
      memset(&bk->val, 0, sizeof(bk->val));
      t->size--;
      pthread_rwlock_unlock(&t->rw);
      return true;
    }
  }
  pthread_rwlock_unlock(&t->rw);
  return false;
}

void upf_sess_table_agg_urr(upf_sess_table_t* t, uint64_t* ul_bytes, uint64_t* dl_bytes){
  if (!t) return;
  uint64_t ul=0, dl=0;
  pthread_rwlock_rdlock(&t->rw);
  for (size_t i=0;i<t->cap;i++){
    if (t->b[i].state == 1){
      ul += t->b[i].val.urr.uplink_bytes;
      dl += t->b[i].val.urr.downlink_bytes;
    }
  }
  pthread_rwlock_unlock(&t->rw);
  if (ul_bytes) *ul_bytes = ul;
  if (dl_bytes) *dl_bytes = dl;
}

bool upf_sess_table_find_by_teid_ul(upf_sess_table_t* t, uint32_t teid_ul, pfcp_session_rules_t* out){
  if (!t || !out) return false;
  pthread_rwlock_rdlock(&t->rw);
  for (size_t i=0;i<t->cap;i++){
    if (t->b[i].state == 1 && t->b[i].val.pdr.teid_ul == teid_ul){
      *out = t->b[i].val;
      pthread_rwlock_unlock(&t->rw);
      return true;
    }
  }
  pthread_rwlock_unlock(&t->rw);
  return false;
}
