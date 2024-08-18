/*******************************************************
 *******************************************************
 *****************  Author : Osama Omar  ***************
 *****************  version : v1.0  	 ***************
 *****************  File : CAN_interface ***************
 *****************  Date : 2/4/2023      ***************
 *******************************************************
 *******************************************************
 */

#ifndef CAN_INTERFACE_H_
#define CAN_INTERFACE_H_


#include "STD_TYPES.h"

#define DISABLE			 0
#define ENABLE 			 1

/* CAN_identifier_type CAN Identifier Type */
#define CAN_ID_STD                  ((u32)(0x0))  
#define CAN_ID_EXT                  ((u32)(0x4))  

/* CAN_remote_transmission_request CAN Remote Transmission Request */
#define CAN_RTR_DATA                ((u32)(0x0))
#define CAN_RTR_REMOTE              ((u32)(0x2))


/* CAN baud rates */
#define CAN_100KBPS  0x001c0013
#define CAN_200KBPS  0x001c0009
//#define CAN_300KBPS  0x001c0004
#define CAN_400KBPS  0x001c0004
#define CAN_500KBPS  0x001c0003
#define CAN_800KBPS  0x00070003
#define CAN_1MBPS    0x001c0001

/* CAN modes */
#define CAN_NORMAL_MODE      		0x00000000
#define CAN_SILENT_MODE      		0x80000000
#define CAN_LOOPBACK_MODE    		0x40000000
#define CAN_SILENT_LOOPBACK_MODE    0xC0000000

/* Filtet Defines */
#define CAN_FILTER_ACTIVE              		   1
#define CAN_FILTER_NOT_ACTIVE          		   0
#define CAN_FILTER_SCALE_DUAL_16BIT    		   0
#define CAN_FILTER_SCALE_SINGLE_32BIT  		   1
#define CAN_FILTER_MODE_MASK           		   0
#define CAN_FILTER_MODE_LIST           		   1
#define CAN_FILTER_FIFO_ASSIGN_0               0
#define CAN_FILTER_FIFO_ASSIGN_1               1
#define CAN_FILTER_BANK_0            		   0
#define CAN_FILTER_BANK_1            		   1
#define CAN_FILTER_BANK_2            		   2
#define CAN_FILTER_BANK_3            		   3
#define CAN_FILTER_BANK_4            		   4
#define CAN_FILTER_BANK_5            		   5
#define CAN_FILTER_BANK_6            		   6
#define CAN_FILTER_BANK_7            		   7
#define CAN_FILTER_BANK_8            		   8
#define CAN_FILTER_BANK_9            		   9
#define CAN_FILTER_BANK_10           		   10
#define CAN_FILTER_BANK_11           		   11
#define CAN_FILTER_BANK_12           		   12
#define CAN_FILTER_BANK_13           		   13





/*CAN Tx message header structure definition */
typedef struct
{
	u32 StdId;    /*the standard identifier */

	u32 ExtId;    /*the extended identifier. */

	u32 IDE;      /* the type of identifier for the message that will be transmitted. */

	u32 RTR;      /*the type of frame for the message that will be transmitted. */

	u32 DLC;      /*the length of the frame that will be transmitted. */

	u8 TransmitGlobalTime; 	/* the time stamp counter value captured on start of frame transmission*/

} CAN_TxHeader_t;


typedef struct
{
	u32 ID;    /*the standard identifier */

    u8 IDE;      /* the type of identifier for the message that will be transmitted. */

    u8 RTR;      /*the type of frame for the message that will be transmitted. */

    u8 DLC;      /*the length of the frame that will be transmitted. */

    u8 FilterMatchIndex;

}CAN_RxHeader_t;

typedef struct
{
	u8  FilterActivation;

	u8  FilterBank;

	u8  FilterMode;

	u8  FilterScale;

	u8  FilterFIFOAssign;

	u32 FilterIDHigh;

	u32 FilterMaskIDHigh;

	u32 FilterIDLow;

	u32 FilterMaskIDLow;



}CAN_FilterHeader_t;

void CAN_voidInit();

void CAN_voidTransmit(CAN_TxHeader_t *pTxHeader, u8 Local_u8Data[]);

void CAN_voidReceive(CAN_RxHeader_t *pRxMsgHeader, u8 copy_FIFOIndex, u8 Local_u8Data[]);

void CAN_voidConfigFilter(CAN_FilterHeader_t *pFilterHeader);

void CAN_voidInitInterrupt(IRQn_Type IRQn);
void CAN_RxFifo0MsgPendingCallback();

#endif /* CAN_INTERFACE_H_ */
