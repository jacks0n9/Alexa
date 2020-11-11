#ifndef _HAL_H
#define _HAL_H
/*****************************************************************************
 * FILE PURPOSE: Define the application programming interface to HAL
 *****************************************************************************
 * DESCRIPTION: The Hardware Abstraction Layer (HAL) defines an API which 
 *              is conistent accross hardware platforms. This file defines
 *              the C-level HAL API.
 *
 *              HAL is designed to exist as a seperate application which can
 *              be seperately linked and loaded into DSP memory. 
 *
 * TABS: NONE
 *
 * Copywrite (C) 2002, Telogy Networks, Inc. 
 *
 *****************************************************************************/
#include <ti/mas/types/types.h>
#include "afe.h"
#include <ti/mas/util/ecomem.h>

typedef void* tfPtr;

/****************************************************************************/
/* Memory buffer information returned to application                        */
/****************************************************************************/
#define HAL_NBUFS                   3
#define HAL_HEAP_BUFN               0
#define HAL_TDM0_DMA_TX_BUFN        1
#define HAL_TDM0_DMA_RX_BUFN        2


/*****************************************************************************
 * DEFINITION: HAL return values to application.
 *****************************************************************************/
typedef enum {
  HAL_SUCCESS = 0,
  HAL_FAIL,
  HAL_HEAP_MEMORY_ERR,           /* Error encountered allocating heap memory */
  HAL_DETECT_NO_CLOCKS,          /* No clocks detected on TDM port           */
  HAL_ILLEGAL_CLOCK_CONFIG,      /* Clocks < nTimeslots or Clocks > Max      */
  HAL_DMA_UNAVAILABLE,           /* DMA resource unavailable                 */
  HAL_TDM_ISR_UNAVAILABLE,       /* TDM isr resource unavailable             */
  HAL_TDM_INVALID_HAL_TDM_ID,    /* Invalid tdm port specified               */
  HAL_TDM_INVALID_TDM_ON_IO,     /* Attempted tdm on IO port                 */
  HAL_TDM_INVALID_TDM_CONFIG,    /* Invalid tdm enable/disable config        */
  HAL_TDM_INVALID_TS,            /* Invalid timeslot specified               */
  HAL_TDM_INVALID_TX_RX_TS_PAIR, /* Invalid tx rx timeslot pair              */
  HAL_TDM_TS_ALREADY_ENABLED,    /* Asked to enable an already enabled ts    */
  HAL_TDM_NO_FREE_TS,            /* Too many timeslots requested             */
  HAL_TDM_INVALID_SUBFRAME_LEN,  /* Invalid Sub frame length                 */ 
  HAL_TDM_INVALID_COMBINE,       /* Invalid tdm resource combine             */
  HAL_TDM_QUEUE_FULL_OPEN,       /* TDM Enable Fail - Queue full */
  HAL_TDM_QUEUE_FULL_CLOSE       /* TDM Disable Fail - Queue full */
} halReturn_t;

typedef ecomemBuffer_t halMemBuffer_t;

/* Memory class definitions */
enum {
  HAL_MEM_CLASS_EXTERNAL = 0,   /* generic (bulk) external memory */
  HAL_MEM_CLASS_INTERNAL = 1    /* generic (bulk) internal memory */
};

/*****************************************************************************
 * DEFINITION: Some TDM peripherals can be configured as gpio
 *****************************************************************************/
typedef enum {
  HAL_TDM_AS_TDM,   /* Port is used as a TDM port */
  HAL_TDM_AS_GPIO   /* Port is used as GPIO       */
} halSportFcn_t;

/*****************************************************************************
 * DEFINITION: The TDM ports can be configured to start up when HAL is
 *             initialized, or to start up when the 1st timeslot is enabled
 *             and shutdown when the last timeslot is disabled.
 *****************************************************************************/
typedef enum {
  HAL_TDM_ALWAYS_ON,     /* TDM port is always on                          */
  HAL_TDM_ON_AS_NEEDED   /* TDM port is on only when a timeslot is enabled */
} halTdmStart_t;

/*****************************************************************************
 * Definition: McBSP type. This only is used for the C54x to distinguish the
 *             McBSP from the eMcBSP
 *****************************************************************************/
typedef enum {
  HAL_TDM_EMCBSP,
  HAL_TDM_MCBSP
} halMcBspType_t;

/*****************************************************************************
 * DEFINITION: Configuration of TDM port in TDM mode
 ****************************************************************************/
#define HAL_TDM_AUTO_DETECT_CLOCKS  0   /* Auto detect clocks per frame */
typedef struct halTdmTdm_s {
  tint  maxChannels;    /* Max number of timeslots a SINGLE core can open    */
  tint  maxRxChannels;  /* Max number of timeslots a SINGLE core can open    */
  tint  maxTxChannels;  /* Max number of timeslots a SINGLE core can open    */
  tuint wordSize;       /* Length of TDM tword, in bits                       */
  tuint rxSubFrameRate; /* Number of samples buffered by DMA in HAL          */
  tuint txSubFrameRate; /* Number of samples buffered by DMA in HAL          */
  tuint buffIncr;       /* The offset where the next buffer will be located  */
  tint  bdxDelay;       /* 1st bit delay enable - for wired or'd ports       */
  tuint silence;        /* TDM output silence pattern                        */
  halTdmStart_t start;  /* Startup/shutdown status of TDM port               */
  tint  maxPhase;       /* Maximum number of phases port tracks              */
  tint  maxMegaPhase;   /* Maximum number of mega phases tracked             */
  tint  mcbsp_block;    /* Which block of 32 timeslots the TDM "sees". This is
                         * only the McBSP. The eMcBSP can see the full range
                         * of timeslots.                                     */
  halMcBspType_t type;  /* Type of McBsp                                     */ 

  /* Periodic Function to call at every subframe */
  void  *handle;
  void (*subFrameCallout) (void *, tuint, Uint32);

  tuint *initTs;        /* If non NULL, this array must contain maxChannels 
                         * values which are the timeslots to bring up (tx and
                         * rx) at startup.                                    */
} halTdmTdm_t;

/*****************************************************************************
 * DEFINITION: TDM port configuration structure for HAL. 
 *****************************************************************************/
typedef struct halTdmInit_s {
  tint  halId;          /* Returned by HAL after new         */
  halSportFcn_t fcn;    /* Use of port, for TDM or for GPIO  */
  union {
    halTdmTdm_t  tdm;
  } u;
 } halTdmInit_t; 

typedef enum {
   HAL_BIT_EXACT,
   HAL_MULAW,
   HAL_ALAW
} halCompand_t;

typedef enum {
   HAL_MONO,
   HAL_STEREO
} halTsType_t;

typedef enum {
   HAL_TDM_TIMESLOT_CONFIG,
   HAL_TDM_TDM_CONFIG,
   HAL_TDM_PHASE_CONFIG,
   HAL_TDM_PHASE_QUERY,
   HAL_TDM_RESOURCE_COMBINE,
   HAL_TDM_SET_CALLOUT,
   HAL_TDM_MEGA_STAGGER,
   HAL_TDM_PHASE_RESET
} halTdmControlMode_t;

/*****************************************************************************
 * tdmClockCfg bit mapped parameters - must match hardware config bit mapping
 *****************************************************************************/
#define HAL_TDM_TDM_CLOCK_CFG_TX_DAT_DELAY(x)      ((x) & 0x0007)
#define HAL_TDM_TDM_CLOCK_CFG_TX_CLK_POLARITY(x)   (((x)>>3)&0x0001)
#define HAL_TDM_TDM_CLOCK_CFG_TX_FSYNC_POLARITY(x) (((x)>>4)&0x0001)
#define HAL_TDM_TDM_CLOCK_CFG_RX_DAT_DELAY(x)      (((x)>>8)&0x0007)
#define HAL_TDM_TDM_CLOCK_CFG_RX_CLK_POLARITY(x)   (((x)>>11)&0x0001)
#define HAL_TDM_TDM_CLOCK_CFG_RX_FSYNC_POLARITY(x) (((x)>>12)&0x0001)
#define HAL_TDM_TDM_CLOCK_CFG_DELAY_MASK           0x0007
#define HAL_TDM_TDM_CLOCK_DAT_DELAY_OFFSET         1

typedef struct halTdmTdmControl_s {
  tuint wordSize;         /* Length of TDM tword, in bits                       */
  tint  bdxDelay;         /* 1st bit delay enable - for wired or'd ports       */
  tuint rxClocksPerFrame; /* Number of bit clocks received per frame sync      */
  tuint txClocksPerFrame; /* Number of bit clocks received per frame sync      */
  tuint clocksPerFrame;   /* Number of bit clocks received per frame sync      */
  tuint tdmClockCfg;      /* TDM clocking configuration - must match value
                           * from hardware config message.                     */
  tuint physPort;         /* Physical TDM port number (indexed from 0)         */
  tuint rxSubFrameRate;   /* Number of samples buffered by DMA in HAL          */
  tuint txSubFrameRate;   /* Number of samples buffered by DMA in HAL          */
  tuint rxSamplingFreq;   /* */
  tuint txSamplingFreq;   /* */
} halTdmTdmControl_t;

typedef struct halTdmControl_s {
  tint         toTdmEnable;    /* Enables or disables tdm xmit             */
  tint         fromTdmEnable;  /* Enables or disables tdm rcv              */
  tint         txTimeslot;     /* TDM Tx timeslot number                   */
  tint         rxTimeslot;     /* TDM Rx timeslot number                   */
  halCompand_t compand;        /* Companding                               */
  tuint        initVal;        /* initial value of TDM                     */
  tuint        txFrameSize;    /* Must be multiple of subFrameRate         */
  tuint        rxFrameSize;    /* Must be multiple of subFrameRate         */
  tuint        phase;          /* Channel staggering phase                 */
  /* Callout Functions when frame received */
  void (*txCallout) (void *, void *, linSample **, tuint, tuint, tuint, tuint);
  void (*rxCallout) (void *, void *, linSample **, tuint, tuint, tuint, tuint);
  tfPtr        txContext;      /* 1st argument to pass back in txCallout   */
  tfPtr        rxContext;      /* 1st argument to pass back in rxCallout   */
  linSample   *txBuffer;       /* Initial tx data to send out              */
  linSample   *rxBuffer;       /* Place to store initial rx data           */
  tuint        txBufSize;      /* Size of tx buffers                       */
  tuint        rxBufSize;      /* Size of rx buffers                       */
  void        *hId;            /* Restore ID used for mega-staggering      */
  halTdmControlMode_t mode;    /* Control Mode: Timeslot or TDM Config     */
  halTdmTdmControl_t  tdmCtrl; /* TDM Run Time Re-Configuration Parameters */
  halTsType_t tstype;          /* Time slot can be mono or a stereo        */
} halTdmControl_t;

/*****************************************************************************
 * DEFINITION: Structure used by HAL to determine its size requirements
 *****************************************************************************/
typedef struct {
  tint nTdmPorts;        /* Number of elements in halTdmInit array */
  halTdmInit_t *tdm;     /* Array of TDM port init structures      */
  tint coreId;           /* Returns the core ID                    */
  tuint revId;           /* Returns the chip revision ID, in  Q.4  */
} halNewConfig_t;

/*****************************************************************************
 * DEFINITION: Structures used for HAL Control Functions 
 *****************************************************************************/
#define HAL_MAX_TDM_PORTS 4    /* 4 TDM ports allowed as follows */
/* CAUTION: do not change the definitions of HAL_TDM_0 through HAL_TDM_4 */
/*          if change is necessary, change halControl code appropriately */
typedef enum {
  HAL_TDM,       /* TDM Ports          */
  HAL_HPI,       /* HPI Peripheral.    */
  HAL_TMR,       /* TIMERS             */
  HAL_EMIF,      /* EMIF Configuration */
  HAL_UL2,       /* UTOPIA Port        */
  HAL_PCI        /* PCI Peripheral     */
} halPeriph_t;

typedef enum {
  HAL_PORT_ENABLE,  /* Enable Port */
  HAL_PORT_DISABLE  /* Disable Port */
} halCtlOp_t;

typedef struct halControl_s {
  halPeriph_t  periph;         /* Peripherals to Control             */
  halCtlOp_t   op;             /* Operation to perform               */
  tint         portHandle;     /* HAL port handle assigned by halNew */

} halControl_t;

/*****************************************************************************
 * DEFINITION: The HAL API is defined by a structure of function calls. HAL
 *             creates an instance of this structure and populates it with
 *             the required function pointers. A pointer to this structure 
 *             is provided to the application to access the HAL functions.
 *****************************************************************************/
typedef struct halApi_s {
  /* Memory Initialization */
  halReturn_t (*halGetSizes) (tint *nbufs, halMemBuffer_t **bufs, halNewConfig_t *cfg);
  halReturn_t (*halNew) (tint  nbufs, halMemBuffer_t *bufs, halNewConfig_t *cfg);

  /* Starting the Peripherals */
  halReturn_t (*halControl) (halControl_t *hal_ctrl );

  /* TDM port */
  halReturn_t (*halTdmInit)    (tint portId, halTdmInit_t *cfg);
  halReturn_t (*halTdmControl) (tint portId, halTdmControl_t *ctl);
} halApi_t;
  
/* For now declare an external HAL type */
extern halApi_t hal;

#endif /* _HAL_H */


