/*------------------------------------------------------------------------
 . ks8851.h - Micrel KS8851 Ethernet driver header
 .
 .
 . This program is free software; you can redistribute it and/or modify
 . it under the terms of the GNU General Public License as published by
 . the Free Software Foundation; either version 2 of the License, or
 . (at your option) any later version.
 .
 . This program is distributed in the hope that it will be useful,
 . but WITHOUT ANY WARRANTY; without even the implied warranty of
 . MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 . GNU General Public License for more details.
 .
 . You should have received a copy of the GNU General Public License
 . along with this program; if not, write to the Free Software
 . Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 .
 ---------------------------------------------------------------------------*/

#ifndef _KS8851_H_
#define _KS8851_H_

#include "ks8851Reg.h"

#define MAC_ADDRESS_LENGTH       6
#define MAC_ADDR_ORDER( i )      ( MAC_ADDRESS_LENGTH - 1 - ( i ))


/* LinkMD definition */

enum {
    CABLE_UNKNOWN,
    CABLE_GOOD,
    CABLE_CROSSED,
    CABLE_REVERSED,
    CABLE_CROSSED_REVERSED,
    CABLE_OPEN,
    CABLE_SHORT
};

#define STATUS_FULL_DUPLEX  0x01
#define STATUS_CROSSOVER    0x02
#define STATUS_REVERSED     0x04

#define LINK_10MBPS_FULL    0x00000001
#define LINK_10MBPS_HALF    0x00000002
#define LINK_100MBPS_FULL   0x00000004
#define LINK_100MBPS_HALF   0x00000008
#define LINK_1GBPS_FULL     0x00000010
#define LINK_1GBPS_HALF     0x00000020
#define LINK_10GBPS_FULL    0x00000040
#define LINK_10GBPS_HALF    0x00000080
#define LINK_SYM_PAUSE      0x00000100
#define LINK_ASYM_PAUSE     0x00000200

#define LINK_AUTO_MDIX      0x00010000
#define LINK_MDIX           0x00020000
#define LINK_AUTO_POLARITY  0x00040000


#define CABLE_LEN_MAXIMUM     15000
#define CABLE_LEN_MULTIPLIER  1 /* 41 */

#define PHY_RESET_TIMEOUT  10


/* MIB counter definition */

#define PORT_COUNTER_NUM                 0x20

#define MIB_COUNTER_RX_BYTE              0x00
#define MIB_COUNTER_RESERVED             0x01
#define MIB_COUNTER_RX_UNDERSIZE         0x02
#define MIB_COUNTER_RX_FRAGMENT          0x03
#define MIB_COUNTER_RX_OVERSIZE          0x04
#define MIB_COUNTER_RX_JABBER            0x05
#define MIB_COUNTER_RX_SYMBOL_ERR        0x06
#define MIB_COUNTER_RX_CRC_ERR           0x07
#define MIB_COUNTER_RX_ALIGNMENT_ERR     0x08
#define MIB_COUNTER_RX_CTRL_8808         0x09
#define MIB_COUNTER_RX_PAUSE             0x0A
#define MIB_COUNTER_RX_BROADCAST         0x0B
#define MIB_COUNTER_RX_MULTICAST         0x0C
#define MIB_COUNTER_RX_UNICAST           0x0D
#define MIB_COUNTER_RX_OCTET_64          0x0E
#define MIB_COUNTER_RX_OCTET_65_127      0x0F
#define MIB_COUNTER_RX_OCTET_128_255     0x10
#define MIB_COUNTER_RX_OCTET_256_511     0x11
#define MIB_COUNTER_RX_OCTET_512_1023    0x12
#define MIB_COUNTER_RX_OCTET_1024_1521   0x13
#define MIB_COUNTER_RX_OCTET_1522_2000   0x14
#define MIB_COUNTER_TX_BYTE              0x15
#define MIB_COUNTER_TX_LATE_COLLISION    0x16
#define MIB_COUNTER_TX_PAUSE             0x17
#define MIB_COUNTER_TX_BROADCAST         0x18
#define MIB_COUNTER_TX_MULTICAST         0x19
#define MIB_COUNTER_TX_UNICAST           0x1A
#define MIB_COUNTER_TX_DEFERRED          0x1B
#define MIB_COUNTER_TX_TOTAL_COLLISION   0x1C
#define MIB_COUNTER_TX_EXCESS_COLLISION  0x1D
#define MIB_COUNTER_TX_SINGLE_COLLISION  0x1E
#define MIB_COUNTER_TX_MULTI_COLLISION   0x1F

#define MIB_TABLE_ENTRIES                (PORT_COUNTER_NUM )

typedef enum
{
    OID_COUNTER_UNKOWN,

    OID_COUNTER_FIRST,
    OID_COUNTER_DIRECTED_BYTES_XMIT = OID_COUNTER_FIRST, /* total bytes transmitted  */
    OID_COUNTER_DIRECTED_FRAMES_XMIT,    /* total packets transmitted */

    OID_COUNTER_BROADCAST_BYTES_XMIT,
    OID_COUNTER_BROADCAST_FRAME_XMIT,

    OID_COUNTER_DIRECTED_BYTES_RCV,      /* total bytes received   */
    OID_COUNTER_DIRECTED_FRAMES_RCV,     /* total packets received */
    OID_COUNTER_BROADCAST_BYTES_RCV,
    OID_COUNTER_BROADCAST_FRAMES_RCV,    /* total broadcast packets received (RXSR: RXBF)                */
    OID_COUNTER_MULTICAST_FRAMES_RCV,    /* total multicast packets received (RXSR: RXMF) or (RDSE0: MF) */
    OID_COUNTER_UNICAST_FRAMES_RCV,      /* total unicast packets received   (RXSR: RXUF)                */

    OID_COUNTER_XMIT_ERROR,              /* total transmit errors */
    OID_COUNTER_XMIT_LATE_COLLISION,     /* transmit Late Collision (TXSR: TXLC) */
    OID_COUNTER_XMIT_MORE_COLLISIONS,    /* transmit Maximum Collision (TXSR: TXMC) */
    OID_COUNTER_XMIT_ALLOC_FAIL,         /* transmit fail because no enought memory in the Tx Packet Memory */
    OID_COUNTER_XMIT_DROPPED,            /* transmit packet drop because no buffer in the host memory */

    OID_COUNTER_XMIT_INT_STOP,           /* transmit MAC process stop from interrupt status (ISR) */
    OID_COUNTER_XMIT_SPACE_INT,          /* transmit space available (ISR)*/
    OID_COUNTER_XMIT_INT,                /* transmit Tx interrupt status (ISR) */

    OID_COUNTER_RCV_ERROR,               /* total receive errors */
    OID_COUNTER_RCV_ERROR_CRC,           /* receive packet with CRC error (RXSR: RXCE) or (RDSE0: CE) */
    OID_COUNTER_RCV_ERROR_MII,           /* receive MII error (RXSR: RXMR) or (RDSE0: RE) */
    OID_COUNTER_RCV_ERROR_TOOLONG,       /* receive frame too long error (RXSR: RXTL) or (RDSE0: TL)  */
    OID_COUNTER_RCV_ERROR_RUNT,          /* receive frame was demaged by a collision  */
    OID_COUNTER_RCV_INVALID_FRAME,       /* receive invalid frame (RXSR: RXFV) */
    OID_COUNTER_RCV_ERROR_ICMP,          /* receive frame with ICMP checksum error (RDSE0: UDPE) */
    OID_COUNTER_RCV_ERROR_IP,            /* receive frame with IP checksum error  (RDSE0: IPE) */
    OID_COUNTER_RCV_ERROR_TCP,           /* receive frame with TCP checksum error (RDSE0: TCPE) */
    OID_COUNTER_RCV_ERROR_UDP,           /* receive frame with UDP checksum error (RDSE0: UDPE) */
    OID_COUNTER_RCV_DROPPED,             /* receive packet drop because no buffer in the host memory */

    OID_COUNTER_RCV_INT_WOL,             /* receive wol wakeup frame detect from interrupt status (ISR) */
    OID_COUNTER_RCV_INT_MAGIC,           /* receive wol magic packet detection from interrupt status (ISR) */
    OID_COUNTER_RCV_INT_LINKUP,          /* receive wol link up detection from interrupt status (ISR) */
    OID_COUNTER_RCV_INT_ENERGY,          /* receive wol energy detection from interrupt status (ISR) */
    OID_COUNTER_RCV_INT_ENERGY_DELAY,    /* receive wol energy delay detection from interrupt status (ISR) */
    OID_COUNTER_RCV_INT_OVERRUN,         /* receive overrun from interrupt status (ISR) */
    OID_COUNTER_RCV_INT_STOP,            /* receive MAC process stop from interrupt status (ISR) */
    OID_COUNTER_RCV_INT,                 /* receive Rx interrupt status (ISR) */

    OID_COUNTER_PHY_INT,                 /* Phy link change interrupt status (ISR) */

    OID_COUNTER_XMIT_OK,
    OID_COUNTER_RCV_OK,

    OID_COUNTER_RCV_ERROR_LEN,

    OID_COUNTER_LAST
} EOidCounter;

typedef struct {
    u32     cnCounter[ PORT_COUNTER_NUM ];    /* Device MIB counter */
    u32     m_cnCounter[ OID_COUNTER_LAST ];  /* Driver statistics counter */
} MIB_CONFIG, *PMIB_CONFIG;


/*
extern void  PrintPacketData ( UCHAR *, int, ULONG );
extern void  PortClearCounters ( PMIB_CONFIG );
extern void  HardwareReadMIBCounter ( USHORT, PULONGLONG );
*/

extern void  ksread ( ulong, ulong );
extern void  kswrite ( ulong, ulong, ulong );

#endif  /* _KS8851_H_ */
