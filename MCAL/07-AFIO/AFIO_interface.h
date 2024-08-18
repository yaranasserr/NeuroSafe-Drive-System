/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/18/2023     ***************
 *******************************************************
 ******************************************************/

#ifndef AFIO_INTERFACE_H
#define AFIO_INTERFACE_H

/* Line options */
#define AFIO_LINE0                          0
#define AFIO_LINE1                          1
#define AFIO_LINE2                          2
#define AFIO_LINE3                          3
#define AFIO_LINE4                          4
#define AFIO_LINE5                          5
#define AFIO_LINE6                          6
#define AFIO_LINE7                          7
#define AFIO_LINE8                          8
#define AFIO_LINE9                          9
#define AFIO_LINE10                         10
#define AFIO_LINE11                         11
#define AFIO_LINE12                         12
#define AFIO_LINE13                         13
#define AFIO_LINE14                         14
#define AFIO_LINE15                         15

/* Port options */
#define AFIO_PortA                          0b0000
#define AFIO_PortB                          0b0001
#define AFIO_PortC                          0b0010

/* Peripheral remapping options */
#define SPI1_REMAP                          0
#define I2C1_REMAP                          1
#define UART1_REMAP                         2
#define UART2_REMAP                         3
#define UART3_REMAP                         4
#define TIM1_REMAP                          6
#define TIM2_REMAP                          8
#define TIM3_REMAP                          10
#define TIM4_REMAP                          12
#define CAN_REMAP                           13

/* Function prototypes */
void MAFIO_voidEXTIConfiguration(u8 Copy_u8EXTIn, u8 Copy_u8Port);
/*
 * Description: Configure EXTI line to be connected to a specific GPIO port.
 * Parameters:
 * - Copy_u8EXTIn: EXTI line number (AFIO_LINE0 to AFIO_LINE15).
 * - Copy_u8Port: GPIO port to be connected to the EXTI line (AFIO_PortA, AFIO_PortB, or AFIO_PortC).
 */

void MAFIO_voidRemapPeripheralPins(u8 Copy_u8Port);
/*
 * Description: Remap peripheral pins to alternative GPIO pins.
 * Parameters:
 * - Copy_u8Port: GPIO port to be remapped (AFIO_PortA, AFIO_PortB, or AFIO_PortC).
 */

#endif  
