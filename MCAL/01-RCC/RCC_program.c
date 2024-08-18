/*******************************************************
 *******************************************************
 *****************  Author : Osama Omar  ***************
 *****************  version : v1.0       ***************
 *****************  Date : 12/2/2023     ***************
 *******************************************************
 *******************************************************
 */

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "RCC_interface.h"
#include "RCC_config.h"
#include "RCC_private.h"




void RCC_voidClockInit(void)
{
	/* MCO Configuration */
	RCC->CFGR = 0X00;
	RCC->CFGR |= (RCC_MCO<<24);

	/* PLL Multiplication Factor */
	RCC->CFGR |= (RCC_PLL_MUL<<18);

	/* PLL Source */
	RCC->CFGR |= (RCC_PLLSRC<<16);

	/* HSE divider for PLL Source */
	RCC->CFGR |= (RCC_PLLXTPRE<<17);

	/* Setting AHB prescaler */
	RCC->CFGR |= (RCC_HPRE<<4);

	/* Setting APB1 prescaler */
	RCC->CFGR |= (RCC_PPRE1<<8);

	/* Setting APB2 prescaler */
	RCC->CFGR |= (RCC_PPRE2<<11);

	/* Enable whether HSE, HSI, PLL to be used*/
	RCC->CR |= (RCC_HSI_ON<<0);
	RCC->CR |= (RCC_HSE_ON<<16);
	RCC->CR |= (RCC_PLL_ON<<24);

	/* Select SYSCLK */
	RCC->CFGR |= (RCC_SW<<0);





}

void RCC_voidPeripheralClockEnable(u8 copy_u8BusName, peripheral_names_t copy_u8peripheralName)
{
	switch(copy_u8BusName)
	{

	/* Enable a peripheral on AHB bus */
	case RCC_AHB :
		SET_BIT(RCC->AHBENR, copy_u8peripheralName);
		//RCC->AHBENR |= (1<<copy_u8peripheralName);
		break;

	/* Enable a peripheral on APB1 bus */
	case RCC_APB1 :
		SET_BIT(RCC->APB1ENR, copy_u8peripheralName);
		//RCC->APB1ENR |= (1<<copy_u8peripheralName);
		break;

	/* Enable a peripheral on APB2 bus */
	case RCC_APB2 :
		SET_BIT(RCC->APB2ENR, copy_u8peripheralName);
		//RCC->APB2ENR |= (1<<copy_u8peripheralName);
		break;
	default :
		break;

	}

}

void RCC_voidPeripheralClockDisable(u8 copy_u8BusName, peripheral_names_t copy_u8peripheralName)
{

	switch(copy_u8BusName)
		{
	    /* disable a peripheral on AHB bus */
		case RCC_AHB :
			SET_BIT(RCC->AHBENR, copy_u8peripheralName);
			break;

		/* disable a peripheral on APB1 bus */
		case RCC_APB1 :
				SET_BIT(RCC->APB1ENR, copy_u8peripheralName);
				break;

		/* disable a peripheral on APB2 bus */
		case RCC_APB2 :
				SET_BIT(RCC->APB2ENR, copy_u8peripheralName);
				break;
		default :
			break;

		}
}



