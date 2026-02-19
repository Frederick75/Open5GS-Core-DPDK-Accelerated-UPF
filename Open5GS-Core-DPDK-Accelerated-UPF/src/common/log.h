#pragma once
#include <stdarg.h>
#include <stdbool.h>

typedef enum {
  LOG_ERROR = 0,
  LOG_WARN  = 1,
  LOG_INFO  = 2,
  LOG_DEBUG = 3,
} log_level_t;

void log_init(log_level_t lvl);
void log_set_level(log_level_t lvl);
log_level_t log_get_level(void);

void log_msg(log_level_t lvl, const char* fmt, ...) __attribute__((format(printf,2,3)));
void log_vmsg(log_level_t lvl, const char* fmt, va_list ap);

#define LOGE(...) log_msg(LOG_ERROR, __VA_ARGS__)
#define LOGW(...) log_msg(LOG_WARN,  __VA_ARGS__)
#define LOGI(...) log_msg(LOG_INFO,  __VA_ARGS__)
#define LOGD(...) log_msg(LOG_DEBUG, __VA_ARGS__)
