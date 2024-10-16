#ifndef __MINI_LOG__
#define __MINI_LOG__

#include <stdarg.h>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <ctime> // Para obtener la hora actual
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

// Función para obtener el timestamp actual
static std::string getCurrentTime() {
  std::time_t now = std::time(nullptr);
  std::tm* local_time = std::localtime(&now);
  char buffer[80];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
  return std::string(buffer);
}

// Función para escribir los mensajes en el archivo log
static void writeToFile(const char* prefix, const char* msg, va_list args) {
  std::ofstream log_file("log.txt", std::ios_base::app); // Abre el archivo en modo append
  if (log_file.is_open()) {
    log_file << getCurrentTime() << " " << prefix << " "; // Agrega el timestamp
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), msg, args); // Formatea el mensaje
    log_file << buffer << std::endl;
    log_file.close(); // Cierra el archivo
  }
}

static void disable_msg() {
  SAVED_LOG_LEVEL = LOG_LEVEL;
  LOG_LEVEL = 0;
}

static void enable_msg() { LOG_LEVEL = SAVED_LOG_LEVEL; }

static void error(const char* msg, ...) {
  if (LOG_LEVEL & ERROR_LVL) {
    fprintf(stderr, "%s ", RED "[ERR] " ANSI_COLOR_RESET);
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\n");
    writeToFile("[ERR]", msg, args);
    va_end(args);
  }
}

static void success(const char* msg, ...) {
  if (LOG_LEVEL & SUCCESS_LVL) {
    printf("%s", GRN "[SUCCESS] " ANSI_COLOR_RESET);
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\n");
    writeToFile("[SUCCESS]", msg, args);
    va_end(args);
  }
}

static void info(const char* msg, ...) {
  if (LOG_LEVEL & INFO_LVL) {
    printf("%s", CYN "[INFO] " ANSI_COLOR_RESET);
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\n");
    writeToFile("[INFO]", msg, args);
    va_end(args);
  }
}

static void debug(const char* msg, ...) {
  if (LOG_LEVEL & DEBUG_LVL) {
    printf("%s", MAG "[DEBUG] " ANSI_COLOR_RESET);
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\n");
    writeToFile("[DEBUG]", msg, args);
    va_end(args);
  }
}

static void warn(const char* msg, ...) {
  if (LOG_LEVEL & WARN_LVL) {
    printf("%s", YEL "[WARN] " ANSI_COLOR_RESET);
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\n");
    writeToFile("[WARN]", msg, args);
    va_end(args);
  }
}

static void critical(const char* msg, ...) {
  if (LOG_LEVEL & CRITICAL_LVL) {
    printf("%s", RED "[CRITICAL] " ANSI_COLOR_RESET);
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\n");
    writeToFile("[CRITICAL]", msg, args);
    va_end(args);
  }
}
}  // namespace logging

#endif
