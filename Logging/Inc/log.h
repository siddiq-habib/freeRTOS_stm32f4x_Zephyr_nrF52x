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
#ifndef LOG_H_
#define LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include"logCore.h"

/* Log Levels */
#define LOG_LEVEL_NONE  0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_DEBUG 4

/* Logging Macros */
#define LOG_ERROR(...)  Z_LOG(LOG_LEVEL_ERROR, __VA_ARGS__)

#define LOG_WARN(...)  Z_LOG(LOG_LEVEL_WARN, __VA_ARGS__)

#define LOG_INFO(...)  Z_LOG(LOG_LEVEL_INFO, __VA_ARGS__)
   
#define LOG_DEBUG(...) Z_LOG(LOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_HEXDUMP_ERROR(_data, _length) \
    Z_LOG_HEXDUMP(LOG_LEVEL_ERROR, _data, _length)

#define LOG_HEXDUMP_WARN(_data, _length) \
    Z_LOG_HEXDUMP(LOG_LEVEL_WARN, _data, _length)

#define LOG_HEXDUMP_INFO(_str, _data, _length) \
    Z_LOG_HEXDUMP(LOG_LEVEL_INFO, _str, _data, _length)

#define LOG_HEXDUMP_DEBUG(_data, _length) \
    Z_LOG_HEXDUMP(LOG_LEVEL_DEBUG, _data, _length)


/* Dynamic module-specific declarations */
#define LOG_MODULE_DECLARE(_name) \
extern const struct log_source_const_data _CONCAT(__log_current_const_data, _name); \
static  const struct log_source_const_data * pcurrent_log_data = &_CONCAT(__log_current_const_data, _name)

#define LOG_MODULE_DEFINE(_name, _level) \
LOG_DATA_SECTION const struct log_source_const_data _CONCAT(__log_current_const_data, _name)=  \
{   .name =  STRINGIFY(_name), \
    .level = _level }; \
LOG_MODULE_DECLARE(_name)


/* Function Declarations */
void logInit(void);


#ifdef __cplusplus
}
#endif

#endif /* LOG_H_ */
