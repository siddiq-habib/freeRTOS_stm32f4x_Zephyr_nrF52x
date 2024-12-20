
#include "stm32f4xx_hal.h"

#include "log.h"
#include "TaskConfig.h"
#include <stdarg.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "assert.h"

extern UART_HandleTypeDef huart2;

/* Logging Queue Size */
#define LOG_QUEUE_SIZE 10
#define LOG_MESSAGE_MAX_LENGTH 128
#define UART_TX_TIMEOUT_MS (1000)


/* Logging message structure */
typedef struct {
    char message[LOG_MESSAGE_MAX_LENGTH];
} LogMessage_t;

const char* log_string[] = {
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG"    
};
extern const struct log_source_const_data __start_log_data[];
extern const struct log_source_const_data __stop_log_data[];


/* Queue to hold log messages */
static QueueHandle_t logQueue;
static StaticQueue_t logQueueBuffer;                  // Static queue control block
static uint8_t logQueueStorageArea[LOG_QUEUE_SIZE * sizeof(LogMessage_t)];  // Queue buffer


static StaticTask_t logTaskBuffer;                   // Static task control block
static StackType_t logTaskStack[LOG_TASK_STACK_SIZE];  // Static stack for the task



static SemaphoreHandle_t txCompleteSemaphore;
static StaticSemaphore_t txCompleteSemaphoreBuffer;

// Declare the mutex
static SemaphoreHandle_t xMutex;



// UART transmit complete callback
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        /* Signal the semaphore to unblock the waiting task */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(txCompleteSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        /* Signal the semaphore to unblock the waiting task */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(txCompleteSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    }
}


/* ---------------- Function Prototypes ---------------- */
static void logTask(void *pvParameters);

/* ---------------- Function Definitions ---------------- */

/* Initialize logging queue and task */
void logInit(void) {

    // Create a static binary semaphore for UART transmission complete
    txCompleteSemaphore = xSemaphoreCreateBinaryStatic(&txCompleteSemaphoreBuffer);
    configASSERT(txCompleteSemaphore != NULL);

    xMutex = xSemaphoreCreateRecursiveMutex();
    configASSERT(xMutex != NULL);

    TaskHandle_t logTaskHandle;
    /* Static initialization of the queue */
    logQueue = xQueueCreateStatic(
        LOG_QUEUE_SIZE,
        sizeof(LogMessage_t),
        logQueueStorageArea,
        &logQueueBuffer
    );
    configASSERT(logQueue != NULL);

    /* Static creation of the logging task */
    logTaskHandle = xTaskCreateStatic(
        logTask,                // Task function
        "LogTask",               // Task name
        LOG_TASK_STACK_SIZE,     // Stack size
        NULL,                    // Parameters
        LOG_TASK_PRIORITY,       // Task priority
        logTaskStack,            // Stack buffer
        &logTaskBuffer           // Task control block
    );
    configASSERT(logTaskHandle != NULL);
}

void logEnqueue( LogMessage_t *pLogMsg) {

    /* Determine context: ISR or task */
    if (xPortIsInsideInterrupt()) {
        /* ISR context */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (xQueueSendFromISR(logQueue, pLogMsg, &xHigherPriorityTaskWoken) != pdPASS) {
            /* Handle queue full in ISR (optional, e.g., increment a missed log counter) */
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        if (xSemaphoreTakeRecursive(xMutex, portMAX_DELAY) == pdTRUE)
        {
            /* Send to the queue (non-blocking in task context) */
            BaseType_t retLogQue;
            retLogQue = xQueueSend(logQueue, pLogMsg, 0);

            if (retLogQue != pdPASS) {
                /* Queue is full, remove the oldest message and add the new one */
                LogMessage_t tempMsg; // Temporary variable to discard the oldest message
                xQueueReceive(logQueue, &tempMsg, 0); // Remove the oldest message
                retLogQue = xQueueSend(logQueue, pLogMsg, 0); // Add the new message
                configASSERT(retLogQue == pdPASS); // Ensure the send succeeds
            }
            // Release the mutex
            xSemaphoreGiveRecursive(xMutex);
        }
    }
}

/* Add the proper Formatting and then Enqueue a log message */
void formatLogAndEnqueue(uint8_t level,  const struct log_source_const_data * plog_data,  const char *fmt, ...) {
    
    configASSERT((((uint8_t*)plog_data >= (uint8_t*)__start_log_data) && 
                    ((uint8_t*)plog_data < (uint8_t*)__stop_log_data)));

    if(level <= plog_data->level)
    {
        LogMessage_t logMsg;
        va_list args;

        /* Get the tick count and convert to milliseconds */
        TickType_t tickCount = xTaskGetTickCount();
        uint32_t totalMs = tickCount * portTICK_PERIOD_MS;

        /* Calculate hours, minutes, seconds, and milliseconds */
        uint32_t ms = totalMs % 1000;
        uint32_t totalSeconds = totalMs / 1000;
        uint32_t seconds = totalSeconds % 60;
        uint32_t totalMinutes = totalSeconds / 60;
        uint32_t minutes = totalMinutes % 60;
        uint32_t hours = totalMinutes / 60;

        /* Format the log message with timestamp and level */
        snprintf(logMsg.message, LOG_MESSAGE_MAX_LENGTH, "[%02u:%02u:%02u:%03u] :: ", 
                (unsigned int)hours, (unsigned int)minutes, (unsigned int)seconds, (unsigned int)ms);
        
        /* Format the log message */
        uint8_t usedLen = strlen(logMsg.message);
        snprintf(logMsg.message + usedLen, LOG_MESSAGE_MAX_LENGTH - usedLen,
                                 "[%s] :: [%s] ",plog_data->name, log_string[level - 1]);

        va_start(args, fmt);
        usedLen = strlen(logMsg.message); // Include brackets and space
        vsnprintf(logMsg.message + usedLen, LOG_MESSAGE_MAX_LENGTH - usedLen, fmt, args);
        va_end(args);

        logEnqueue(&logMsg);

      
    }
}


void logHexDump(uint8_t level, const char * str, const struct log_source_const_data * plog_data ,
			const uint8_t*  data, size_t length)
{
    configASSERT((((uint8_t*)plog_data >= (uint8_t*)__start_log_data) && 
                ((uint8_t*)plog_data < (uint8_t*)__stop_log_data)));

    if(level <= plog_data->level)
    {
        if (xSemaphoreTakeRecursive(xMutex, portMAX_DELAY) == pdTRUE)
        {
            char hexBuffer[49];  // 16 bytes * 3 chars per byte + 1 (null terminator)
            char asciiBuffer[17]; // 16 bytes + 1 (null terminator)

            formatLogAndEnqueue(level, plog_data, "\nHex Dump (%s : Length(%d)):\n",str, length);
            for (size_t i = 0; i < length; i++)
            {
                // Add hex representation to hexBuffer
                snprintf(&hexBuffer[(i % 16) * 3], 4, "%02X ", data[i]);

                // Add printable ASCII character or '.' to asciiBuffer
                asciiBuffer[i % 16] = (data[i] >= 32 && data[i] <= 126) ? data[i] : '.';

                // If line is full or this is the last byte, log the line
                if ((i % 16 == 15) || (i == length - 1)) {
                    LogMessage_t logMsg;
                    asciiBuffer[(i % 16) + 1] = '\0'; // Null-terminate ASCII buffer

                    // Log the hexBuffer and asciiBuffer
                    
                    snprintf(logMsg.message,LOG_MESSAGE_MAX_LENGTH , "\t\t\t  %04dX  |%-48s|  %s\n",i - (i % 16),  hexBuffer, asciiBuffer);
                    logEnqueue(&logMsg);


                    // Reset buffers for the next line
                    memset(hexBuffer, ' ', sizeof(hexBuffer));
                    hexBuffer[48] = '\0'; // Null-terminate hexBuffer
                }
            }
            //Release the mutex
            xSemaphoreGiveRecursive(xMutex);
        }
    }   
}


/* Logging task: Dequeues and transmit log messages */

static void logTask(void *pvParameters) {
    LogMessage_t logMsg;
    
    UNUSED(pvParameters);
    while (1) {
        /* Wait for a message in the queue */
        if (xQueueReceive(logQueue, &logMsg, portMAX_DELAY) == pdTRUE) {
            // Send the message over UART
            HAL_UART_Transmit_IT(&huart2, (uint8_t*)logMsg.message, strlen(logMsg.message));

            // Wait for UART transmission to complete
            if (xSemaphoreTake(txCompleteSemaphore, pdMS_TO_TICKS(UART_TX_TIMEOUT_MS)) == pdTRUE) {
                // Transmission completed, proceed to next message
            } else {
                // Handle error if semaphore is not taken (timeout or failure)
            }
        }
    }
}
