#pragma once

#include "printf.h"
#include "entry.h"
#include "sched.h"

static void print_status(const char *level) {
  if (current)
    printf("%s[%d]: ", level, current->pid);
  else
    printf("%s[?]: ", level);
}

#define _LOG_COMMON(level, fmt, ...) do { \
  print_status(level); \
  printf(fmt "\r\n", ##__VA_ARGS__); \
} while(0)


#define INFO(fmt, ...) _LOG_COMMON("INFO", fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) _LOG_COMMON("WARN", fmt, ##__VA_ARGS__)

#define PANIC(fmt, ...) do { \
  _LOG_COMMON("!!! PANIC", fmt, ## __VA_ARGS__); \
  if (current) \
    exit_task(); \
  else \
    err_hang(); \
} while(0)
