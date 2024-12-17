#ifndef _TASKCONFIG_H_
#define _TASKCONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"

#define TEST1TASK_STACK_SIZE (configMINIMAL_STACK_SIZE*5)
#define TEST1TASK_TASK_PRIORITY (configMAX_PRIORITIES - 1)

#define TEST2TASK_STACK_SIZE (configMINIMAL_STACK_SIZE*5)
#define TEST2TASK_TASK_PRIORITY (configMAX_PRIORITIES - 1)

/* Logging Task Storage */
#define LOG_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 5)
#define LOG_TASK_PRIORITY (tskIDLE_PRIORITY + 1)



void createApplicationTasks(void);




#ifdef __cplusplus
}
#endif
#endif /* _TASKCONFIG_H_ */