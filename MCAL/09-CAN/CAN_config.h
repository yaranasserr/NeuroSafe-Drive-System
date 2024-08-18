/*******************************************************
 *******************************************************
 *****************  Author : Osama Omar  ***************
 *****************  version : v1.0  	 ***************
 *****************  File : CAN_config    ***************
 *****************  Date : 2/4/2023     ***************
 *******************************************************
 *******************************************************
 */

#ifndef CAN_CONFIG_H_
#define CAN_CONFIG_H_

 /* This parameters can be set to ENABLE or DISABLE. */
#define AutoRetransmission           ENABLE
#define TransmitFifoPriority         DISABLE


/* Options :

   CAN_100KBPS
   CAN_200KBPS
   CAN_400KBPS
   CAN_500KBPS
   CAN_800KBPS
   CAN_1MBPS
 */
#define CAN_BAUDRATE  CAN_100KBPS


/*
   Options :

   CAN_NORMAL_MODE
   CAN_SILENT_MODE
   CAN_LOOPBACK_MODE
   CAN_SILENT_LOOPBACK_MODE
 */
#define CAN_MODE      CAN_LOOPBACK_MODE

#endif /* CAN_CONFIG_H_ */
