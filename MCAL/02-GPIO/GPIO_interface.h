/*******************************************************
 *******************************************************
 *****************  Author : Noha Elnemr ***************
 *****************  version : v1.0       ***************
 *****************  Date : 12/2/2023     ***************
 *******************************************************
 *******************************************************
 */

#ifndef GPIO_INTERFACE_H_
#define GPIO_INTERFACE_H_

#include "STD_TYPES.h"
#include "BIT_MATH.h"

/*PORTS Definition*/
#define PORTA                     0
#define PORTB                     1
#define PORTC                     2

/*PINS Definition*/
#define PIN0                      0
#define PIN1                      1
#define PIN2                      2
#define PIN3                      3
#define PIN4                      4
#define PIN5                      5
#define PIN6                      6
#define PIN7                      7
#define PIN8                      8
#define PIN9                      9
#define PIN10                     10
#define PIN11                     11
#define PIN12                     12
#define PIN13                     13
#define PIN14                     14
#define PIN15                     15

/*Modes Definition*/
#define GPIO_GP_OUTPUT_PP_10MHZ   0b0001
#define GPIO_GP_OUTPUT_PP_2MHZ    0b0010
#define GPIO_GP_OUTPUT_PP_50MHZ   0b0011

#define GPIO_GP_OUTPUT_OD_10MHZ   0b0101
#define GPIO_GP_OUTPUT_OD_2MHZ    0b0110
#define GPIO_GP_OUTPUT_OD_50MHZ   0b0111

#define GPIO_AF_OUTPUT_PP_10MHZ   0b1001
#define GPIO_AF_OUTPUT_PP_2MHZ    0b1010
#define GPIO_AF_OUTPUT_PP_50MHZ   0b1011

#define GPIO_AF_OUTPUT_OD_10MHZ   0b1101
#define GPIO_AF_OUTPUT_OD_2MHZ    0b1110
#define GPIO_AF_OUTPUT_OD_50MHZ   0b1111

#define GPIO_INPUT_ANALOG         0b0000
#define GPIO_INPUT_FLOATING       0b0100
#define GPIO_INPUT_PU_PD          0b1000

/*Values Definition*/
#define GPIO_HIGH                 1
#define GPIO_LOW                  0

/*prototypes functions*/
u8 GPIO_voidSetPinMode(u8 copy_u8Port, u8 copy_u8Pin , u8 copy_u8Mode);
u8 GPIO_voidSetPinValue(u8 copy_u8Port, u8 copy_u8Pin , u8 copy_u8Value);
u8 GPIO_voidGetPinValue(u8 copy_u8Port, u8 copy_u8Pin);
u8 GPIO_u8SetPortValue(u8 Copy_u8Port , u16 Copy_u16Value);

#endif
