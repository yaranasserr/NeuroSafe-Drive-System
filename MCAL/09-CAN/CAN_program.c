/*******************************************************
 *******************************************************
 *****************  Author : Osama Omar  ***************
 *****************  version : v1.0  	 ***************
 *****************  File : CAN_program    ***************
 *****************  Date : 2/4/2023     ***************
 *******************************************************
 *******************************************************
 */

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "RCC_interface.h"
#include "GPIO_interface.h"
#include "AFIO_interface.h"
#include "NVIC_interface.h"

#include "CAN_interface.h"
#include "CAN_config.h"
#include "CAN_private.h"

void CAN_voidInit()
{
	/* Enable CAN clock */
	RCC_voidPeripheralClockEnable(RCC_APB1, CAN1);
	/* Enable AFIO Clock */
	RCC_voidPeripheralClockEnable(RCC_APB2, AFIO);

	/* Enable GPIO Clock */
	RCC_voidPeripheralClockEnable(RCC_APB2, IOPA);

	MAFIO_voidRemapPeripheralPins(CAN_REMAP);

	/* CAN pins :  Rx -> PB8 & Tx -> PB9 */
	/* Rx Pin set as input floating */
	GPIO_voidSetPinMode(PORTA, PIN11, GPIO_INPUT_FLOATING);

	/* Tx Pin set as alternate function push pull */
	GPIO_voidSetPinMode(PORTA, PIN12, GPIO_AF_OUTPUT_PP_2MHZ);




	/* exit CAN sleep mode */
    CLR_BIT(CAN->MCR,1);
    /* wait to exit sleep mode */
    while (!(GET_BIT(CAN->MSR,1) == 0));
     /* Set CAN Initialization mode */
    SET_BIT(CAN->MCR,0);
    /* wait to enter in Initialization mode */
    while(!(GET_BIT(CAN->MSR,0) == 1));

    /*CHECK Transmit FIFO priority */
#if TransmitFifoPriority == ENABLE
    /* Priority driven by the request order*/
    SET_BIT(CAN -> MCR,2);
#else
    /*Priority driven by the identifier of the message*/
    CLR_BIT(CAN -> MCR,2);
#endif

#if AutoRetransmission == ENABLE
   /* A message will be transmitted only once*/
   SET_BIT(CAN -> MCR,4);
#else
   /*The CAN hardware will automatically retransmit the message*/
   CLR_BIT(CAN -> MCR,4);
#endif



    /* Set baud rate  */
   	CAN->BTR |= CAN_BAUDRATE;

   	/* Select CAN mode */
   	CAN->BTR |= CAN_MODE;

    /* Exit CAN Initialization mode */
    CLR_BIT(CAN->MCR,0);

	/* wait until CAN exit initialization mode  */
	while(!(GET_BIT(CAN->MSR, 0) == 0));

}



void CAN_voidTransmit(CAN_TxHeader_t *pTxHeader, u8 Local_u8Data[])
{
	u32 local_u8TransmitMailbox; // which mailbox is empty to set Local_u8Data in it (read CODE bit in TSR register)
	
	 /* Check that all the Tx mailboxes are not full */
	if ( (GET_BIT(CAN->TSR, 26) != 0x0) ||(GET_BIT (CAN->TSR,27) != 0x0) ||(GET_BIT(CAN->TSR,28) != 0x0) )   /* TME0 , TME1 , TME2 */
	{
		/* Select an empty transmit mailbox */
		local_u8TransmitMailbox = ((CAN->TSR >> 24) & 0x3); //(read CODE[1:0] bit in TSR register)
		 /* Check transmit mailbox value */
		if (local_u8TransmitMailbox <=2)
		{
			/* Set up the Id */
			if (pTxHeader->IDE == CAN_ID_STD)
			{
				CAN->TxMailbox[local_u8TransmitMailbox].TIR = ((pTxHeader->StdId << 21) | pTxHeader->RTR);
			}
			else
			{
				CAN->TxMailbox[local_u8TransmitMailbox].TIR = ((pTxHeader->ExtId << 3) | pTxHeader->IDE | pTxHeader->RTR);
			}

			/* Set up the DLC */
			CAN->TxMailbox[local_u8TransmitMailbox].TDTR = (pTxHeader->DLC);
			/* Set up the Transmit Global Time mode */
			if (pTxHeader->TransmitGlobalTime == ENABLE)
			{
				SET_BIT(CAN->TxMailbox[local_u8TransmitMailbox].TDTR, 8);
			}
		}
				 /* Set up the Local_u8Data field */
			CAN->TxMailbox[local_u8TransmitMailbox].TDHR =
						   ((u32)Local_u8Data[7] << 24) | ((u32)Local_u8Data[6] << 16) |((u32)Local_u8Data[5] << 8) |((u32)Local_u8Data[4]);

			CAN->TxMailbox[local_u8TransmitMailbox].TDLR =
						   ((u32)Local_u8Data[3] << 24) |((u32)Local_u8Data[2] << 16) |((u32)Local_u8Data[1] << 8) |((u32)Local_u8Data[0]);


			/* Request transmission */
			SET_BIT(CAN->TxMailbox[local_u8TransmitMailbox].TIR, 0);
	}
}


void CAN_voidReceive(CAN_RxHeader_t *pRxMsgHeader, u8 copy_FIFOIndex , u8 copy_u8Data[])
{
	/* Check IDE bit to decide the format of the identifier received */
	pRxMsgHeader->IDE = GET_BIT(CAN->RxFIFOMailbox[copy_FIFOIndex].RIR,2);

	if(pRxMsgHeader->IDE == 0)
	{
		/* Standard format identifier message */
		pRxMsgHeader->ID = CAN->RxFIFOMailbox[copy_FIFOIndex].RIR >> 21;
	}
	else
	{
		/* Extended format identifier message */
		pRxMsgHeader->ID = CAN->RxFIFOMailbox[copy_FIFOIndex].RIR >> 3;
	}

	/* Check type of frame */
	pRxMsgHeader->RTR = GET_BIT(CAN->RxFIFOMailbox[copy_FIFOIndex].RIR, 1);

	/* Length of data received */
	pRxMsgHeader->DLC = (CAN->RxFIFOMailbox[copy_FIFOIndex].RDTR) & 0xF ;

	/* Save filter match index */
	pRxMsgHeader->FilterMatchIndex = (CAN->RxFIFOMailbox[copy_FIFOIndex].RDTR) & 0xFF00 ;

	/* Save data received */
	copy_u8Data[0] = CAN->RxFIFOMailbox[copy_FIFOIndex].RLR >> 0;
	copy_u8Data[1] = CAN->RxFIFOMailbox[copy_FIFOIndex].RLR >> 8;
	copy_u8Data[2] = CAN->RxFIFOMailbox[copy_FIFOIndex].RLR >> 16;
	copy_u8Data[3] = CAN->RxFIFOMailbox[copy_FIFOIndex].RLR >> 24;

	copy_u8Data[4] = CAN->RxFIFOMailbox[copy_FIFOIndex].RHR >> 0;
	copy_u8Data[5] = CAN->RxFIFOMailbox[copy_FIFOIndex].RHR >> 8;
	copy_u8Data[6] = CAN->RxFIFOMailbox[copy_FIFOIndex].RHR >> 16;
	copy_u8Data[7] = CAN->RxFIFOMailbox[copy_FIFOIndex].RHR >> 24;

	/* Release FIFO  after data received */
	if(copy_FIFOIndex == 0)
	{
		CAN->RF0R |= 1<<5;
	}
	else
	{
		CAN->RF1R |= 1<<5;
	}
}


void CAN_voidConfigFilter(CAN_FilterHeader_t *pFilterHeader)
{
	/* Initialize filter (FINIT=1) */
	CAN->FMR |= (1<<0);

	/* Choose Filter MOde */
	CAN->FM1R |= (pFilterHeader->FilterMode << pFilterHeader->FilterBank);

	/* Choose Filter Scale */
	CAN->FS1R |= (pFilterHeader->FilterScale << pFilterHeader->FilterBank);

	/* Choose Filter FIFO Assignment */
	CAN->FFA1R |= (pFilterHeader->FilterFIFOAssign << pFilterHeader->FilterBank);

	/* Deactivate filter to edit FIR Register */
	CAN->FA1R |= (0 << pFilterHeader->FilterBank);

	if(pFilterHeader->FilterScale == 0)
	{
		CAN->FilterBankRegisters[pFilterHeader->FilterBank].FR1 = (((pFilterHeader->FilterMaskIDLow  & 0x0000FFFF) << 16)  | (pFilterHeader->FilterIDLow  & 0x0000FFFF));

		CAN->FilterBankRegisters[pFilterHeader->FilterBank].FR2 = (((pFilterHeader->FilterMaskIDHigh & 0x0000FFFF) << 16)  | (pFilterHeader->FilterIDHigh & 0x0000FFFF));
	}
	else if(pFilterHeader->FilterScale == 1)
	{
		CAN->FilterBankRegisters[pFilterHeader->FilterBank].FR1 = (((pFilterHeader->FilterIDHigh     &  0x0000FFFF) << 16) | (pFilterHeader->FilterIDLow     & 0x0000FFFF));
		CAN->FilterBankRegisters[pFilterHeader->FilterBank].FR2 = (((pFilterHeader->FilterMaskIDHigh &  0x0000FFFF) << 16) | (pFilterHeader->FilterMaskIDLow & 0x0000FFFF));
	}

	/* Activate filter  */
	CAN->FA1R |= (1 << pFilterHeader->FilterBank);

	/* De-Initialize filter (FINIT=0) */
	CLR_BIT(CAN->FMR, 0);
}

void CAN_voidInitInterrupt(IRQn_Type IRQn)
{
	NVIC_voidEnableIRQn(IRQn);
	NVIC_voidSetPending(IRQn);
}

__weak void CAN_RxFifo0MsgPendingCallback()
{

}
