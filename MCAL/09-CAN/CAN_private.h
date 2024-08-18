/*******************************************************
 *******************************************************
 *****************  Author : Osama Omar  ***************
 *****************  version : v1.0  	 ***************
 *****************  File : CAN_private   ***************
 *****************  Date : 4/4/2023      ***************
 *******************************************************
 *******************************************************
 */

#ifndef CAN_PRIVATE_H_
#define CAN_PRIVATE_H_

#include "STD_TYPES.h"

#define CAN_BASE_ADDRESS 0x40006400


/************ CAN Tx Mailbox Registers***********/
typedef struct {

	u32 TIR;
	u32 TDTR;
	u32 TDLR;
	u32 TDHR;

}CAN_TxMailbox_t;


/************ CAN Rx FIFO Mailbox Registers***********/
typedef struct {

	u32 RIR;
	u32 RDTR;
	u32 RLR;
	u32 RHR;

}CAN_RxFIFOMailbox_t;


/************ CAN Filter Registers***********/
typedef struct {

	u32 FR1;
	u32 FR2;

}CAN_Filters_t;



/*********** CAN Registers ********************/
typedef struct{

	u32 MCR;
	u32 MSR;
	u32 TSR;
	u32 RF0R;
	u32 RF1R;
	u32 IER;
	u32 ESR;
	u32 BTR;
	u32 Resrved0[88];
	/* 3 Tx MailBoxes definition from another structure */
	CAN_TxMailbox_t TxMailbox[3];
	/* 2 Rx FIFO MailBoxes definition from another structure */
	CAN_RxFIFOMailbox_t RxFIFOMailbox[2];
	u32 Reserved1[12];
	u32 FMR;
	u32 FM1R;
	u32 Reserve2;
	u32 FS1R;
	u32 Reserve3;
	u32 FFA1R;
	u32 Reserve4;
	u32 FA1R;
	u32 Reserve5[8];
	/* 28 Filter definition from another structure */
	CAN_Filters_t FilterBankRegisters[28];

}CAN_t;


#define CAN ((volatile CAN_t *)CAN_BASE_ADDRESS)


#endif /* CAN_PRIVATE_H_ */
