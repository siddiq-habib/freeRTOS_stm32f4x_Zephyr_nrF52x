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
