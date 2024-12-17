
#include "stm32f4xx_hal.h"

#include "logging.h"
#include <stdarg.h>

extern UART_HandleTypeDef huart2;

int _write(int file, char *ptr, int len)
{
  (void)file;
  HAL_UART_Transmit_IT(&huart2, (uint8_t*)ptr, len);
  return len;
}


SemaphoreHandle_t log_mutex = NULL;
StaticSemaphore_t xMutexBuffer;

void log_print(const char *level, const char *fmt, ...) {
    /* Initialize the mutex if not already created */
    if (log_mutex == NULL) {
        log_mutex = xSemaphoreCreateMutexStatic( &xMutexBuffer );
    }

    /* Take the mutex for thread safety */
    if (xSemaphoreTake(log_mutex, portMAX_DELAY) == pdTRUE) {
        va_list args;
        va_start(args, fmt);
        printf("[%s] ", level);  // Print log level
        vprintf(fmt, args);      // Print formatted message
        printf("\n");
        va_end(args);
        xSemaphoreGive(log_mutex);
    }
}
