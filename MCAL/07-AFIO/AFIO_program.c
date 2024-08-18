/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/18/2023     ***************
 *******************************************************
 ******************************************************/


#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "AFIO_private.h"
#include "AFIO_config.h"
#include "AFIO_interface.h"


void MAFIO_voidEXTIConfiguration(u8 Copy_u8EXTIn , u8 Copy_u8Port)
{
	if(Copy_u8EXTIn <= 3)
	{
		AFIO->EXTICR[0] &= ~(0b1111 << (Copy_u8EXTIn * 4));
		AFIO->EXTICR[0] |= (Copy_u8Port << (Copy_u8EXTIn * 4));
	}
	else if(Copy_u8EXTIn <= 7)
	{
		AFIO->EXTICR[1] &= ~(0b1111 << (Copy_u8EXTIn * 4));
		AFIO->EXTICR[1] |= (Copy_u8Port << (Copy_u8EXTIn * 4));
	}
	else if(Copy_u8EXTIn <= 11)
	{
		AFIO->EXTICR[2] &= ~(0b1111 << (Copy_u8EXTIn * 4));
		AFIO->EXTICR[2] |= (Copy_u8Port << (Copy_u8EXTIn * 4));
	}
	else if(Copy_u8EXTIn <= 15)
	{
		AFIO->EXTICR[3] &= ~(0b1111 << (Copy_u8EXTIn * 4));
		AFIO->EXTICR[3] |= (Copy_u8Port << (Copy_u8EXTIn * 4));
	}
}

void MAFIO_voidRemapPeripheralPins(u8 Copy_u8Port)
{
	switch(Copy_u8Port)
	{
	case SPI1_REMAP:
		SET_BIT(AFIO->MAPR, SPI1_REMAP);
	case I2C1_REMAP:
		SET_BIT(AFIO->MAPR, I2C1_REMAP);
	case CAN_REMAP:
		CLR_BIT(AFIO->MAPR, CAN_REMAP);
		CLR_BIT(AFIO->MAPR, (CAN_REMAP+1));

	}
}
