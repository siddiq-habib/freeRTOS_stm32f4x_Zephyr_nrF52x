#include "TaskConfig.h"

#include "main.h"
#include "log.h"


LOG_MODULE_DEFINE(AppTask, LOG_LEVEL_DEBUG);

extern void Log_ResetReason();

static StaticTask_t xTaskTest1TCB;
static StackType_t uxTaskTest1Stack[ MAINTASK_STACK_SIZE ];
static StaticTask_t xTaskTest2TCB;
static StackType_t uxTaskTest2Stack[ TESTTASK_STACK_SIZE ];


extern void Test2Task_Module(void );
static void MainTask( void *pvParameters );
static void TestTask( void *pvParameters );


void createApplicationTasks(void)
{
    TaskHandle_t taskHandle = NULL;
    taskHandle =  xTaskCreateStatic(MainTask, "TaskMain", MAINTASK_STACK_SIZE, (void*) NULL,
                                        MAINTASK_TASK_PRIORITY, uxTaskTest1Stack, &xTaskTest1TCB);
    configASSERT(taskHandle != NULL);

    taskHandle =  xTaskCreateStatic(TestTask, "TaskTest", TESTTASK_STACK_SIZE, (void*) NULL,
                                        TESTTASK_TASK_PRIORITY, uxTaskTest2Stack, &xTaskTest2TCB);
    configASSERT(taskHandle != NULL);
}


/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
   implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
   used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    /* If the buffers to be provided to the Idle task are declared inside this
       function then they must be declared static - otherwise they will be allocated on
       the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
       state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
       Note that, as the array is necessarily of type StackType_t,
       configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
   application must provide an implementation of vApplicationGetTimerTaskMemory()
   to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    /* If the buffers to be provided to the Timer task are declared inside this
       function then they must be declared static - otherwise they will be allocated on
       the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
       task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
       Note that, as the array is necessarily of type StackType_t,
      configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}


static void MainTask(void *pvParameters)
{
   UNUSED(pvParameters);
   LOG_INFO("Main Task\r\n");

   Log_ResetReason();
   while(1)
   {
         vTaskDelay(pdMS_TO_TICKS(100));
         HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
         LOG_INFO("After LED Toggle\r\n"); 
   }
  vTaskDelete(NULL);
}

static void TestTask(void *pvParameters)
{
  UNUSED(pvParameters);
  while(1)
  {
   static int counter = 0;
   counter++;

   if(counter > 0xFFFF)
      counter = 0;

   //Test2Task_Module();
   LOG_INFO("Test2Task  %d\r\n", counter);
   vTaskDelay(pdMS_TO_TICKS(10));
   }
  vTaskDelete(NULL);
}

