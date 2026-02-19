#include "common/log.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>

static log_level_t g_lvl = LOG_INFO;
static pthread_mutex_t g_mu = PTHREAD_MUTEX_INITIALIZER;

static const char* lvl_s(log_level_t l){
  switch(l){
    case LOG_ERROR: return "ERROR";
    case LOG_WARN:  return "WARN";
    case LOG_INFO:  return "INFO";
    case LOG_DEBUG: return "DEBUG";
    default:        return "UNK";
  }
}

void log_init(log_level_t lvl){ log_set_level(lvl); }
void log_set_level(log_level_t lvl){ g_lvl = lvl; }
log_level_t log_get_level(void){ return g_lvl; }

void log_vmsg(log_level_t lvl, const char* fmt, va_list ap){
  if (lvl > g_lvl) return;
  struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
  struct tm tm; localtime_r(&ts.tv_sec, &tm);

  pthread_mutex_lock(&g_mu);
  fprintf(stderr, "%04d-%02d-%02d %02d:%02d:%02d.%03ld [%s] ",
          tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
          tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec/1000000,
          lvl_s(lvl));
  vfprintf(stderr, fmt, ap);
  fputc('\n', stderr);
  pthread_mutex_unlock(&g_mu);
}

void log_msg(log_level_t lvl, const char* fmt, ...){
  va_list ap; va_start(ap, fmt);
  log_vmsg(lvl, fmt, ap);
  va_end(ap);
}
