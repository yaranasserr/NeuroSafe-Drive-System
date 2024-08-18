/*
 * STK_interface.h
 *
 *  Created on: Dec 14, 2023
 *      Author: osama
 */

#ifndef STK_INTERFACE_H_
#define STK_INTERFACE_H_

#define SYSTICK_SINGLE_INTERVAL_MODE 0
#define SYSTICK_PERIODIC_INTERVAL_MODE 1

/*
 * Inputs : void
 * Output : void
 * Description : Initializes the STK Timer frequency and exception enable
 */

void STK_voidSTKInit(void);


/*
 * Inputs : Delay in milliseconds
 * Output : void
 * Description : Generates a software delay in millis
 */

void STK_voidWaitBlock(u32 copy_u32Millis);


/*
 * Inputs : Delay in milliseconds
 * Output : void
 * Description : Used to execute a user defined ISR after a certain amount of time in millis for a once
 */
void STK_voidSingleInterval(u32 copy_u32Millis , ptrFunction callBack);


/*
 * Inputs : Delay in milliseconds
 * Output : void
 * Description : Used to execute a user defined ISR after a certain amount of time in millis periodically
 */

void STK_voidPeriodicInterval(u32 copy_u32Millis , ptrFunction callBack);


#endif /* STK_INTERFACE_H_ */
