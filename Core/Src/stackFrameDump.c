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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h" // Include the header file for your specific STM32 device
#include "log.h"

#define FAULT_FLAG 0xDEAD

LOG_MODULE_DEFINE(StackFrameDump, LOG_LEVEL_DEBUG);

// Create a buffer to hold the log information
char logBuffer[512];
extern WWDG_HandleTypeDef hwwdg;
extern RTC_HandleTypeDef hrtc;
/*  Stack Frame Structure
+-----------------+
| R0              | <- Stack Pointer (SP)
+-----------------+
| R1              |
+-----------------+
| R2              |
+-----------------+
| R3              |
+-----------------+
| R12             |
+-----------------+
| LR (Link Register) |
+-----------------+
| PC (Program Counter) |
+-----------------+
| xPSR            |
+-----------------+
*/

// Define the structure of the stack frame
typedef struct
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
} StackFrame;

// Define a structure to hold the fault status registers
typedef struct
{
    uint32_t hfsr;
    uint32_t cfsr;
    uint32_t mmfar;
    uint32_t bfar;
    uint32_t afsr;
} FaultStatusRegisters;
// Define a structure to hold the complete core dump information
typedef struct
{
    StackFrame stackFrame;
    FaultStatusRegisters faultStatus;
} CoreDump;


static StackFrame *stackFramePtr;
void Fault_HandlerC(StackFrame *stackFrame)
{
   // Check if the fault information has already been stored
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != FAULT_FLAG)
    {
        // Create a CoreDump structure to hold the stack frame and fault status registers
        CoreDump coreDump;

        // Access the stack frame and retrieve information
        coreDump.stackFrame.r0 = stackFrame->r0;
        coreDump.stackFrame.r1 = stackFrame->r1;
        coreDump.stackFrame.r2 = stackFrame->r2;
        coreDump.stackFrame.r3 = stackFrame->r3;
        coreDump.stackFrame.r12 = stackFrame->r12;
        coreDump.stackFrame.lr = stackFrame->lr;
        coreDump.stackFrame.pc = stackFrame->pc;
        coreDump.stackFrame.psr = stackFrame->psr;

        // Read fault status registers
        coreDump.faultStatus.hfsr = SCB->HFSR;
        coreDump.faultStatus.cfsr = SCB->CFSR;
        coreDump.faultStatus.mmfar = SCB->MMFAR;
        coreDump.faultStatus.bfar = SCB->BFAR;
        coreDump.faultStatus.afsr = SCB->AFSR;

        // Store the retrieved information in backup registers
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, coreDump.stackFrame.r0);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, coreDump.stackFrame.r1);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, coreDump.stackFrame.r2);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, coreDump.stackFrame.r3);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, coreDump.stackFrame.r12);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6, coreDump.stackFrame.lr);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR7, coreDump.stackFrame.pc);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR8, coreDump.stackFrame.psr);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR9, coreDump.faultStatus.hfsr);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR10, coreDump.faultStatus.cfsr);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR11, coreDump.faultStatus.mmfar);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR12, coreDump.faultStatus.bfar);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR13, coreDump.faultStatus.afsr);

        // Set the fault flag to indicate that the fault information has been stored
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, FAULT_FLAG);
    }

#if defined(DEBUG)
    // Disable all breakpoints and halt the system
    //__asm volatile ("bkpt #0"); // Trigger a breakpoint to halt the system
#endif
}



void Log_StackCoreDump(void)
{
    // Check if the fault information has been stored
    uint32_t faultFlag = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);

    if (faultFlag == FAULT_FLAG)
    {
        // Create a CoreDump structure to hold the retrieved information
        CoreDump coreDump;

        // Read the stored information from backup registers
        coreDump.stackFrame.r0 = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
        coreDump.stackFrame.r1 = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
        coreDump.stackFrame.r2 = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
        coreDump.stackFrame.r3 = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
        coreDump.stackFrame.r12 = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);
        coreDump.stackFrame.lr = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR6);
        coreDump.stackFrame.pc = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR7);
        coreDump.stackFrame.psr = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR8);
        coreDump.faultStatus.hfsr = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR9);
        coreDump.faultStatus.cfsr = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR10);
        coreDump.faultStatus.mmfar = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR11);
        coreDump.faultStatus.bfar = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR12);
        coreDump.faultStatus.afsr = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR13);

        sniprintf(logBuffer, sizeof(logBuffer), "\nRegister Core Dump\n");

        // Print the retrieved information into the buffer
        snprintf(logBuffer + strlen(logBuffer), sizeof(logBuffer)- + strlen(logBuffer),
            "faultFlag: 0x%08lX\n"
             "r0: 0x%08lX\n"
             "r1: 0x%08lX\n"
             "r2: 0x%08lX\n"
             "r3: 0x%08lX\n"
             "r12: 0x%08lX\n"
             "lr: 0x%08lX\n"
             "pc: 0x%08lX\n"
             "psr: 0x%08lX\n"
             "hfsr: 0x%08lX\n"
             "cfsr: 0x%08lX\n"
             "mmfar: 0x%08lX\n"
             "bfar: 0x%08lX\n"
             "afsr: 0x%08lX\n",
             faultFlag,
             coreDump.stackFrame.r0,
             coreDump.stackFrame.r1,
             coreDump.stackFrame.r2,
             coreDump.stackFrame.r3,
             coreDump.stackFrame.r12,
             coreDump.stackFrame.lr,
             coreDump.stackFrame.pc,
             coreDump.stackFrame.psr,
             coreDump.faultStatus.hfsr,
             coreDump.faultStatus.cfsr,
             coreDump.faultStatus.mmfar,
             coreDump.faultStatus.bfar,
             coreDump.faultStatus.afsr);

        // Print the buffer content (for example, to a console or log file)
        LOG_ERROR("%s", logBuffer);
    }

        // Clear the fault flag
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0);
    }

/**
  * @brief This function handles Window watchdog interrupt.
  */
void WWDG_IRQHandler_C(StackFrame *stackFrame)
{
  /* USER CODE BEGIN WWDG_IRQn 0 */
    stackFramePtr = stackFrame;
  /* USER CODE END WWDG_IRQn 0 */
  HAL_WWDG_IRQHandler(&hwwdg);
  /* USER CODE BEGIN WWDG_IRQn 1 */

  /* USER CODE END WWDG_IRQn 1 */
}


__weak void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hwwdg);
    Fault_HandlerC(stackFramePtr);
}

__attribute__ ((naked)) void StackFrameDump(void)
{
    __asm volatile
    (
        // Check EXC_RETURN bit 2 to determine if the exception frame is using PSP or MSP
        "  movs r0, #4\n"          // Load 4 into r0
        "  movs r1, lr\n"          // Load the link register into r1
        "  tst r0, r1\n"           // Test bit 2 of the EXC_RETURN value
        "  beq _MSP\n"             // If bit 2 is 0, use MSP, otherwise use PSP
        "  mrs r0, psp\n"          // Move the current value of PSP into r0
        "  b _HALT\n"              // Branch to _HALT
        "_MSP:\n"
        "  mrs r0, msp\n"          // Move the current value of MSP into r0
        "_HALT:\n"
        "  ldr r2, handler2_address_const\n"  // Load the address of Fault_HandlerC into r2
        "  bx r2\n"                // Branch to the address in r2
        "handler2_address_const: .word Fault_HandlerC\n"  // Define the address of Fault_HandlerC
    );
}

void Log_ResetReason(void) {
    
if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
    LOG_INFO("Reset due to NRST pin reset\r\n");
   }
   if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)) {
    LOG_INFO("Reset due to Power-on reset\r\n");
   }
   if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
    LOG_INFO("Reset due to Software reset\r\n");
   }
   if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) {
    LOG_INFO("Reset due to Window Watchdog reset\r\n");
    Log_StackCoreDump();
   }
   if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
    LOG_INFO("Reset due to Independent Watchdog reset\r\n");
    Log_StackCoreDump();
   }
   if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) {
    LOG_INFO("Reset due to Low Power reset\r\n");
   }

   /* Clear reset flags */
   __HAL_RCC_CLEAR_RESET_FLAGS();
}