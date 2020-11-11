/*
 *  ======== l16pcmenc_tii.h ========
 *  Interface for the L16PCMENC_TII module;TII's implementation 
 *  of the IL16PCMENC interface
 */
#ifndef L16PCMENC_TII_
#define L16PCMENC_TII_

#include "ti/xdais/dm/isphenc1.h"
#include <ti/xdais/dm/ispeech1_pcm.h>

/*
 *  ======== L16PCMENC_TII_IALG ========
 *  TII's implementation of the IALG interface for L16PCMENC
 */
extern IALG_Fxns L16PCMENC_TII_IALG;

/*
 *  ======== L16PCMENC_TII_IL16PCMENC ========
 *  TII's implementation of the IL16PCMENC interface
 */
extern ISPHENC1_Fxns L16PCMENC_TII_IL16PCMENC;

extern Int  L16PCMENC_TII_alloc(const IALG_Params *, struct IALG_Fxns **,
                                  IALG_MemRec *);
extern Int  L16PCMENC_TII_numAlloc(Void);
extern Int  L16PCMENC_TII_free(IALG_Handle, IALG_MemRec *);
extern Int  L16PCMENC_TII_initObj(IALG_Handle, const IALG_MemRec *,
                                    IALG_Handle, const IALG_Params *);
extern Void L16PCMENC_TII_moved(IALG_Handle, const IALG_MemRec *, IALG_Handle,
                                  const IALG_Params *);
extern XDAS_Int32 L16PCMENC_TII_encode (ISPHENC1_Handle handle,
                                        XDM1_SingleBufDesc *inBufs,
                                        XDM1_SingleBufDesc *outBufs,
                                        ISPHENC1_InArgs *inArgs,
                                        ISPHENC1_OutArgs *outArgs);
extern XDAS_Int32 L16PCMENC_TII_control(ISPHENC1_Handle handle,
                                      ISPHENC1_Cmd cmd,
                                      ISPHENC1_DynamicParams *params,
                                      ISPHENC1_Status *status);

#endif	/* L16PCMENC_TII_ */
