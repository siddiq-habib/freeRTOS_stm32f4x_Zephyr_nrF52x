#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h" // Include the header file for your specific STM32 device

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

void Fault_HandlerC(StackFrame *stackFrame)
{
    // Access the stack frame and retrieve information
    uint32_t r0 = stackFrame->r0;
    uint32_t r1 = stackFrame->r1;
    uint32_t r2 = stackFrame->r2;
    uint32_t r3 = stackFrame->r3;
    uint32_t r12 = stackFrame->r12;
    uint32_t lr = stackFrame->lr;
    uint32_t pc = stackFrame->pc;
    uint32_t psr = stackFrame->psr;

    // Read fault status registers
    uint32_t hfsr = SCB->HFSR;
    uint32_t cfsr = SCB->CFSR;
    uint32_t mmfar = SCB->MMFAR;
    uint32_t bfar = SCB->BFAR;
    uint32_t afsr = SCB->AFSR;

    // Print the retrieved information
    printf("R0: 0x%08lX\n", r0);
    printf("R1: 0x%08lX\n", r1);
    printf("R2: 0x%08lX\n", r2);
    printf("R3: 0x%08lX\n", r3);
    printf("R12: 0x%08lX\n", r12);
    printf("LR: 0x%08lX\n", lr);
    printf("PC: 0x%08lX\n", pc);
    printf("PSR: 0x%08lX\n", psr);
    printf("HFSR: 0x%08lX\n", hfsr);
    printf("CFSR: 0x%08lX\n", cfsr);
    printf("MMFAR: 0x%08lX\n", mmfar);
    printf("BFAR: 0x%08lX\n", bfar);
    printf("AFSR: 0x%08lX\n", afsr);

    // Disable all breakpoints and halt the system
    __asm volatile ("bkpt #0"); // Trigger a breakpoint to halt the system
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