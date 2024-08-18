/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/18/2023     ***************
 *******************************************************
 ******************************************************/

#ifndef EXTI_INTERFACE_H
#define EXTI_INTERFACE_H

/* Line options */
#define EXTI_LINE0                          0
#define EXTI_LINE1                          1
#define EXTI_LINE2                          2
#define EXTI_LINE3                          3
#define EXTI_LINE4                          4
#define EXTI_LINE5                          5
#define EXTI_LINE6                          6
#define EXTI_LINE7                          7
#define EXTI_LINE8                          8
#define EXTI_LINE9                          9
#define EXTI_LINE10                         10
#define EXTI_LINE11                         11
#define EXTI_LINE12                         12
#define EXTI_LINE13                         13
#define EXTI_LINE14                         14
#define EXTI_LINE15                         15

/* Trigger Point options */
#define EXTI_RISING                         0
#define EXTI_FALLING                        1
#define EXTI_ONCHANGE                       2

/* Function prototypes */
void MEXTI_voidEnableTriggerLine(u8 Copy_u8EXTIn, u8 Copy_u8Trigger);
/*
 * Description: Enables the trigger for the specified external interrupt line.
 * Parameters:
 * - Copy_u8EXTIn: External interrupt line number (EXTI_LINE0 to EXTI_LINE15).
 * - Copy_u8Trigger: Trigger point (EXTI_RISING, EXTI_FALLING, or EXTI_ONCHANGE).
 */

void MEXTI_voidDisableTriggerLine(u8 Copy_u8EXTIn);
/*
 * Description: Disables the trigger for the specified external interrupt line.
 * Parameters:
 * - Copy_u8EXTIn: External interrupt line number (EXTI_LINE0 to EXTI_LINE15).
 */

void MEXTI_voidCallBack(u8 Copy_u8EXTIn, void (*EXTInFunc)(void));
/*
 * Description: Sets a callback function to be executed when the specified external interrupt occurs.
 * Parameters:
 * - Copy_u8EXTIn: External interrupt line number (EXTI_LINE0 to EXTI_LINE15).
 * - EXTInFunc: Pointer to the callback function.
 */

#endif  
