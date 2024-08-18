/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/6/2023     ***************
 *******************************************************
 ******************************************************/

#include "STD_TYPES.h"       // Include standard data types header
#include "BIT_MATH.h"        // Include bit manipulation macros header
#include "NVIC_interface.h"  // Include NVIC interface header
#include "NVIC_private.h"    // Include NVIC private header
#include "NVIC_config.h"     // Include NVIC configuration header

void MNVIC_EnableInterrupt(u8 Copy_u8InterruptID)
{
    NVIC->ISER[Copy_u8InterruptID / 32] = 1 << (Copy_u8InterruptID % 32);
    // Enable the specified interrupt by setting the corresponding bit in the ISER register
}

void MNVIC_DisableInterrupt(u8 Copy_u8InterruptID)
{
    NVIC->ICER[Copy_u8InterruptID / 32] = 1 << (Copy_u8InterruptID % 32);
    // Disable the specified interrupt by setting the corresponding bit in the ICER register
}

void MNVIC_SetPendingFlag(u8 Copy_u8InterruptID)
{
    NVIC->ISPR[Copy_u8InterruptID / 32] = 1 << (Copy_u8InterruptID % 32);
    // Set the pending flag for the specified interrupt by setting the corresponding bit in the ISPR register
}

void MNVIC_voidSetPriority(u8 Copy_u8InterruptID, u8 Copy_u8Priority)
{
    if (Copy_u8InterruptID < 60)
    {
        NVIC->IPR[Copy_u8InterruptID] = Copy_u8Priority;
        // Set the priority for the specified interrupt in the IPR register
    }
    else
    {
        // Handle error for invalid interrupt ID
    }
}
