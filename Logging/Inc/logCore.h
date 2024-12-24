/******************************************************************************
 *  COPYRIGHT (C) 2023 Cresnt Ltd
 *  All rights reserved.
 *
 *  This software is the confidential and proprietary information of
 *  Cresnt Ltd ("Confidential Information"). You shall not
 *  disclose such Confidential Information and shall use it only in
 *  accordance with the terms of the license agreement you entered into
 *  with Cresnt Ltd.
 *
 *  Cresnt Ltd makes no representations or warranties about the
 *  suitability of the software, either express or implied, including but
 *  not limited to the implied warranties of merchantability, fitness for
 *  a particular purpose, or non-infringement. Cresnt Ltd shall not
 *  be liable for any damages suffered by licensee as a result of using,
 *  modifying, or distributing this software or its derivatives.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited.
 *
 *****************************************************************************/
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

void logHexDump(uint8_t level, const char* str, const struct log_source_const_data * pcurrent_log_data ,
			const uint8_t*  data, size_t length);
void formatLogAndEnqueue(uint8_t level, const struct log_source_const_data * pcurrent_log_data ,  const char *fmt, ...);


#define Z_LOG_HEXDUMP(_level, _str, _data, _length) \
	Z_LOG_HEXDUMP2(_level, _str, pcurrent_log_data,   _data, _length)

#define Z_LOG_HEXDUMP2(_level, _str, _source, _data, _len) \
	logHexDump(_level, _str,  _source, _data, _len)


#define Z_LOG(_level, ...) \
	Z_LOG2(_level, pcurrent_log_data, __VA_ARGS__)

#define Z_LOG2(_level, _source, ...) formatLogAndEnqueue(_level,_source, __VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif /* LOGCORE_H_ */
