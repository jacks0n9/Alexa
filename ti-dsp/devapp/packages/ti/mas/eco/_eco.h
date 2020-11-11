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
 *  ======== _eco.h ========
 */
#ifndef ti_mas__ECO_H_
#define ti_mas__ECO_H_

#include <ti/sdo/ce/visa.h>
#include <eco.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _ECO_CPROCESS        0
#define _ECO_CCONTROL        1

/* msgq message to decode */
typedef struct {
    VISA_MsgHeader  visa;
    union {
        struct {
            ECO_InArgs        inArgs;
            ECO_OutArgs       outArgs;
        } process;
        struct {
            ECO_Cmd             cmd;
            ECO_DynamicParams   params;
            ECO_Status          status;
        } control;
    } cmd;
} _ECO_Msg;

#ifdef __cplusplus
}
#endif

#endif

