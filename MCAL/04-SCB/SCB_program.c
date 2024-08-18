/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/7/2023     ***************
 *******************************************************
 ******************************************************/

#include "STD_TYPES.h"       // Include standard data types header
#include "BIT_MATH.h"        // Include bit manipulation macros header
#include "SCB_interface.h"   // Include SCB interface header
#include "SCB_private.h"     // Include SCB private header
#include "SCB_config.h"      // Include SCB configuration header

void MSCB_voidSetPriorityGrouping(void)
{
    SCB_AIRCR = SCB_GROUP_SCB_DISTRIBUTION;
    // Set the priority grouping configuration for the System Control Block (SCB) AIRCR register
}
