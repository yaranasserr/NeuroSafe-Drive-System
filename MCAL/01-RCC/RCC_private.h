/*******************************************************
 *******************************************************
 *****************  Author : Osama Omar  ***************
 *****************  version : v1.0       ***************
 *****************  Date : 12/2/2023     ***************
 *******************************************************
 *******************************************************
 */


#ifndef RCC_PRIVATE_H
#define RCC_PRIVATE_H



#define RCC_BASE_ADDRESS  0x40021000

typedef struct
{
	u32 CR;
	u32 CFGR;
	u32 CIR;
	u32 APB2RSTR;
	u32 APB1RSTR;
	u32 AHBENR;
	u32 APB2ENR;
	u32 APB1ENR;
	u32 BDCR;
	u32 CSR;

}RCC_t;

#define RCC ((volatile RCC_t *)RCC_BASE_ADDRESS)


#endif
