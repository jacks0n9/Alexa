/* =============================================================*
 *  File : l16pcmdec_tii_ialg.c                                 *
 *                                                              *
 *  Implementation of the IALG_Fxns for the L16PCM_TII module.  *
 *                                                              *
 * =============================================================*
 */
#ifdef CCODESECT
#pragma CODE_SECTION(L16PCMDEC_TII_alloc,   ".text:L16PCMDecAlgAlloc")
#pragma CODE_SECTION(L16PCMDEC_TII_numAlloc,".text:L16PCMDecAlgNumAlloc")
#pragma CODE_SECTION(L16PCMDEC_TII_free,    ".text:L16PCMDecAlgFree")
#pragma CODE_SECTION(L16PCMDEC_TII_initObj, ".text:L16PCMDecAlgInit")
#pragma CODE_SECTION(L16PCMDEC_TII_moved,   ".text:L16PCMDecAlgMoved")
#pragma CODE_SECTION(L16PCMDEC_TII_decode,  ".text:L16PCMDecode")
#pragma CODE_SECTION(L16PCMDEC_TII_control, ".text:L16PCMDecControl")
#endif

#include "xdc/std.h"
#include <string.h>
#include "l16pcmdec_tii.h"
#include "l16pcmdec_tii_priv.h"

/*
 *  ======== L16PCMDEC_TII_alloc ========
 *  Return a table of memory descriptors that describe the memory needed
 *  to construct a L16PCMDEC_TII_Obj structure.
 */
Int L16PCMDEC_TII_alloc(const IALG_Params *l16pcmdecParams, IALG_Fxns **fxns,
                          IALG_MemRec memTab[])
{
  /* Request memory for L16PCMDEC object */
  memTab[0].size      = sizeof(L16PCMDEC_TII_Obj);
  memTab[0].alignment = 8;
  memTab[0].space     = IALG_DARAM0;
  memTab[0].attrs     = IALG_PERSIST;

  return (NBUFS_DEC);
}

/*
 *  ======== L16PCMDEC_TII_numAlloc ========
 *  Returns the number of buffers needed.
 */
Int L16PCMDEC_TII_numAlloc()
{
  return (NBUFS_DEC);
}

/*
 *  ======== L16PCMDEC_TII_free ========
 *  Return a table of memory pointers that should be freed.  Note
 *  that this should include *all* memory requested in the
 *  L16PCMDEC_TII_alloc operation above.
 */
Int L16PCMDEC_TII_free(IALG_Handle handle, IALG_MemRec memTab[])
{
  Int n;

  n = L16PCMDEC_TII_alloc(NULL, NULL, memTab);

  memTab[0].size = sizeof(L16PCMDEC_TII_Obj);
  memTab[0].base = (Void *) handle;

  return (n);
}

/*  ======== L16PCMDEC_TII_initObj ========
 *  Initialize the memory allocated for our instance.
 */
Int L16PCMDEC_TII_initObj(IALG_Handle handle, const IALG_MemRec memTab[],
                          IALG_Handle p, const IALG_Params *l16pcmdecParams)
{
  return (ISPHDEC1_EOK);
}

/*  ======== L16PCMDEC_TII_moved ========
 *  Fixeup any pointers to data that has been moved by the client.
 */
Void L16PCMDEC_TII_moved(IALG_Handle handle, const IALG_MemRec memTab[],
                          IALG_Handle p, const IALG_Params *l16pcmdecParams)
{
}

/*
 *  ======== L16PCMDEC_TII_L16PCMDecode ========
 *  TII's implementation of the L16PCMDecode operation.
 */
XDAS_Int32 L16PCMDEC_TII_decode (ISPHDEC1_Handle handle,
                                XDM1_SingleBufDesc *inBufs,
                                XDM1_SingleBufDesc *outBufs,
                                ISPHDEC1_InArgs *inArgs,
                                ISPHDEC1_OutArgs *outArgs)
{
  //L16PCMDEC_TII_Obj *l16pcmdec = (L16PCMDEC_TII_Obj *)handle;
  XDAS_Int16      temp_in, temp_out, i;
  XDAS_Int16      frame_size = inBufs->bufSize;
  XDAS_Int16      *tmpOutPtr = (XDAS_Int16 *)(outBufs->buf);    

  /* report _how_ we accessed the 2 data buffers */
  XDM_SETACCESSMODE_READ(inBufs->accessMask);
  XDM_SETACCESSMODE_WRITE(outBufs->accessMask);

  if(inArgs->frameType == ISPHDEC1_FTYPE_SPEECHGOOD)
  {
    for(i = 0; i < frame_size/2; i++)
    {
        temp_in = ((XDAS_Int16 *)(inBufs->buf))[i];
        temp_out = temp_in;
        tmpOutPtr[i] = temp_out;
    }  
  }
  else
  {
    memset((XDAS_Int8 *)outBufs->buf, 0, frame_size);
  }

  outBufs->bufSize = frame_size;

  return(ISPHDEC1_EOK);
}

/*
 *  ======== L16PCMDEC_TII_L16PCMControl ========
 *  TII's implementation of the L16PCMDecoder Control.
 */
XDAS_Int32 L16PCMDEC_TII_control (ISPHDEC1_Handle handle,
                                ISPHDEC1_Cmd cmd,
                                ISPHDEC1_DynamicParams *params,
                                ISPHDEC1_Status *status)
{
  return (ISPHDEC1_EOK);
}

/* Nothing past this point */
