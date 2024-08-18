/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 2/07/2023     ***************
 *******************************************************
 ******************************************************/

#include "STD_TYPES.h"   // Standard types header file
#include "BIT_MATH.h"    // Bit manipulation header file

#include "TIMER_interface.h"  // Timer interface header file
#include "TIMER_config.h"     // Timer configuration header file
#include "TIMER_private.h"    // Timer private header file

void MTIM_voidInit(void)
{
    // Set prescaler value
    TIMER2->PSC = TIM_PSC;

    // Set center-aligned mode
    TIMER2->CR1 &= ~(0b11 << 5);
    TIMER2->CR1 |= (TIM_CMS << 5);

    // Set direction of counting
    TIMER2->CR1 &= ~(1 << 4);
    TIMER2->CR1 |= (TIM_DIR << 4);

    // Enable update interrupt
    TIMER2->DIER &= ~(1 << 0);
    TIMER2->DIER |= (TIM_UIE << 0);

    // Set prescaler clock source
    TIMER2->SMCR &= ~((0b111 << 0));
}

void MTIM_voidPWMUPCh1(u32 Copy_u32CRRValue)
{
    // Enable preload register on TIMx_CCR1
    TIMER2->CR1 |= (1 << 7);

    // Configure CC1 channel as output
    TIMER2->CCMR1 &= ~((0b11) << 0);

    // Set OC1 active high
    TIMER2->CCER &= ~(1 << 1);

    // Set PWM mode 1
    TIMER2->CCMR1 &= ~((0b111) << 4);
    TIMER2->CCMR1 |= ((0b110) << 4);

    // Enable preload register on TIMx_CCR1
    TIMER2->CCMR1 |= (1 << 3);

    // Enable the output on channel 1
    TIMER2->CCER |= (1 << 0);

    // Set auto-reload register value
    TIMER2->ARR = 60000;

    // Set capture/compare register value for channel 1
    TIMER2->CCR[0] = Copy_u32CRRValue;

    // Initialize all registers by setting UG bit in TIMx_EGR register
    TIMER2->EGR |= (1 << 0);

    // Enable counter
    TIMER2->CR1 |= (1 << 0);
}


void MTIM_voidPWMUPCh2(u32 Copy_u32CRRValue)
{
    // Enable preload register on TIMx_CCR2
    TIMER2->CR1 |= (1 << 7);

    // Configure CC2 channel as output
    TIMER2->CCMR1 &= ~((0b11) << 8);

    // Set OC2 active high
    TIMER2->CCER &= ~(1 << 5);

    // Set PWM mode 1
    TIMER2->CCMR1 &= ~((0b111) << 12);
    TIMER2->CCMR1 |= ((0b110) << 12);

    // Enable preload register on TIMx_CCR2
    TIMER2->CCMR1 |= (1 << 11);

    // Enable the output on channel 2
    TIMER2->CCER |= (1 << 4);

    // Set auto-reload register value
    TIMER2->ARR = 60000;

    // Set capture/compare register value for channel 2
    TIMER2->CCR[1] = Copy_u32CRRValue;

    // Initialize all registers by setting UG bit in TIMx_EGR register
    TIMER2->EGR |= (1 << 0);

    // Enable counter
    TIMER2->CR1 |= (1 << 0);
}

void MTIM_voidPWMUPCh3(u32 Copy_u32CRRValue)
{
    // Enable preload register on TIMx_CCR3
    TIMER2->CR1 |= (1 << 7);

    // Configure CC3 channel as output
    TIMER2->CCMR2 &= ~((0b11) << 0);

    // Set OC3 active high
    TIMER2->CCER &= ~(1 << 9);

    // Set PWM mode 1
    TIMER2->CCMR2 &= ~((0b111) << 4);
    TIMER2->CCMR2 |= ((0b110) << 4);

    // Enable preload register on TIMx_CCR3
    TIMER2->CCMR2 |= (1 << 3);

    // Enable the output on channel 3
    TIMER2->CCER |= (1 << 8);

    // Set auto-reload register value
    TIMER2->ARR = 60000;

    // Set capture/compare register value for channel 3
    TIMER2->CCR[2] = Copy_u32CRRValue;

    // Initialize all registers by setting UG bit in TIMx_EGR register
    TIMER2->EGR |= (1 << 0);

    // Enable counter
    TIMER2->CR1 |= (1 << 0);
}

void MTIM_voidPWMUPCh4(u32 Copy_u32CRRValue)
{
    // Enable preload register on TIMx_CCR4
    TIMER2->CR1 |= (1 << 7);

    // Configure CC4 channel as output
    TIMER2->CCMR2 &= ~((0b11) << 8);

    // Set OC4 active high
    TIMER2->CCER &= ~(1 << 13);

    // Set PWM mode 1
    TIMER2->CCMR2 &= ~((0b111) << 12);
    TIMER2->CCMR2 |= ((0b110) << 12);

    // Enable preload register on TIMx_CCR4
    TIMER2->CCMR1 |= (1 << 11);

    // Enable the output on channel 4
    TIMER2->CCER |= (1 << 12);

    // Set auto-reload register value
    TIMER2->ARR = 60000;

    // Set capture/compare register value for channel 4
    TIMER2->CCR[3] = Copy_u32CRRValue;

    // Initialize all registers by setting UG bit in TIMx_EGR register
    TIMER2->EGR |= (1 << 0);

    // Enable counter
    TIMER2->CR1 |= (1 << 0);
}