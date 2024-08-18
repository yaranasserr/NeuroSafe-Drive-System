/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 2/07/2023     ***************
 *******************************************************
 ******************************************************/

#ifndef TIMER_INTERFACE_H
#define TIMER_INTERFACE_H

/* Timers options */
#define TIMER_2         0  // Timer 2
#define TIMER_3         1  // Timer 3

/* Function prototypes */

/**
 * @brief Initialize the timer module.
 * 
 * This function initializes the timer module.
 */
void MTIM_voidInit(void);

/**
 * @brief Set PWM duty cycle for channel 1 in upcounting mode.
 * 
 * @param Copy_u32CRRValue The value to set for the capture/compare register.
 */
void MTIM_voidPWMUPCh1(u32 Copy_u32CRRValue);

/**
 * @brief Set PWM duty cycle for channel 2 in upcounting mode.
 * 
 * @param Copy_u32CRRValue The value to set for the capture/compare register.
 */
void MTIM_voidPWMUPCh2(u32 Copy_u32CRRValue);

/**
 * @brief Set PWM duty cycle for channel 3 in upcounting mode.
 * 
 * @param Copy_u32CRRValue The value to set for the capture/compare register.
 */
void MTIM_voidPWMUPCh3(u32 Copy_u32CRRValue);

/**
 * @brief Set PWM duty cycle for channel 4 in upcounting mode.
 * 
 * @param Copy_u32CRRValue The value to set for the capture/compare register.
 */
void MTIM_voidPWMUPCh4(u32 Copy_u32CRRValue);

#endif  