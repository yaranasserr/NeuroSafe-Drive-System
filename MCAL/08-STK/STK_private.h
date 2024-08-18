/*
 * STK_private.h
 *
 *  Created on: Dec 14, 2023
 *      Author: osama
 */

#ifndef STK_PRIVATE_H_
#define STK_PRIVATE_H_

#define STK_BASE_ADDRESS     0xE000E010


typedef struct
{
	u32 CTRL;
	u32 LOAD;
	u32 VAL;
	u32 CALIB;
}STK_t;



#define STK     ((volatile STK_t *)STK_BASE_ADDRESS)






#endif /* STK_PRIVATE_H_ */
