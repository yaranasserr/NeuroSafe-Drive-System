/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/18/2023     ***************
 *******************************************************
 ******************************************************/

#ifndef AFIO_PRIVATE_H
#define AFIO_PRIVATE_H

#define AFIO_BASE_ADDRESS     0x40010000  // Base address of AFIO peripheral

typedef struct
{
    volatile u32 EVCR;          // Event control register
    volatile u32 MAPR;          // AF remap and debug I/O configuration register
    volatile u32 EXTICR[4];     // External interrupt configuration registers
    volatile u32 Reserved;      // Reserved
    volatile u32 MAPR2;         // AF remap and debug I/O configuration register 2

} AFIO_t;  // AFIO peripheral structure

#define AFIO         ((volatile AFIO_t*)AFIO_BASE_ADDRESS)  // Pointer to AFIO peripheral

#endif  
