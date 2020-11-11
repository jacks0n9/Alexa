/* 
 *  Copyright 2007
 *  Texas Instruments Incorporated
 *
 *  All rights reserved.  Property of Texas Instruments Incorporated
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 * 
 */
/*
 *  ======== _devnode.h ========
 */
#ifndef ti_mas__DEVNODE_H_
#define ti_mas__DEVNODE_H_

#include <ti/sdo/ce/visa.h>
#include <devnode.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _DEVNODE_CPROCESS        0
#define _DEVNODE_CCONTROL        1

/* msgq message to decode */
typedef struct {
    VISA_MsgHeader  visa;
    union {
        struct {
            DEVNODE_InArgs        inArgs;
            DEVNODE_OutArgs       outArgs;
        } process;
        struct {
            DEVNODE_Cmd           cmd;
            DEVNODE_Ctrl_InArgs   inArgs;
            DEVNODE_Ctrl_OutArgs  outArgs;
        } control;
    } cmd;
} _DEVNODE_Msg;

#ifdef __cplusplus
}
#endif

#endif

