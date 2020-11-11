/* 
 *  Copyright 2007
 *  Texas Instruments Incorporated
 *
 *  All rights reserved.  Property of Texas Instruments Incorporated
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 * 
 */
/*
 *  ======== devnode.c ========
 *  Scaling API, exposed to the application.
 *
 *  The methods defined here must be independent of whether the underlying
 *  algorithm is executed locally or remotely.
 *
 *  In fact, these methods must exist on *both* the client and server; the
 *  DEVNODE skeletons (devnode_skel.c) call these API's to create instances on the
 *  remote CPU.
 */
//#define DEVNODE_INTERNAL_DBG 1

#include <xdc/std.h>

#include <ti/xdais/ialg.h>
#include <ti/sdo/ce/visa.h>

#include <ti/sdo/ce/osal/Log.h>
#include <ti/sdo/ce/trace/gt.h>

#include "devnode.h"
#include "_devnode.h"

#ifdef DEVNODE_INTERNAL_DBG
#ifdef WIN32
#include <assert.h>
#include <windows.h>
#include <IoctlCfg.h>
#include <pkfuncs_oal.h>
#elif defined(LINUX)
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif  /* WIN32 */
#endif /*#ifdef DEVNODE_INTERNAL_DBG */
#include <stdlib.h>


/* default params*/
DEVNODE_Params DEVNODE_PARAMS = {
   sizeof(DEVNODE_Params)
};

#define GTNAME "ti.mas.devnode.DEVNODE"

GT_Mask curTrace = {NULL,NULL};

#ifdef DEVNODE_INTERNAL_DBG
#define MAX_DEV_DBG_ENTRY     500
#define MAX_DEV_PROBE_POINTS  20
Uint32 devnodeDbgTimeStamp[MAX_DEV_DBG_ENTRY][MAX_DEV_PROBE_POINTS];
Uint32 devnodeDbgTimeStampIdx;

static int              memfd;
Timer_Obj   *timer_obj = NULL;

#ifdef WIN32
static MEMREQ mReq;
#elif defined(LINUX)
static void *mmapAddr = NULL;
#endif  /* WIN32 */


int devnode_SetupTimer(void)
{
    Uint32 addr, offset, size;
#ifdef WIN32
    PMEMREQ pmReq = &mReq;
    DWORD dwBytes;
    Bool bIoctrlStatus;
#endif  /* WIN32 */

    if(timer_obj)
        return 1;        /* timer has already been created */

    addr = (Uint32)Timer_phys0;
    offset = addr & (0xFFF);     // compute offset within page
    addr = addr & ~(0xFFF);      // round down to aligned page base 
    size = 0x1000;               // round up to page size	

#ifdef WIN32
    pmReq->phyAddr = addr;
    pmReq->phySize = size;

    bIoctrlStatus = KernelIoControl ( IOCTL_HAL_MEMORY_ALLOC, 
                                                     (LPVOID)pmReq, 
                                                     sizeof(MEMREQ), 
                                                     &(pmReq->vPtr), 
                                                     sizeof(pmReq->vPtr), 
                                                     &dwBytes);
    if(bIoctrlStatus)
    {
        timer_obj = (Timer_Obj *)((Uint32)pmReq->vPtr+offset);  
        return 0;
    }
    else
    {
        return 1;
    }
 
#elif defined(LINUX)
    memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if(memfd == -1)
    {
        return 1;
    }

    mmapAddr = mmap(0,                // Preferred start address
        size,                       // Length to be mapped
        PROT_READ | PROT_WRITE,     // really only need read access
        MAP_SHARED,                 // Shared memory
        memfd,                      // File descriptor
        addr);                      // The byte offset from fd
    if(mmapAddr == MAP_FAILED)
    {
        timer_obj = NULL;
        return 1;
    }

    timer_obj = (Timer_Obj *)((Uint32)mmapAddr + offset);
    return 0;
#else
    return 0;
#endif /* WIN32 */
}

int devnode_ReadTimer(Uint32 *pTimeRead)
{

    if(!pTimeRead)
    {
        return 1;
    }

    if(!timer_obj)
    {
        *pTimeRead = 0;
        return 1;
    }
    
    *pTimeRead = timer_obj->tim34;
    return 0;
}

Uint32 devnode_TimerToMicroSec(Uint32 timeCount)
{
    return (timeCount/TIMER_FREQ);
}

int devnode_FreeTimer(void)
{
#ifdef WIN32
    PMEMREQ pmReq = &mReq;
    DWORD dwBytes;
    Bool bIoctrlStatus;

    if(!timer_obj)
        return 1;
    
    bIoctrlStatus = KernelIoControl (IOCTL_HAL_MEMORY_FREE,
                                                    pmReq, 
                                                    sizeof(MEMREQ), 
                                                    NULL, 
                                                    0, 
                                                    &dwBytes);
    if(bIoctrlStatus)
        return 0;
    else
        return 1;

#elif defined(LINUX)

    Int bIoctrlStatus;

    if(!timer_obj)
        return 1;
    
    bIoctrlStatus = munmap(mmapAddr, 4096);
    close(memfd);
    if(bIoctrlStatus == 0)
        return 0;
    else
        return 1;
#endif /* WIN32 */
}

void devnode_PrintTimestamps(void)
{
        Uint32 delta_usec;
        Uint16 ii,jj;
        Uint32 tmin[MAX_DEV_PROBE_POINTS],tmax[MAX_DEV_PROBE_POINTS],sum[MAX_DEV_PROBE_POINTS];
        printf("\n ************* Devnode time cosuming break down **********************\n");
        for(jj=0;jj<8;jj++)
        {
            tmin[jj]=1000;
            tmax[jj]=0;
            sum[jj]=0;
            for(ii=0;ii<MAX_DEV_DBG_ENTRY;ii++)
            {
                delta_usec = (devnodeDbgTimeStamp[ii][jj+1] - devnodeDbgTimeStamp[ii][jj])/TIMER_FREQ;
                tmin[jj]=(delta_usec<tmin[jj]?delta_usec:tmin[jj]);
                tmax[jj]=(delta_usec>tmax[jj]?delta_usec:tmax[jj]);
                sum[jj] += delta_usec;
            }
        }
        for(ii=0;ii<8;ii++)
        {
                printf("delta[%d->%d]: min= %5d, max=%5d, average=%5d \n",
                          ii, ii+1, tmin[ii],tmax[ii],sum[ii]/MAX_DEV_DBG_ENTRY);
        }

        /* for single Memory_getBufferPhysicalAddress call */
        for(jj=10;jj<13;jj=jj+2)
        {
            tmin[jj]=1000;
            tmax[jj]=0;
            sum[jj]=0;
            for(ii=0;ii<MAX_DEV_DBG_ENTRY;ii++)
            {
                delta_usec = (devnodeDbgTimeStamp[ii][jj+1] - devnodeDbgTimeStamp[ii][jj])/TIMER_FREQ;
                tmin[jj]=(delta_usec<tmin[jj]?delta_usec:tmin[jj]);
                tmax[jj]=(delta_usec>tmax[jj]?delta_usec:tmax[jj]);
                sum[jj] += delta_usec;
            }
            printf("\nsingle %s call takes: min= %5d, max=%5d, average=%5d \n",
                      (jj==10? "Memory_getBufferPhysicalAddress":"Memory_getBufferVirtualAddress"), tmin[jj],tmax[jj],sum[jj]/MAX_DEV_DBG_ENTRY);
        }
}

#endif  /* DEVNODE_INTERNAL_DBG */

/*
 *  ======== DEVNODE_create ========
 */
DEVNODE_Handle DEVNODE_create(Engine_Handle engine, String name,
    DEVNODE_Params *params)
{
    DEVNODE_Handle visa;
    static Bool curInit = FALSE;

    if (curInit != TRUE) {
        curInit = TRUE;
        GT_create(&curTrace, GTNAME);
    }

    GT_3trace(curTrace, GT_ENTER, "DEVNODE_create> "
        "Enter (engine=0x%x, name='%s', params=0x%x)\n",
        engine, name, params);

    if (params == NULL) {
        params = &DEVNODE_PARAMS;
    }

    visa = VISA_create(engine, name, (IALG_Params *)params,
        sizeof (_DEVNODE_Msg), "ti.mas.devnode.IDEVNODE");

    GT_1trace(curTrace, GT_ENTER, "DEVNODE_create> return (0x%x)\n", visa);

#ifdef DEVNODE_INTERNAL_DBG
    devnode_SetupTimer();
    memset(devnodeDbgTimeStamp,0,sizeof(devnodeDbgTimeStamp));
    devnodeDbgTimeStampIdx=0;
#endif /* #ifdef DEVNODE_INTERNAL_DBG */

    return (visa);
}


/*
 *  ======== DEVNODE_delete ========
 */
Void DEVNODE_delete(DEVNODE_Handle handle)
{
    GT_1trace(curTrace, GT_ENTER, "DEVNODE_delete> Enter (handle=0x%x)\n",
        handle);

    VISA_delete(handle);

    GT_0trace(curTrace, GT_ENTER, "DEVNODE_delete> return\n");
#ifdef DEVNODE_INTERNAL_DBG
    devnode_FreeTimer();
#endif /* #ifdef DEVNODE_INTERNAL_DBG */
}


/*
 *  ======== DEVNODE_process ========
 *  This method must be the same for both local and remote invocation;
 *  each call site in the client might be calling different implementations
 *  (one that marshalls & sends and one that simply calls).
 */
XDAS_Int32 DEVNODE_process(DEVNODE_Handle handle, DEVNODE_InArgs *inArgs, DEVNODE_OutArgs *outArgs)
{
    XDAS_Int32 retVal = DEVNODE_EFAIL;
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer((Uint32 *)&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][0]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */    
    GT_3trace(curTrace, GT_ENTER, "DEVNODE_process> "
        "Enter (handle=0x%x, inArgs=0x%x, "
        "outArgs=0x%x)\n", handle, inArgs, outArgs);

    if (handle) {
        IDEVNODE_Fxns *fxns =
            (IDEVNODE_Fxns *)VISA_getAlgFxns((VISA_Handle)handle);
        IDEVNODE_Handle alg = VISA_getAlgHandle((VISA_Handle)handle);

        if (fxns && (alg != NULL)) {
            /*
             * To better enable data visualization tooling, drop a
             * well-formed breadcrumb describing what we're doing.
             */
            Log_printf(ti_sdo_ce_dvtLog, "%s", (Arg)"DEVNODE:process",
                (Arg)handle, (Arg)0);
            VISA_enter((VISA_Handle)handle);
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer((Uint32 *)&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][1]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */
            retVal = fxns->process(alg, inArgs, outArgs);
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer((Uint32 *)&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][7]));
    }
#endif /* #ifdef DEVNODE_INTERNAL_DBG */
            VISA_exit((VISA_Handle)handle);
        }
    }

    GT_2trace(curTrace, GT_ENTER, "DEVNODE_process> "
        "Exit (handle=0x%x, retVal=0x%x)\n", handle, retVal);
#ifdef DEVNODE_INTERNAL_DBG
    if(devnodeDbgTimeStampIdx<MAX_DEV_DBG_ENTRY)
    {
        devnode_ReadTimer((Uint32 *)&(devnodeDbgTimeStamp[devnodeDbgTimeStampIdx][8]));
    }
    devnodeDbgTimeStampIdx++;
#if defined(LINUX) || defined(WIN32)
    if(devnodeDbgTimeStampIdx==MAX_DEV_DBG_ENTRY)
    {
        devnode_PrintTimestamps();
        /* Only collect once */
        devnodeDbgTimeStampIdx++;
    }
#endif  /* defined(LINUX) || defined(WIN32) */
#endif /* #ifdef DEVNODE_INTERNAL_DBG */
    return (retVal);
}

/*
 *  ======== DEVNODE_control ========
 *  This method must be the same for both local and remote invocation;
 *  each call site in the client might be calling different implementations
 *  (one that marshalls & sends and one that simply calls).  This API
 *  abstracts *all* image decoders (both high and low complexity
 *  decoders are envoked using this method).
 */
XDAS_Int32 DEVNODE_control(DEVNODE_Handle handle, DEVNODE_Cmd cmd, DEVNODE_Ctrl_InArgs *inArgs, DEVNODE_Ctrl_OutArgs *outArgs)
{
    XDAS_Int32 retVal = DEVNODE_EFAIL;

    GT_3trace(curTrace, GT_ENTER, "DEVNODE_control> "
        "Enter (handle=0x%x, inArgs=%d, outArgs=0x%x\n",
        handle, inArgs, outArgs);

    if (handle) {
        IDEVNODE_Fxns *fxns =
            (IDEVNODE_Fxns *)VISA_getAlgFxns((VISA_Handle)handle);
        IDEVNODE_Handle alg = VISA_getAlgHandle((VISA_Handle)handle);

        if (fxns && (alg != NULL)) {
            /*
             * To better enable data visualization tooling, drop a
             * well-formed breadcrumb describing what we're doing.
             */
            Log_printf(ti_sdo_ce_dvtLog, "%s", (Arg)"DEVNODE:control",
                (Arg)handle, (Arg)0);
            VISA_enter((VISA_Handle)handle);
            retVal = fxns->control(alg, cmd, inArgs, outArgs);
            VISA_exit((VISA_Handle)handle);
        }
    }

    GT_2trace(curTrace, GT_ENTER, "DEVNODE_control> "
        "Exit (handle=0x%x, retVal=0x%x)\n", handle, retVal);

    return (retVal);
}
