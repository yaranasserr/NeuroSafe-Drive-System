/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/6/2023     ***************
 *******************************************************
 ******************************************************/


#ifndef NVIC_INTERFACE_H
#define NVIC_INTERFACE_H

void MNVIC_EnableInterrupt(u8 Copy_u8InterruptID);
void MNVIC_DisableInterrupt(u8 Copy_u8InterruptID);
void MNVIC_SetPendingFlag(u8 Copy_u8InterruptID);
void MNVIC_voidSetPriority(u8 Copy_u8InterruptID , u8 Copy_u8Priority);

#endif 