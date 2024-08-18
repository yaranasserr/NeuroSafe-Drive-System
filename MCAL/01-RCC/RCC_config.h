/*******************************************************
 *******************************************************
 *****************  Author : Osama Omar  ***************
 *****************  version : v1.0       ***************
 *****************  Date : 12/2/2023     ***************
 *******************************************************
 *******************************************************
 */


#ifndef RCC_CONFIG_H
#define RCC_CONFIG_H

#include "RCC_interface.h"

#define ON  1
#define OFF 0

/* Choose Which clock sources should be enabled :

		 HSE_EN ON  -> HSE enabled
		 HSE_EN OFF -> HSE disabled

		 HSI_EN ON  -> HSI enabled
		 HSI_EN OFF -> HSI disabled

		 PLL_EN ON  -> PLL enabled
		 PLL_EN OFF -> PLL disabled
 */
#define RCC_HSE_ON ON
#define RCC_HSI_ON ON
#define RCC_PLL_ON ON


/*
   Select MCO Output source :

 	 MCO 0xx -> No clock
 	 MCO 100 -> SYSCLK Selected
 	 MCO 101 -> HSI Selected
 	 MCO 110 -> HSE Selected
 	 MCO 111 -> PLL divided by 2
 */

#define RCC_MCO 0b000


/*
  Select PLL Multiplication factor :

 	PLL_MUL 0000 -> PLL input clock x 2
	PLL_MUL 0001 -> PLL input clock x 3
	PLL_MUL 0010 -> PLL input clock x 4
	PLL_MUL 0011 -> PLL input clock x 5
	PLL_MUL 0100 -> PLL input clock x 6
	PLL_MUL 0101 -> PLL input clock x 7
	PLL_MUL 0110 -> PLL input clock x 8
	PLL_MUL 0111 -> PLL input clock x 9
	PLL_MUL 1000 -> PLL input clock x 10
	PLL_MUL 1001 -> PLL input clock x 11
	PLL_MUL 1010 -> PLL input clock x 12
	PLL_MUL 1011 -> PLL input clock x 13
	PLL_MUL 1100 -> PLL input clock x 14
	PLL_MUL 1101 -> PLL input clock x 15
	PLL_MUL 1110 -> PLL input clock x 16
	PLL_MUL 1111 -> PLL input clock x 16
*/

#define RCC_PLL_MUL 0b0010



/*
  Select PLL source :

 	PLLSRC 0 -> HSI divided by 2
	PLLSRC 1 -> HSE Selected

*/

#define RCC_PLLSRC 1


/*
  Select HSE divider for PLL source :

 	PLLXTPRE 0 -> HSE not divided
	PLLXTPRE 1 -> HSE divided by 2

*/

#define RCC_PLLXTPRE 0


/*
  Select AHB prescaler :

 	HPRE 0xxx: SYSCLK not divided
	HPRE 1000: SYSCLK divided by 2
	HPRE 1001: SYSCLK divided by 4
	HPRE 1010: SYSCLK divided by 8
	HPRE 1011: SYSCLK divided by 16
	HPRE 1100: SYSCLK divided by 64
	HPRE 1101: SYSCLK divided by 128
	HPRE 1110: SYSCLK divided by 256
	HPRE 1111: SYSCLK divided by 512
*/

#define RCC_HPRE 0b0000


/*
  Select APB1 prescaler :

 	PPRE1 0xx -> HCLK not divided
	PPRE1 100 -> HCLK divided by 2
	PPRE1 101 -> HCLK divided by 4
	PPRE1 110 -> HCLK divided by 8
	PPRE1 111 -> HCLK divided by 16

*/

#define RCC_PPRE1 0b000


/*
  Select APB2 prescaler :

 	PPRE2 0xx -> HCLK not divided
	PPRE2 100 -> HCLK divided by 2
	PPRE2 101 -> HCLK divided by 4
	PPRE2 110 -> HCLK divided by 8
	PPRE2 111 -> HCLK divided by 16

*/

#define RCC_PPRE2 0b000


/*
  Select system clock switch :

 	SW 00 -> HSI is selected as SYSCLK
	SW 01 -> HSE is selected as SYSCLK
	SW 10 -> PLL is selected as SYSCLK
	SW 11 -> not allowed

*/

#define RCC_SW RCC_PLL








#endif
