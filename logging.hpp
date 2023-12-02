#ifndef __MINI_LOG__
#define __MINI_LOG__

#include <stdarg.h>
#include <cstdint>

#include "colors.h"

#define PRINT_VAR_ARGS(format, ...) \
  do {                              \
    va_list arg_list;               \
    va_start(arg_list, format);     \
    vprintf(format, arg_list);      \
    printf("\n");                   \
    va_end(arg_list);               \
  } while (0)

namespace logging {

static uint8_t ERROR_LVL =    1 << 0;
static uint8_t SUCCESS_LVL =  1 << 1;
static uint8_t DEBUG_LVL =    1 << 2;
static uint8_t WARN_LVL =     1 << 3;
static uint8_t CRITICAL_LVL = 1 << 4;
static uint8_t INFO_LVL =     1 << 5;

static short unsigned int SAVED_LOG_LEVEL = 0;
static short unsigned int LOG_LEVEL =
    ERROR_LVL | SUCCESS_LVL | DEBUG_LVL | WARN_LVL | INFO_LVL;

static void disable_msg() {
  SAVED_LOG_LEVEL = LOG_LEVEL;
  LOG_LEVEL = 0;
}

static void enable_msg() { LOG_LEVEL = SAVED_LOG_LEVEL; }

static void error(const char* msg, ...) {
  if (LOG_LEVEL & ERROR_LVL) {
    fprintf(stderr, "%s ", RED "[ERR] " ANSI_COLOR_RESET);
    PRINT_VAR_ARGS(msg, msg);
  }
}

static void success(const char* msg, ...) {
  if (LOG_LEVEL & SUCCESS_LVL) {
    printf("%s", GRN "[SUCESS] " ANSI_COLOR_RESET);
    PRINT_VAR_ARGS(msg, msg);
  }
}

static void info(const char* msg, ...) {
  if (LOG_LEVEL & SUCCESS_LVL) {
    printf("%s", CYN "[INFO] " ANSI_COLOR_RESET);
    PRINT_VAR_ARGS(msg, msg);
  }
}

static void debug(const char* msg, ...) {
  if (LOG_LEVEL & DEBUG_LVL) {
    printf("%s", MAG "[DEBUG] " ANSI_COLOR_RESET);
    PRINT_VAR_ARGS(msg, msg);
  }
}
static void warn(const char* msg, ...) {
  if (LOG_LEVEL & WARN_LVL) {
    printf("%s", YEL "[WARN] " ANSI_COLOR_RESET);
    PRINT_VAR_ARGS(msg, msg);
  }
}
static void critical(const char* msg, ...) {
  if (LOG_LEVEL & CRITICAL_LVL) {
    printf("%s", RED "[CRITICAL] " ANSI_COLOR_RESET);
    PRINT_VAR_ARGS(msg, msg);
  }
}
}  // namespace logging

#endif