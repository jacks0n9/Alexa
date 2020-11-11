#ifndef MISC_PRIV_H
#define MISC_PRIV_H

#include <xdc/std.h>
#include <std.h>
#include <ti/xdais/ialg.h>
#include <tsk.h>
#include <ti/mas/misc/misc.h>

typedef struct MISC_Obj {
    IECO_Obj  alg;            /* MUST be first field of all XDAS algs */
    TSK_Handle wdtTskHdl;
    WDT_Status wdtStatus;
    Uint32 wdt_kicks;
} MISC_Obj;

Int MISC_numAlloc(Void);

Int MISC_alloc(const IALG_Params *algParams, IALG_Fxns **pf, IALG_MemRec memTab[]);

Int MISC_initObj(IALG_Handle, const IALG_MemRec *, IALG_Handle, const IALG_Params *params);

Int MISC_free(IALG_Handle, IALG_MemRec *);

XDAS_Int32 MISC_Control(IECO_Handle handle, IECO_Cmd messageID, IECO_DynamicParams *ctrl, IECO_Status *status);

XDAS_Int32 MISC_Process(IECO_Handle handle, IECO_InArgs *inArgs, IECO_OutArgs *outArgs);

void WDT_kick(void);
void WDT_change(Uint32 value);
void WDT_control(Uint32 enable);
void WDT_prescale(Uint32 value);


#endif /* (MISC_PRIV_H) */
