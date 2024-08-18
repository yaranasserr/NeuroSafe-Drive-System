/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/18/2023     ***************
 *******************************************************
 ******************************************************/

#include "STD_TYPES.h"       // Include standard data types header
#include "BIT_MATH.h"        // Include bit manipulation macros header

#include "EXTI_private.h"    // Include private EXTI header
#include "EXTI_config.h"     // Include EXTI configuration header
#include "EXTI_interface.h"  // Include EXTI interface header

/* Global Pointers to Functions */
void (*EXTI_EXTI0PtrFunc)(void) = NULL;
void (*EXTI_EXTI1PtrFunc)(void) = NULL;
void (*EXTI_EXTI2PtrFunc)(void) = NULL;
void (*EXTI_EXTI3PtrFunc)(void) = NULL;
void (*EXTI_EXTI4PtrFunc)(void) = NULL;
void (*EXTI_EXTI5_9PtrFunc)(void) = NULL;
void (*EXTI_EXTI10_15PtrFunc)(void) = NULL;

void MEXTI_voidEnableTriggerLine(u8 Copy_u8EXTIn, u8 Copy_u8Trigger)
{
    /* Enable Interrupt */
    EXTI->IMR |= (1 << Copy_u8EXTIn);

    /* Set Trigger Point */
    switch (Copy_u8Trigger)
    {
    case EXTI_RISING:
        EXTI->RTSR |= (1 << Copy_u8EXTIn);
        break;
    case EXTI_FALLING:
        EXTI->FTSR |= (1 << Copy_u8EXTIn);
        break;
    case EXTI_ONCHANGE:
        EXTI->RTSR |= (1 << Copy_u8EXTIn);
        EXTI->FTSR |= (1 << Copy_u8EXTIn);
        break;
    default:
        // Handle error
        break;
    }
}

void MEXTI_voidDisableTriggerLine(u8 Copy_u8EXTIn)
{
    /* Disable Interrupt */
    EXTI->IMR &= ~(1 << Copy_u8EXTIn);
}

void MEXTI_voidCallBack(u8 Copy_u8EXTIn, void (*EXTInFunc)(void))
{
    /* Assign Callback Function Pointer Based on EXTI Line */
    switch (Copy_u8EXTIn)
    {
    case EXTI_LINE0:
        EXTI_EXTI0PtrFunc = EXTInFunc;
        break;
    case EXTI_LINE1:
        EXTI_EXTI1PtrFunc = EXTInFunc;
        break;
    case EXTI_LINE2:
        EXTI_EXTI2PtrFunc = EXTInFunc;
        break;
    case EXTI_LINE3:
        EXTI_EXTI3PtrFunc = EXTInFunc;
        break;
    case EXTI_LINE4:
        EXTI_EXTI4PtrFunc = EXTInFunc;
        break;
    case EXTI_LINE5:
    case EXTI_LINE6:
    case EXTI_LINE7:
    case EXTI_LINE8:
    case EXTI_LINE9:
        EXTI_EXTI5_9PtrFunc = EXTInFunc;
        break;
    case EXTI_LINE10:
    case EXTI_LINE11:
    case EXTI_LINE12:
    case EXTI_LINE13:
    case EXTI_LINE14:
    case EXTI_LINE15:
        EXTI_EXTI10_15PtrFunc = EXTInFunc;
        break;
    default:
        // Handle error for invalid EXTI line
        break;
    }
}

/* Interrupt Service Routines (ISR) */
void EXTI0_IRQHandler(void)
{
    EXTI_EXTI0PtrFunc();  // Call function pointer for EXTI line 0
}

void EXTI1_IRQHandler(void)
{
    EXTI_EXTI1PtrFunc();  // Call function pointer for EXTI line 1
}

void EXTI2_IRQHandler(void)
{
    EXTI_EXTI2PtrFunc();  // Call function pointer for EXTI line 2
}

void EXTI3_IRQHandler(void)
{
    EXTI_EXTI3PtrFunc();  // Call function pointer for EXTI line 3
}

void EXTI4_IRQHandler(void)
{
    EXTI_EXTI4PtrFunc();  // Call function pointer for EXTI line 4
}

void EXTI9_5IRQHandler(void)
{
    EXTI_EXTI5_9PtrFunc();  // Call function pointer for EXTI lines 5 to 9
}

void EXTI15_10IRQHandler(void)
{
    EXTI_EXTI10_15PtrFunc();  // Call function pointer for EXTI lines 10 to 15
}
