#ifndef LOGCORE_H_
#define LOGCORE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdint.h>


#define LOG_DATA_SECTION __attribute__((section(".log_data"), used))


#define Z_STRINGIFY(x) #x
#define STRINGIFY(s) Z_STRINGIFY(s)

/* concatenate the values of the arguments into one */
#define _DO_CONCAT(x, y) x ## y
#define _CONCAT(x, y) _DO_CONCAT(x, y)

struct log_source_const_data {
	const char *name;
	uint8_t level;
};

void logEnqueue(uint8_t level, const struct log_source_const_data * pcurrent_log_data ,  const char *fmt, ...);


#define Z_LOG(_level, ...) \
	Z_LOG2(_level, pcurrent_log_data, __VA_ARGS__)

#define Z_LOG2(_level, _source, ...) logEnqueue(_level,_source, __VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif /* LOGCORE_H_ */
