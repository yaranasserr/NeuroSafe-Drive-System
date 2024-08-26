#ifndef MOTOR_H_
#define MOTOR_H_

#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "RCC_interface.h"
#include "GPIO_interface.h"
#include "NVIC_interface.h"
#include "AFIO_interface.h"
#include "EXTI_interface.h"
#include "SCB_interface.h"
#include "STK_interface.h"
#include "TIMER_interface.h"

u8 Motor_u8MoveForward();
u8 Motor_u8MoveBackward();
u8 Motor_u8TurnRight();
u8 Motor_u8TurnLeft();
u8 Motor_u8Stop();


#endif /* MOTOR_H_ */
