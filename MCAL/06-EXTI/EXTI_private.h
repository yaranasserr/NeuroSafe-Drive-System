/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/18/2023     ***************
 *******************************************************
 ******************************************************/

#ifndef EXTI_PRIVATE_H
#define EXTI_PRIVATE_H

#define EXTI_BASE_ADDRESS     0x40010400  // Base address of EXTI peripheral

typedef struct
{
    volatile u32 IMR;    // Interrupt mask register
    volatile u32 EMR;    // Event mask register
    volatile u32 RTSR;   // Rising trigger selection register
    volatile u32 FTSR;   // Falling trigger selection register
    volatile u32 SWIER;  // Software interrupt event register
    volatile u32 PR;     // Pending register

} EXTI_t;  // EXTI peripheral structure

#define EXTI         ((volatile EXTI_t*)EXTI_BASE_ADDRESS)  // Pointer to EXTI peripheral

#endif 
