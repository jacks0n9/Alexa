/*
 *  ======== l16pcmdec_tii_ialgvt.c ========
 *  Definition of L16PCMDEC_TII_IL16PCMDEC, the IL16PCMDEC function structure
 *  the defines TII's implementation of the IL16PCMDEC interface.
 *  Since IL16PCMDEC derives from IALG, we also define the symbol
 *  L16PCMDEC_TII_IALG, TII's implementation of the IALG interface.
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
#include "xdc/std.h"

#include "l16pcmdec_tii.h"

#define IALGFXNSDEC                           \
    &L16PCMDEC_TII_IALG,      /* module ID */   \
    NULL,                     /* activate */    \
    L16PCMDEC_TII_alloc,      /* algAlloc */    \
    NULL,                     /* control */     \
    NULL,                     /* deactivate */  \
    L16PCMDEC_TII_free,       /* free */        \
    L16PCMDEC_TII_initObj,    /* init */        \
    L16PCMDEC_TII_moved,      /* moved */       \
    L16PCMDEC_TII_numAlloc    /* numAlloc */

/*
 *  ======== L16PCMDEC_TII_IL16PCMDEC ========
 *  This structure defines TII's implementation of the IL16PCMDEC interface
 *  for the L16PCMDEC_TII module.
 */
ISPHDEC1_Fxns L16PCMDEC_TII_IL16PCMDEC = { /* module_vendor_interface */
    IALGFXNSDEC,
    L16PCMDEC_TII_decode,
    L16PCMDEC_TII_control
};

/*
 *  ======== L16PCMDEC_TII_IALG ========
 *  This structure defines TII's implementation of the IALG interface
 *  for the L16PCMDEC_TII module.
 */

#ifdef _TI_

asm("_L16PCMDEC_TII_IALG .set _L16PCMDEC_TII_IL16PCMDEC");

#else

/*
 *  We duplicate the structure here to allow this code to be compiled and
 *  run non-DSP platforms at the expense of unnecessary data space
 *  consumed by the definition below.
 */
IALG_Fxns L16PCMDEC_TII_IALG = {      /* module_vendor_interface */
    IALGFXNSDEC
};

#endif

/* Nothing past this point */
