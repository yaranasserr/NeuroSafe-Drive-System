#ifndef TIM_CONFIG_H
#define TIM_CONFIG_H


/**

ICU_CHANNEL1_MAPPED_TIMER is Selected by :
										1- TIM_CHANNEL_OUTPUT
										2- TIM_CHANNEL_IC_TL1
										3- TIM_CHANNEL_IC_TL2
										4- TIM_CHANNEL_IC_TRC

**/
#define  ICU_CHANNEL1_MAPPED_TIMER			TIM_CHANNEL_IC_TL1




/**

ICU_CHANNEL2_MAPPED_TIMER is Selected by :
										1- TIM_CHANNEL_OUTPUT
										2- TIM_CHANNEL_IC_TL1
										3- TIM_CHANNEL_IC_TL2
										4- TIM_CHANNEL_IC_TRC

**/
#define  ICU_CHANNEL2_MAPPED_TIMER			TIM_CHANNEL_IC_TL1


/**
 *
 *
 * ICU_CHANNEL1_MAPPED_TIMER_TRIGGER is Selected by :
 * 												1-TIM_RISING_EDGE
 * 												2-TIM_FALLING_EDGE
 *
 *
 */


#define ICU_CHANNEL1_MAPPED_TIMER_TRIGGER		TIM_RISING_EDGE


/**
 *
 *
 * ICU_CHANNEL2_MAPPED_TIMER_TRIGGER is Selected by :
 * 												1-TIM_RISING_EDGE
 * 												2-TIM_FALLING_EDGE
 */


#define ICU_CHANNEL2_MAPPED_TIMER_TRIGGER			TIM_FALLING_EDGE


/**
 *  ICU_WITH_DMA	select  :
 *  						1-TIM_ENABLE
 *  						2-TIM_DISABLE
 *
 */

#define ICU_WITH_DMA	TIM_ENABLE
#endif
