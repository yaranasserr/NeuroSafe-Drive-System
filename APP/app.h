/*
 * app.h
 *
 *  Created on: Apr 28, 2024
 *      Author: Noha Elnemr
 */

#ifndef APP_H_
#define APP_H_

#include "RCC_interface.h"
#include "GPIO_interface.h"
#include "NVIC_interface.h"
#include "AFIO_interface.h"
#include "EXTI_interface.h"
#include "SCB_interface.h"
#include "STK_interface.h"
#include "TIMER_interface.h"
#include "CAN_interface.h"
#include "Motor.h"

u8 Directions_Received[5];
CAN_FilterHeader_t can_filter;
CAN_RxHeader_t can_rx;


u8 APP_u8AppInit();
u8 APP_u8InitializeMotorPins();
u8 APP_u8ReceiveDataFromCANBus();
u8 APP_u8CreateFilter();
u8 APP_u8DirectionDecision();



#endif /* APP_H_ */
