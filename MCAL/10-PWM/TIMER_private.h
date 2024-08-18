/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 2/07/2023     ***************
 *******************************************************
 ******************************************************/

#ifndef TIMER_PRIVATE_H
#define TIMER_PRIVATE_H

#define TIMER2_BASE_ADDRESS              0x40000000  // Base address of Timer 2 peripheral
#define TIMER3_BASE_ADDRESS              0x40000400  // Base address of Timer 3 peripheral

typedef struct
{
    volatile u32 CR1;       // Control register 1
    volatile u32 CR2;       // Control register 2
    volatile u32 SMCR;      // Slave mode control register
    volatile u32 DIER;      // DMA/Interrupt enable register
    volatile u32 SR;        // Status register
    volatile u32 EGR;       // Event generation register
    volatile u32 CCMR1;     // Capture/compare mode register 1
    volatile u32 CCMR2;     // Capture/compare mode register 2
    volatile u32 CCER;      // Capture/compare enable register
    volatile u32 CNT;       // Counter register
    volatile u32 PSC;       // Prescaler register
    volatile u32 ARR;       // Auto-reload register
    volatile u32 Reserved1; // Reserved
    volatile u32 CCR[4];    // Capture/compare registers
    volatile u32 Reserved2; // Reserved
    volatile u32 DCR;       // DMA control register
    volatile u32 DMAR;      // DMA address for full transfer
} timer_t;                  // Timer peripheral structure

#define TIMER2            ((volatile timer_t*)TIMER2_BASE_ADDRESS)  // Pointer to Timer 2 peripheral
#define TIMER3            ((volatile timer_t*)TIMER3_BASE_ADDRESS)  // Pointer to Timer 3 peripheral

#endif  
