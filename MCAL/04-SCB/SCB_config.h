/*******************************************************
 *******************************************************
 *****************  Author : Ali Ahmed   ***************
 *****************  version : v0         ***************
 *****************  Date : 12/7/2023     ***************
 *******************************************************
 ******************************************************/

#ifndef SCB_CONFIG_H
#define SCB_CONFIG_H

/*
 * Options for SCB interrupt priority grouping and subpriority configuration:
 * - SCB_GROUP_4_SUB_0: Group 4, Subpriority 0
 * - SCB_GROUP_3_SUB_1: Group 3, Subpriority 1
 * - SCB_GROUP_2_SUB_2: Group 2, Subpriority 2
 * - SCB_GROUP_1_SUB_3: Group 1, Subpriority 3
 * - SCB_GROUP_0_SUB_4: Group 0, Subpriority 4
 */
#define SCB_GROUP_SUB_DISTRIBUTION      SCB_GROUP_2_SUB_2  // Default configuration

#endif  
