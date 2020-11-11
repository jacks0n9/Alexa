#ifndef GENCTL_PRIV_H
#define GENCTL_PRIV_H

#include <xdc/std.h>
#include <std.h>
#include <ti/xdais/ialg.h>

#include <ti/mas/genCtl/genCtl.h>

typedef struct GENCTL_Obj {
    IECO_Obj  alg;            /* MUST be first field of all XDAS algs */
} GENCTL_Obj;

Int GENCTL_numAlloc(Void);

Int GENCTL_alloc(const IALG_Params *algParams, IALG_Fxns **pf, IALG_MemRec memTab[]);

Int GENCTL_initObj(IALG_Handle, const IALG_MemRec *, IALG_Handle, const IALG_Params *params);

Int GENCTL_free(IALG_Handle, IALG_MemRec *);

XDAS_Int32 GENCTL_Control(IECO_Handle handle, IECO_Cmd messageID, IECO_DynamicParams *ctrl, IECO_Status *status);

XDAS_Int32 GENCTL_Process(IECO_Handle handle, IECO_InArgs *inArgs, IECO_OutArgs *outArgs);

#endif /* (GENCTL_PRIV_H) */
