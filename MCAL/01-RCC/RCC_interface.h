/*******************************************************
 *******************************************************
 *****************  Author : Osama Omar  ***************
 *****************  version : v1.0       ***************
 *****************  Date : 12/2/2023     ***************
 *******************************************************
 *******************************************************
 */

#ifndef RCC_INTERFACE_H
#define RCC_INTERFACE_H

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#define RCC_AHB  0
#define RCC_APB1 1
#define RCC_APB2 2

#define RCC_HSI 0b00
#define RCC_HSE 0b01
#define RCC_PLL 0b10


typedef enum
{
	DMA1,
	DMA2,
	SRAM,
	FLITF=4,
	CRC=6,
	FSMC=8,
	SDIO=10,
	TIM2=0,
	TIM3,
	TIM4,
	TIM5,
	TIM6,
	TIM7,
	TIM12,
	TIM13,
	TIM14,
	WWDG=11,
	SPI2=14,
	SPI3,
	USART2=17,
	USART3,
	USAR4,
	USART5,
	I2C1,
	I2C2,
	USB,
	CAN1=25,
	BKP=27,
	PWR,
	DAC,
	AFIO=0,
	IOPA=2,
	IOPB,
	IOPC,
	IOPD,
	IOPE,
	IOPF,
	IOPG,
	ADC1,
	ADC2,
	TIM1,
	SPI1,
	TIM8,
	USART1,
	ADC3,
	TIM9=19,
	TIM10,
	TIM11

}peripheral_names_t;


/*
 * @funciton description : Initializes the system clock according to the user configuration
 * @param  : void
 * @return : void
 * */
void RCC_voidClockInit(void);


/*
 * @funciton description : Enables a peripheral clock to be used
 * @param  : Takes the name of the BUS the peripheral is on NAME of the Bus should be (RCC_AHB,
 * 			 RCC_APB1, RCC_APB2) and also takes the name of the peripheral.
 * @return : void
 * */
void RCC_voidPeripheralClockEnable(u8 copy_u8BusName, peripheral_names_t copy_u8peripheralName);


/*
 * @funciton description : disables a peripheral clock to be used
 * @param  : Takes the name of the BUS the peripheral is on NAME of the Bus should be (AHB, APB1, APB2)
 * 			 and also takes the name of the peripheral
 * @return : void
 * */
void RCC_voidPeripheralClockDisable(u8 copy_u8BusName, peripheral_names_t copy_u8peripheralName);





#endif
