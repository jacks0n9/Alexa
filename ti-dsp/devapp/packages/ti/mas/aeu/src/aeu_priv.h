#ifndef AEU_PRIV_H
#define AEU_PRIV_H

#include <ti/xdais/ialg.h>
#include <ti/mas/aeu/aeu.h>

typedef Void  *ALGO_Handle;

typedef struct AEU_Obj {
    IECO_Obj  alg;            /* MUST be first field of all XDAS algs */
    Uint32    samplingRate;
    Uint16    num_svd;   
    Uint16    num_snl;   
    Uint16    num_slm;
    Uint16    num_veu;
    AEU_SampleRate svd_samp_rate;
    ALGO_Handle  *algoHandleArray;
} AEU_Obj;

#define AEU_OBJ_BUFF   0
#define AEU_ALGO_BUFF  1
#define AEU_SVD_BUFF   2
#define AEU_SNL_BUFF   3
#define AEU_SLM_BUFF   4
#define AEU_VEU_BUFF   5
#define AEU_VEU_BUFF_1 6
#define AEU_NUM_BUFF   (AEU_VEU_BUFF_1 + 1)

Int AEU_alloc(const IALG_Params *algParams, IALG_Fxns **pf, IALG_MemRec memTab[]);

Int AEU_free(IALG_Handle, IALG_MemRec *);

Int AEU_initObj(IALG_Handle, const IALG_MemRec *, IALG_Handle, const IALG_Params *params);

Int AEU_numAlloc(Void);

XDAS_Int32 AEU_Control(IECO_Handle handle, IECO_Cmd messageID, IECO_DynamicParams *ctrl, IECO_Status *status);

XDAS_Int32 AEU_Process(IECO_Handle handle, IECO_InArgs *inArgs, IECO_OutArgs *outArgs);
                
#endif /* (AEU_PRIV_H) */
