/*
 *  ======== l16pcmdec_tii.h ========
 *  Interface for the L16PCMDEC_TII module; TII's implementation 
 *  of the IL16PCMDEC interface
 */
#ifndef L16PCMDEC_TII_
#define L16PCMDEC_TII_

#include "ti/xdais/dm/isphdec1.h"

/*
 *  ======== L16PCMDEC_TII_IALG ========
 * TII's implementation of the IALG interface for L16PCMDEC
 */
extern IALG_Fxns L16PCMDEC_TII_IALG;

/*
 *  ======== L16PCMDEC_TII_IL16PCMDEC ========
 *  TII's implementation of the IL16PCMDEC interface
 */
extern ISPHDEC1_Fxns L16PCMDEC_TII_IL16PCMDEC;

extern Int  L16PCMDEC_TII_alloc(const IALG_Params *, struct IALG_Fxns **,
                                  IALG_MemRec *);
extern Int  L16PCMDEC_TII_numAlloc(Void);
extern Int  L16PCMDEC_TII_free(IALG_Handle, IALG_MemRec *);
extern Int  L16PCMDEC_TII_initObj(IALG_Handle, const IALG_MemRec *,
                                    IALG_Handle, const IALG_Params *);
extern Void L16PCMDEC_TII_moved(IALG_Handle, const IALG_MemRec *, IALG_Handle,
                                  const IALG_Params *);
extern XDAS_Int32 L16PCMDEC_TII_decode (ISPHDEC1_Handle handle,
                                XDM1_SingleBufDesc *inBufs,
                                XDM1_SingleBufDesc *outBufs,
                                ISPHDEC1_InArgs *inArgs,
                                ISPHDEC1_OutArgs *outArgs);
extern XDAS_Int32 L16PCMDEC_TII_control (ISPHDEC1_Handle handle,
                                      ISPHDEC1_Cmd cmd,
                                      ISPHDEC1_DynamicParams *params,
                                      ISPHDEC1_Status *status);

#endif	/* L16PCMDEC_TII_ */
