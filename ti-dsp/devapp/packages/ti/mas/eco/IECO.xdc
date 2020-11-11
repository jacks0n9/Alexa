/*
 *  Copyright 2009 by Texas Instruments Incorporated.
 *
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 */

/*!
 *  ======== IECO ========
 *  Embedded Communication Object Interface
 *
 *  All ECO modules must implement this configuration interface.
 */
metaonly interface IECO inherits ti.sdo.ce.ICodec
{
    override config String serverFxns = "ECO_SKEL";
    override config String stubFxns = "ECO_STUBS";

    override readonly config Int rpcProtocolVersion = 1;
}

