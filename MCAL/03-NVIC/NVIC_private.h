/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/6/2023     ***************
 *******************************************************
 ******************************************************/


#ifndef NVIC_PRIVATE_H
#define NVIC_PRIVATE_H

#define NVIC_BASE_ADDRESS     0xE000E100

typedef struct 
{
	u32 ISER[3];
	u32 Reversed[29];
	u32 ICER[3];
	u32 Reversed1[29];
	u32 ISPR[3];
	u32 Reversed2[29];
	u32 ICPR[3];
	u32 Reversed3[29];
	u32 IABR[3];
	u32 Reversed4[61];
	u8  IPR[84];
	u32 Reversed5[655];
	u32 STIR;
	
}NVIC_t;

#define NVIC     ((volatile NVIC_t*)NVIC_BASE_ADDRESS)



#endif 