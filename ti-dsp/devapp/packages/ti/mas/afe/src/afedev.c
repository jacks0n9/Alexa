#include <xdc/std.h>
#include <string.h>

#include <std.h>
#include <dev.h>
#include <fxn.h>
#include <mem.h>
#include <que.h>
#include <sem.h>
#include <sys.h>
#include <hwi.h>

#include <src/afedev.h>
#include <src/afedevloc.h>
#include <src/afeloc.h>
#include <src/afedevalg.h>

#include <ti/mas/afe/src/afeinstru.h>

#include <ti/mas/inc/frame.h>

#include "control.h"

static Int      DAF_close(DEV_Handle device);
static Int      DAF_ctrl(DEV_Handle device, Uns cmd, Arg arg);
static Int      DAF_idle(DEV_Handle device, Bool flush);
static Int      DAF_issue(DEV_Handle device);
static Int      DAF_open(DEV_Handle device, String name);
static Bool     DAF_ready(DEV_Handle device, SEM_Handle sem);
static Int      DAF_reclaim(DEV_Handle device);

/*
 *  Driver function table.
 */
far DEV_Fxns DAF_FXNS = {
    DAF_close,          /* close */
    DAF_ctrl,           /* ctrl */
    DAF_idle,           /* idle */
    DAF_issue,          /* issue */
    DAF_open,           /* open */
    DAF_ready,          /* ready */
    DAF_reclaim,        /* reclaim */
};

DAF_Params DAF_PARAMS = {
  16000,
  10,
  2
};

#define DAF_MAX_INSTANCES 1

far DAF_Obj DAF_Global[DAF_MAX_INSTANCES];

// Track driver initialization
far Bool  bAfeDevDeviceInit = FALSE;

/*
 *  ======== DAF_init ========
 */
Void DAF_init (DEV_Attrs *afedevAttr)
{
  afedevAttr = afedevAttr;

  return;
}

Int DAF_new (Void)
{
  DAF_Obj *pDAF_Global;
  MdUns usInstanceId;

  /* run this only once */
  if (bAfeDevDeviceInit == TRUE)
  {
      return SYS_OK;
  }
  bAfeDevDeviceInit = TRUE;

  /* init global structure */
  usInstanceId = 0;
  pDAF_Global = &DAF_Global[usInstanceId];

  pDAF_Global->bInitTx     = FALSE;
  pDAF_Global->bReadyTx    = FALSE;
  pDAF_Global->bInitRx     = FALSE;
  pDAF_Global->bReadyRx    = FALSE;

  ((AFE_Object *)(pDAF_Global->pAfeObj))->pAfeDevObj = pDAF_Global;
  return SYS_OK;
}

/*
 *  ======== DAF_open ========
 */
Int DAF_open(DEV_Handle pDevice, String Name)
{
    DAF_Obj *pDAF_Global;
    MdUns usInstanceId;
    Int createStatus = SYS_OK;
    Name = Name;

    /* init global structure */
    usInstanceId = 0;
    pDAF_Global = &DAF_Global[usInstanceId];

    pDAF_Global->pAfeObj = ((DAF_Params *)(pDevice->params))->afedevObj;

    /* init driver if not already done */
    createStatus = DAF_new();

    if (createStatus != SYS_OK)
    {
        return createStatus;
    }

    /* Instance number */
    usInstanceId = 0;

    /* retrieve pointer to global instance */
    pDAF_Global = &DAF_Global[usInstanceId];
    pDevice->object = pDAF_Global;

    if (pDevice->mode == DEV_INPUT)
    {
        if (pDAF_Global->bInitRx == TRUE)
            return SYS_EBUSY;

        /* mark as used, store pointer to device handle */
        pDAF_Global->bInitRx = TRUE;
        pDAF_Global->ptDeviceRx = pDevice;
        pDAF_Global->bEnabledRx = FALSE;
    }
    else /* DEV_OUTPUT */
    {
        if (pDAF_Global->bInitTx == TRUE)
            return SYS_EBUSY;

        /* mark as used, store pointer to device handle */
        pDAF_Global->bInitTx = TRUE;
        pDAF_Global->ptDeviceTx = pDevice;
        pDAF_Global->bEnabledTx = FALSE;
    }
    if ( (pDAF_Global->bInitRx == TRUE) &&
         (pDAF_Global->bInitTx == TRUE) )
    {
       //start the driver
       if (AFE_SUCCESS != AFE_open(pDAF_Global->pAfeObj))
       {
           return SYS_ENOTIMPL;
       }

    }
    return SYS_OK;
}

/*
 *  ======== DAF_close ========
 */
Int DAF_close(DEV_Handle pDevice)
{
    DAF_Obj *pDAF_Global = pDevice->object;

    if (pDevice->mode == DEV_INPUT)
    {
        if (pDAF_Global->bInitRx == FALSE)
            return SYS_EINVAL;

        pDAF_Global->bInitRx = FALSE;
        pDAF_Global->ptDeviceRx = NULL;
    }
    else /* DEV_OUTPUT */
    {
        if (pDAF_Global->bInitTx == FALSE)
            return SYS_EINVAL;

        pDAF_Global->bInitTx = FALSE;
        pDAF_Global->ptDeviceTx = NULL;
    }
    if ( (pDAF_Global->bInitRx == FALSE) &&
         (pDAF_Global->bInitTx == FALSE) )
    {
       AFEDEV_delete(pDAF_Global->pAfeObj);
       bAfeDevDeviceInit = FALSE;
    }
    return SYS_OK;
}

/*
 *  ======== DAF_ctrl ========
 */
Int DAF_ctrl(DEV_Handle pDevice, Uns cmd, Arg inArgs)
{
  UInt32 msgID = (UInt32) cmd;
  UInt16 status = SYS_OK;
  DAF_SioCtrl *sioCtrl = (DAF_SioCtrl *)inArgs;

  DAF_Obj *pDAF_Global = pDevice->object;

  switch (msgID)
    {
      case AFE_MSG_HW_CONFIG:
        status = AFE_processHwConfig(pDAF_Global->pAfeObj, (AFE_HwConfig *)(sioCtrl->ctrl));
        break;

      case AFE_MSG_STREAM_CONFIG:
        status = AFE_processStreamConfig(pDAF_Global->pAfeObj, (AFE_StreamConfig *)(sioCtrl->ctrl));
        break;

      case AFE_MSG_SHUTDOWN:
        status = AFE_shutDown(pDAF_Global->pAfeObj);
        break;

      case AFE_MSG_LAB:
      {
          // Lab messages
          status = ctrlMsgHandler((tCtrlMsg *)  sioCtrl->ctrl);
      }
      break;


      default:
        status = AFE_STATUS_INVALID_MSG;
        break;
    }


  return status;
}

/*
 *  ======== DAF_idle ========
 */
Int DAF_idle(DEV_Handle pDevice, Bool bFlush)
{
    DAF_Obj *pDAF_Global = pDevice->object;
    Uns   usOldST1;
    MdUns frameCount;

    bFlush = bFlush;

    if (pDevice->mode == DEV_INPUT)
    {
        pDAF_Global->bReadyRx = FALSE;
    }
    else /* DEV_OUTPUT */
    {
        pDAF_Global->bReadyTx = FALSE;
    }

    /* transfer buffers back to todevice queue */
    usOldST1 = HWI_disable();

    /* move all frames to todevice queue to keep them in order of arrival */
    while (!QUE_empty(pDevice->todevice))
        QUE_put(pDevice->fromdevice, QUE_get(pDevice->todevice));

    /* temporarly move all frames to fromdevice queue */
    while (!QUE_empty(pDevice->fromdevice))
        QUE_put(pDevice->todevice, QUE_get(pDevice->fromdevice));

    /* move all frames to todevice queue while counting them */
    while (!QUE_empty(pDevice->todevice))
    {
        frameCount++;
        QUE_put(pDevice->fromdevice, QUE_get(pDevice->todevice));
    }
    HWI_restore(usOldST1);

    while (!QUE_empty(pDevice->fromdevice) && ((frameCount--)>0))
    {
        if (pDevice->mode == DEV_INPUT)
        {
            /* unlock DAF_ready */
            if (pDAF_Global->pReadyRx != NULL)
                SEM_postBinary (pDAF_Global->pReadyRx);
        }
        else /* DEV_OUTPUT */
        {
            /* unlock DAF_ready */
            if (pDAF_Global->pReadyTx != NULL)
                SEM_postBinary (pDAF_Global->pReadyTx);
        }
    }

    return SYS_OK;
}

/*
 *  ======== DAF_issue ========
 */
int bypasss = 0;

Int DAF_issue(DEV_Handle pDevice)
{
  DAF_Obj *pDAF_Global = pDevice->object;
  DEV_Frame *ptFrame;

    /* Check if this is the first time issue happens in a session then Resync the toMsp/fromMsp buffers */
    AFE_resync(pDAF_Global->pAfeObj, pDevice->timeout);

    if (pDevice->mode == DEV_INPUT)
    {
      if(pDAF_Global->bReadyRx == FALSE)
      {
        /* identify that first buffer has been received */
        pDAF_Global->bReadyRx = TRUE;
      }
    }
    else /* DEV_OUTPUT */
    {
      if(pDAF_Global->bReadyTx == FALSE)
      {
        /* identify that first buffer has been received */
        pDAF_Global->bReadyTx = TRUE;
      }

      /* copy frame data from AFE buffer into the DAF buffer */
      if (!QUE_empty(pDAF_Global->ptDeviceTx->todevice))
        ptFrame = (DEV_Frame *) QUE_get (pDAF_Global->ptDeviceTx->todevice);
      else
        ptFrame = NULL;

      if (ptFrame != NULL)
      {
        /* move buffer from toDevice to fromDevice queue */
        QUE_put (pDAF_Global->ptDeviceTx->fromdevice, ptFrame);
      }
    }

    return SYS_OK;
}

/*
 *  ======== DAF_ready ========
 */
Bool DAF_ready(DEV_Handle pDevice, SEM_Handle pSem)
{
    DAF_Obj *pDAF_Global = pDevice->object;

    Bool       bStatus = FALSE;
    Uns        usOldST1;
    SEM_Handle *pReady;

    if (pDevice->mode == DEV_INPUT)
    {
        pReady = &pDAF_Global->pReadyRx;
    }
    else /* DEV_OUTPUT */
    {
        pReady = &pDAF_Global->pReadyTx;
    }

    usOldST1 = HWI_disable();
    *pReady = pSem;
    bStatus = !(QUE_empty(pDevice->fromdevice));
    HWI_restore(usOldST1);

    return bStatus;
}

/*
 *  ======== DAF_reclaim ========
 */
Int DAF_reclaim(DEV_Handle pDevice)
{
    DAF_Obj *pDAF_Global = pDevice->object;

    DEV_Frame *ptFrame;

    if (pDevice->mode == DEV_INPUT)
    {
        /* copy frame data from AFE buffer into the DAF buffer */
        /* and move buffer from toDevice to fromDevice queue */
        if (!QUE_empty(pDAF_Global->ptDeviceRx->todevice))
            ptFrame = (DEV_Frame *) QUE_get (pDAF_Global->ptDeviceRx->todevice);
        else
            ptFrame = NULL;

        if (ptFrame != NULL)
        {
            /* release Rx frame back to application */
            QUE_put (pDAF_Global->ptDeviceRx->fromdevice, ptFrame);
        }
    }
    else /* DEV_OUTPUT */
    {
    }

    if (!(QUE_empty(pDevice->fromdevice)))
        return SYS_OK;
    else
        return SYS_ETIMEOUT;
}


