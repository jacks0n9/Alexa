/* =============================================================*
 *  File : l16pcmenc_tii_ialg.c                                 *
 *                                                              *
 *  Implementation of the IALG_Fxns for the L16PCM_TII module.  *
 *                                                              *
 * =============================================================*
 */
#ifdef CCODESECT
#pragma CODE_SECTION(L16PCMENC_TII_alloc,   ".text:L16PCMEncAlgAlloc")
#pragma CODE_SECTION(L16PCMENC_TII_numAlloc,".text:L16PCMEncAlgNumAlloc")
#pragma CODE_SECTION(L16PCMENC_TII_free,    ".text:L16PCMEncAlgFree")
#pragma CODE_SECTION(L16PCMENC_TII_initObj, ".text:L16PCMEncAlgInit")
#pragma CODE_SECTION(L16PCMENC_TII_moved,   ".text:L16PCMEncAlgMoved")
#pragma CODE_SECTION(L16PCMENC_TII_encode,  ".text:L16PCMEncode")
#pragma CODE_SECTION(L16PCMENC_TII_control, ".text:L16PCMEncControl")
#endif

#include <xdc/std.h>
#include <string.h>

#include <ti/xdais/dm/isphenc1.h>
#include <ti/xdais/dm/ispeech1_pcm.h>

#include "l16pcmenc_tii.h"
#include "l16pcmenc_tii_priv.h"

/*
 *  ======== L16PCMENC_TII_alloc ========
 *  Return a table of memory descriptors that describe the memory needed
 *  to construct a L16PCMENC_TII_Obj structure.
 */
Int L16PCMENC_TII_alloc(const IALG_Params *l16pcmencParams, IALG_Fxns **fxns,
                          IALG_MemRec memTab[])
{
  /* Request memory for L16PCMENC object */
  memTab[0].size      = sizeof(L16PCMENC_TII_Obj);
  memTab[0].alignment = 8;
  memTab[0].space     = IALG_DARAM0;
  memTab[0].attrs     = IALG_PERSIST;

  return (NBUFS_ENC);
}

/*
 *  ======== L16PCMENC_TII_numAlloc ========
 *  Returns the number of buffers needed.
 */
Int L16PCMENC_TII_numAlloc()
{
  return (NBUFS_ENC);
}

/*
 *  ======== L16PCMENC_TII_free ========
 *  Return a table of memory pointers that should be freed.  Note
 *  that this should include *all* memory requested in the
 *  L16PCMENC_TII_alloc operation above.
 */
Int L16PCMENC_TII_free(IALG_Handle handle, IALG_MemRec memTab[])
{
  Int n;

  n = L16PCMENC_TII_alloc(NULL, NULL, memTab);

  memTab[0].size = sizeof(L16PCMENC_TII_Obj);
  memTab[0].base = (Void *) handle;

  return (n);
}

/*  ======== L16PCMENC_TII_initObj ========
 *  Initialize the memory allocated for our instance.
 */
Int L16PCMENC_TII_initObj(IALG_Handle handle, const IALG_MemRec memTab[],
                          IALG_Handle p, const IALG_Params *l16pcmencParams)
{
  return (ISPHENC1_EOK);
}

/*  ======== L16PCMENC_TII_moved ========
 *  Fixeup any pointers to data that has been moved by the client.
 */
Void L16PCMENC_TII_moved(IALG_Handle handle, const IALG_MemRec memTab[],
                          IALG_Handle p, const IALG_Params *l16pcmencParams)
{
}

/*
 *  ======== L16PCMENC_TII_L16PCMEncoder ========
 *  TII's implementation of the L16PCMEncoder operation.
 */
XDAS_Int32 L16PCMENC_TII_encode (ISPHENC1_Handle handle,
                                XDM1_SingleBufDesc *inBufs,
                                XDM1_SingleBufDesc *outBufs,
                                ISPHENC1_InArgs *inArgs,
                                ISPHENC1_OutArgs *outArgs)
{
  XDAS_Int16      temp_in, temp_out, i;
  XDAS_Int16      frame_size = inBufs->bufSize; /* frame size in words (bytes) */ 
  XDAS_Int16      *tmpOutPtr = (XDAS_Int16 *)(outBufs->buf);    

  for(i = 0; i < frame_size/2; i++)
  {
      temp_in = ((XDAS_Int16 *)(inBufs->buf))[i];
      temp_out = temp_in;
      tmpOutPtr[i] = temp_out;
  }

  /* report _how_ we accessed the 2 data buffers */
  XDM_CLEARACCESSMODE_WRITE(inBufs->accessMask);
  XDM_SETACCESSMODE_READ(inBufs->accessMask);

  XDM_CLEARACCESSMODE_READ(outBufs->accessMask);
  XDM_SETACCESSMODE_WRITE(outBufs->accessMask);

  outBufs->bufSize  = frame_size;
  outArgs->frameType      = (XDAS_Int16) ISPHENC1_FTYPE_SPEECH;

  return (ISPHENC1_EOK);
}

/*
 *  ======== L16PCMENC_TII_L16PCMControl ========
 *  TII's implementation of the L16PCMEncoder Control.
 */
XDAS_Int32 L16PCMENC_TII_control (ISPHENC1_Handle handle,
                                ISPHENC1_Cmd cmd,
                                ISPHENC1_DynamicParams *params,
                                ISPHENC1_Status *status)
{
  return (ISPHENC1_EOK);
}

/* Nothing past this point */
