/*
 *  ======== l16pcmenc_TII_ialgvt.c ========
 *  Definition of L16PCMENC_TII_IL16PCMENC, the IL16PCMENC function structure
 *  the defines TII's implementation of the IL16PCMENC interface.
 *  Since IL16PCMENC derives from IALG, we also define the symbol
 *  L16PCMENC_TII_IALG, TII's implementation of the IALG interface.
 *
 *  We place these tables in a separate file for two reasons:
 *     1. We want allow one to replace these tables with
 *    different definitions.  For example, one may
 *    want to build a system where the FIR is activated
 *    once and never deactivated, moved, or freed.
 *
 *     2. Eventually there will be a separate "system build"
 *    tool that builds these tables automatically 
 *    and if it determines that only one implementation
 *    of an API exists, "short circuits" the vtable by
 *    linking calls directly to the algorithm's functions.
 */
#include <xdc/std.h>
#include <string.h>

#include "l16pcmenc_tii.h"

/*
 *  ======== L16PCMENC_TII_IL16PCMENC ========
 *  TII's implementation of the IL16PCM interface
 */
#define IALGFXNSENC                             \
    &L16PCMENC_TII_IALG,      /* module ID  */  \
    NULL,                     /* activate   */  \
    L16PCMENC_TII_alloc,      /* algAlloc   */  \
    NULL,                     /* control    */  \
    NULL,                     /* deactivate */  \
    L16PCMENC_TII_free,       /* free       */  \
    L16PCMENC_TII_initObj,    /* init       */  \
    L16PCMENC_TII_moved,      /* moved      */  \
    L16PCMENC_TII_numAlloc    /* numAlloc   */

/*
 *  ======== L16PCMENC_TII_IL16PCMENC ========
 *  This structure defines TII's implementation of the IL16PCMENC interface
 *  for the L16PCMENC_TII module.
 */
ISPHENC1_Fxns L16PCMENC_TII_IL16PCMENC = { /* module_vendor_interface */
    IALGFXNSENC,
    L16PCMENC_TII_encode,
    L16PCMENC_TII_control
};

/*
 *  ======== L16PCMENC_TII_IALG ========
 *  This structure defines TII's implementation of the IALG interface
 *  for the L16PCMENC_TII module.
 */

#ifdef _TI_

asm("_L16PCMENC_TII_IALG .set _L16PCMENC_TII_IL16PCMENC");

#else

/*
 *  We duplicate the structure here to allow this code to be compiled and
 *  run non-DSP platforms at the expense of unnecessary data space
 *  consumed by the definition below.
 */
IALG_Fxns L16PCMENC_TII_IALG = {      /* module_vendor_interface */
    IALGFXNSENC
};

#endif

/* Nothing past this point */
