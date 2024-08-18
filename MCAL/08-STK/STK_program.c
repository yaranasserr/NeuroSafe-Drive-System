/*
 * STK_program.c
 *
 *  Created on: Dec 14, 2023
 *      Author: osama
 */


#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "STK_interface.h"
#include "STK_private.h"
#include "STK_config.h"

#include "RCC_interface.h"
#include "RCC_private.h"
#include "RCC_config.h"

ptrFunction Systick_Callback_function = NULL;

static u8  Systick_mode;

void STK_voidSTKInit(void)
{
	/* Selecting STK clock source */
	STK->CTRL |= (STK_FREQUENCY<<2);

	/* Selecting whether to enable STK exception requests or not */
	STK->CTRL |= (STK_EXCEPTION_ENABLE<<1);

}


void STK_voidWaitBlock(u32 copy_u32Millis)
{
	u32 local_u32ticks = 0;

	if(RCC_SW == RCC_HSE || RCC_SW == RCC_HSI)
	{
		local_u32ticks = (copy_u32Millis/1000)*1000000;
	}

	/* Selecting STK clock source */
	STK->CTRL |= (STK_FREQUENCY<<2);

	/* Selecting whether to enable STK exception requests or not */
	STK->CTRL |= (0<<1);

	/* Load Ticks value to the register */
	STK->LOAD = local_u32ticks;

	/* Enable counter to load the value and start counting */
	STK->CTRL |= (1<<0);

	/* Wait until Ticks are cleared */
	while(GET_BIT(STK->CTRL, 16) == 0);

	/* Disable counter */
	STK->CTRL |= (0<<0);
	STK->LOAD = 0;
	STK->VAL = 0;


}

void STK_voidSingleInterval(u32 copy_u32Millis , ptrFunction callBack)
{
	u32 local_u32ticks = 0;

	if(RCC_SW == RCC_HSE || RCC_SW == RCC_HSI)
	{
		local_u32ticks = (copy_u32Millis/1000)*1000000;
	}

	/* Selecting STK clock source */
	STK->CTRL |= (STK_FREQUENCY<<2);

	/* Selecting whether to enable STK exception requests or not */
	STK->CTRL |= (1<<1);

	/*  */

	Systick_Callback_function = callBack;

	/* Load Ticks value to the register */
	STK->LOAD = local_u32ticks;

	/* Selecting SYSTICK mode*/
	Systick_mode = SYSTICK_SINGLE_INTERVAL_MODE;

	/* Enable counter to load the value and start counting */
	STK->CTRL |= (1<<0);
}

void STK_voidPeriodicInterval(u32 copy_u32Millis , ptrFunction callBack)
{
	u32 local_u32ticks = 0;

	if(RCC_SW == RCC_HSE || RCC_SW == RCC_HSI)
	{
		local_u32ticks = (copy_u32Millis/1000)*1000000;
	}

	/* Selecting STK clock source */
	STK->CTRL |= (STK_FREQUENCY<<2);

	/* Selecting whether to enable STK exception requests or not */
	STK->CTRL |= (1<<1);

	/*  */

	Systick_Callback_function = callBack;

	/* Load Ticks value to the register */
	STK->LOAD = local_u32ticks;

	/* Selecting SYSTICK mode*/
	Systick_mode = SYSTICK_PERIODIC_INTERVAL_MODE;

	/* Enable counter to load the value and start counting */
	STK->CTRL |= (1<<0);
}


void SysTick_Handler(void)
{
	if (Systick_mode == SYSTICK_SINGLE_INTERVAL_MODE)
	{

		/* Disable counter */
		STK->CTRL |= (0<<0);
		STK->LOAD = 0;
		STK->VAL = 0;

		Systick_Callback_function();
	}
	else
	{
		Systick_Callback_function();
	}

}
