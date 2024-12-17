#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

/* Log Levels */
#define LOG_LEVEL_NONE  0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_DEBUG 4

/* Current log level */
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

/* Thread-safety: Logging semaphore */
extern SemaphoreHandle_t log_mutex;

/* Logging Macros */
#define LOG_ERROR(fmt, ...) \
    do { if (LOG_LEVEL >= LOG_LEVEL_ERROR) log_print("ERROR", fmt, ##__VA_ARGS__); } while (0)

#define LOG_WARN(fmt, ...) \
    do { if (LOG_LEVEL >= LOG_LEVEL_WARN) log_print("WARN", fmt, ##__VA_ARGS__); } while (0)

#define LOG_INFO(fmt, ...) \
    do { if (LOG_LEVEL >= LOG_LEVEL_INFO) log_print("INFO", fmt, ##__VA_ARGS__); } while (0)

#define LOG_DEBUG(fmt, ...) \
    do { if (LOG_LEVEL >= LOG_LEVEL_DEBUG) log_print("DEBUG", fmt, ##__VA_ARGS__); } while (0)

/* Logging Function */
void log_print(const char *level, const char *fmt, ...);

#endif /* LOG_H */
