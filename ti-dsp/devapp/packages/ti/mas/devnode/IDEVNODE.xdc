/*
 *  Copyright 2007 by Texas Instruments Incorporated.
 *
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 */

/*!
 *  ======== IDEVNODE ========
 *  Scale Codec Configuration interface
 *
 *  All Scale Codec modules must implement this configuration interface.
 */
metaonly interface IDEVNODE inherits ti.sdo.ce.ICodec
{
    override config String serverFxns = "DEVNODE_SKEL";
    override config String stubFxns = "DEVNODE_STUBS";

    override readonly config Int rpcProtocolVersion = 1;
}

