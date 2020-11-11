#ifndef __DAFLOC_H_
#define __DAFLOC_H_

// Standard Includes
#include <xdc/std.h>
#include <dev.h>
#include <sem.h>
#include <sys.h>
#include <mem.h>
#include <que.h>

#include <afe.h>
#include <src/afeloc.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct DAF_Obj {

    MdUns         frameSize;
    Uns           usFrequency;

    Bool          bInitTx;
    Bool          bReadyTx;
    SEM_Handle    pReadyTx;
    DEV_Handle    ptDeviceTx;
    Bool          bEnabledTx;
    Uns           usAudioFormatTx;

    Bool          bInitRx;
    Bool          bReadyRx;
    SEM_Handle    pReadyRx;
    DEV_Handle    ptDeviceRx;
    Bool          bEnabledRx;
    Uns           usAudioFormatRx;

    AFE_Handle    pAfeObj;
} DAF_Obj;

typedef DAF_Obj *DAF_Handle;         // device object handle

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif // __DAFLOC_H_
