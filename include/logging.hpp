#ifndef __MINI_LOG__
#define __MINI_LOG__

#include <stdarg.h>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <ctime>
#include "colors.h"

#define PRINT_VAR_ARGS(format, ...) \
  do {                              \
    va_list arg_list;               \
    va_start(arg_list, format);     \
    vprintf(format, arg_list);      \
    printf("\n");                   \
    va_end(arg_list);               \
  } while (0)

namespace Logging {
  constexpr uint8_t ERROR_LVL =    1 << 0;
  constexpr uint8_t SUCCESS_LVL =  1 << 1;
  constexpr uint8_t DEBUG_LVL =    1 << 2;
  constexpr uint8_t WARN_LVL =     1 << 3;
  constexpr uint8_t CRITICAL_LVL = 1 << 4;
  constexpr uint8_t INFO_LVL =     1 << 5;
  constexpr uint8_t LOG_WRITE =    1 << 6;

  extern uint8_t SAVED_LOG_LEVEL;
  extern uint8_t LOG_LEVEL;

  std::string getCurrentTime();

  void writeToFile(const char* prefix, const char* msg, va_list args);

  void disable_msg();

  void disable_file_logging();

  void enable_msg();

  void error(const char* msg, ...);

  void success(const char* msg, ...);

  void info(const char* msg, ...);

  void debug(const char* msg, ...);

  void log(const char* msg, ...);

  void warn(const char* msg, ...);

  void critical(const char* msg, ...);
}

#endif
