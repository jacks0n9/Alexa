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
 *  ======== eco.c ========
 *  Scaling API, exposed to the application.
 *
 *  The methods defined here must be independent of whether the underlying
 *  algorithm is executed locally or remotely.
 *
 *  In fact, these methods must exist on *both* the client and server; the
 *  ECO skeletons (eco_skel.c) call these API's to create instances on the
 *  remote CPU.
 */
#include <xdc/std.h>

#include <ti/xdais/ialg.h>
#include <ti/sdo/ce/visa.h>

#include <ti/sdo/ce/osal/Log.h>
#include <ti/sdo/ce/trace/gt.h>

#include "eco.h"
#include "_eco.h"

/* default params*/
ECO_Params ECO_PARAMS = {
   sizeof(ECO_Params)
};

#define GTNAME "ti.mas.eco.ECO"

static GT_Mask curTrace = {NULL,NULL};


/*
 *  ======== ECO_create ========
 */
ECO_Handle ECO_create(Engine_Handle engine, String name,
    ECO_Params *params)
{
    ECO_Handle visa;
    static Bool curInit = FALSE;

    if (curInit != TRUE) {
        curInit = TRUE;
        GT_create(&curTrace, GTNAME);
    }

    GT_3trace(curTrace, GT_ENTER, "ECO_create> "
        "Enter (engine=0x%x, name='%s', params=0x%x)\n",
        engine, name, params);

    if (params == NULL) {
        params = &ECO_PARAMS;
    }

    visa = VISA_create(engine, name, (IALG_Params *)params,
        sizeof (_ECO_Msg), "ti.mas.eco.IECO");

    GT_1trace(curTrace, GT_ENTER, "ECO_create> return (0x%x)\n", visa);

    return (visa);
}


/*
 *  ======== ECO_delete ========
 */
Void ECO_delete(ECO_Handle handle)
{
    GT_1trace(curTrace, GT_ENTER, "ECO_delete> Enter (handle=0x%x)\n",
        handle);

    VISA_delete(handle);

    GT_0trace(curTrace, GT_ENTER, "ECO_delete> return\n");
}


/*
 *  ======== ECO_process ========
 *  This method must be the same for both local and remote invocation;
 *  each call site in the client might be calling different implementations
 *  (one that marshalls & sends and one that simply calls).
 */
XDAS_Int32 ECO_process(ECO_Handle handle, ECO_InArgs *inArgs, ECO_OutArgs *outArgs)
{
    XDAS_Int32 retVal = ECO_EFAIL;

    GT_3trace(curTrace, GT_ENTER, "ECO_process> "
        "Enter (handle=0x%x, inArgs=0x%x, "
        "outArgs=0x%x)\n", handle, inArgs, outArgs);

    if (handle) {
        IECO_Fxns *fxns =
            (IECO_Fxns *)VISA_getAlgFxns((VISA_Handle)handle);
        IECO_Handle alg = VISA_getAlgHandle((VISA_Handle)handle);

        if (fxns && (alg != NULL)) {
            /*
             * To better enable data visualization tooling, drop a
             * well-formed breadcrumb describing what we're doing.
             */
            Log_printf(ti_sdo_ce_dvtLog, "%s", (Arg)"ECO:process",
                (Arg)handle, (Arg)0);
            VISA_enter((VISA_Handle)handle);
            retVal = fxns->process(alg, inArgs, outArgs);
            VISA_exit((VISA_Handle)handle);
        }
    }

    GT_2trace(curTrace, GT_ENTER, "ECO_process> "
        "Exit (handle=0x%x, retVal=0x%x)\n", handle, retVal);

    return (retVal);
}

/*
 *  ======== ECO_control ========
 *  This method must be the same for both local and remote invocation;
 *  each call site in the client might be calling different implementations
 *  (one that marshalls & sends and one that simply calls).  This API
 *  abstracts *all* image decoders (both high and low complexity
 *  decoders are envoked using this method).
 */
XDAS_Int32 ECO_control(ECO_Handle handle, ECO_Cmd cmd, ECO_DynamicParams *params, ECO_Status *status)
{
    XDAS_Int32 retVal = ECO_EFAIL;

    GT_3trace(curTrace, GT_ENTER, "ECO_control> "
        "Enter (handle=0x%x, params=%d, status=0x%x\n",
        handle, params, status);

    if (handle) {
        IECO_Fxns *fxns =
            (IECO_Fxns *)VISA_getAlgFxns((VISA_Handle)handle);
        IECO_Handle alg = VISA_getAlgHandle((VISA_Handle)handle);

        if (fxns && (alg != NULL)) {
            /*
             * To better enable data visualization tooling, drop a
             * well-formed breadcrumb describing what we're doing.
             */
            Log_printf(ti_sdo_ce_dvtLog, "%s", (Arg)"ECO:control",
                (Arg)handle, (Arg)0);
            VISA_enter((VISA_Handle)handle);
            retVal = fxns->control(alg, cmd, params, status);
            VISA_exit((VISA_Handle)handle);
        }
    }

    GT_2trace(curTrace, GT_ENTER, "ECO_control> "
        "Exit (handle=0x%x, retVal=0x%x)\n", handle, retVal);

    return (retVal);
}

typedef struct VISA_FakeObj {
    IALG_Fxns       *fxns;          /* this must be first */
    Ptr             algHandle;      /* local ALG_Handle or VISA_Handle */
    Engine_Node     node;           /* remote node */
} VISA_FakeObj;

ECO_Handle ECO_getEcoHandle(ECO_Handle handle)
{
  return((ECO_Handle)Engine_getRemoteVisa( ((VISA_FakeObj *)handle)->node));
}
