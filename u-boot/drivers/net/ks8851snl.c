/*
 * Micrel KSZ8851SNL Network driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <spi.h>
#include <malloc.h>
#include <net.h>
#include <linux/types.h>
#include "../spi/omap3_spi.h"
#include "ks8851snl.h"


#define DRIVERNAME "ksz8851snl"


#define mdelay(n)       udelay((n)*1000)

/* KS8851 SPI Command definition */

#define SPI_REG_READ	    0x00    /* KS8851SNL Register SPI Read  Command */
#define SPI_REG_WRITE	    0x40    /* KS8851SNL Register SPI Write Command */
#define SPI_RXQ_READ	    0x80    /* KS8851SNL RXQ SPI Read  Command */
#define SPI_TXQ_WRITE	    0xC0    /* KS8851SNL TXQ SPI Write Command */

#define SPI_BYTE_ACCESS	    0x01    /* KS8851SNL BYTE access  */
#define SPI_WORD_ACCESS	    0x03    /* KS8851SNL WORD access  */
#define SPI_DWORD_ACCESS	0x0F    /* KS8851SNL DWORD access */


typedef struct ks_device {
	struct eth_device	*dev;	/* back pointer */
	struct spi_slave	*slave;
    PMIB_CONFIG          pmib;
	u32                  rx_frame_count;
	int                  initstate;
	int                  fdump_tx;
	int                  fdump_rx;
} ks_dev_t;


/* global */
struct eth_device *g_dev=NULL;

u8 default_eth_mac[] = {
	0x00, 0x10, 0xA1, 0x88, 0x51, 0x01
};


/*
 * local functions to access SPI
 *
 * reg: register inside KSZ8851SNL
 * data: 8/16 bits to write
 *
 * ks_r8:		read 8 bits
 * ks_r16:		read 16 bits
 * ks_r32:		read 32 bits
 * ks_w8:		write 8 bits
 * ks_w16:		write 16 bits
 * ks_w32:		write 32 bits
 * enc_rbuf:	read from ENC28J60 into buffer
 * enc_wbuf:	write from buffer into ENC28J60
 * omap3_evm_spi_read:	spi read from omap3 spi controller
 */

/*
 * omap3_evm_spi_read
 *
 * Read data 'din_len' from SPI slave register, and store in 'bReadData' buffer
 * by BYTE (8 bit per SPI transfer) through SPI controller.
 */
static int omap3_evm_spi_read
(
    ks_dev_t *ks,
    u8   *dout,
    u8   *din,
	int  dout_len,
	int  din_len
)
{
	spi_xfer(ks->slave, dout_len * 8, dout, NULL, SPI_XFER_BEGIN);
	spi_xfer(ks->slave, din_len * 8, NULL, din, SPI_XFER_END);

	return (0);
}

/*
 * omap3_evm_spi_write
 *
 * Write data 'dout_len' to  SPI slave register,
 * by BYTE (8 bit per SPI transfer) through SPI controller.
 */
static int omap3_evm_spi_write
(
    ks_dev_t *ks,
    u8   *dout,
	int  dout_len
)
{
	spi_xfer(ks->slave, dout_len * 8, dout, NULL, (SPI_XFER_BEGIN | SPI_XFER_END));
	return (0);
}


/*
 * Read a register by byte width
 */
static u8 ks_r8(ks_dev_t *ks, const u8 reg)
{
    u8  bEnable;
    u8  dout[2];
    u8  din;

	bEnable = SPI_BYTE_ACCESS << (reg & 0x03);
	dout[0] = SPI_REG_READ | (bEnable << 2);
	dout[0] |= (reg >> 6);
	dout[1] = (reg << 2) & 0xf0;
    //printf("%s: dout[0]: 0x%02x\n", __func__, dout[0]);
    //printf("%s: dout[1]: 0x%02x\n", __func__, dout[1]);

    /* Issure SPI Register READ cmd to SPI controller */
    omap3_evm_spi_read ( ks, dout, (u8 *)&din, 2, 1 );

	return din;
}

/*
 * Read a register by word width
 */
static u16 ks_r16(ks_dev_t *ks, const u8 reg)
{
    u8  bEnable;
    u8  dout[2];
    u16 din;


	bEnable = SPI_WORD_ACCESS << (reg & 0x02);
	dout[0] = SPI_REG_READ | (bEnable << 2);
	dout[0] |= (reg >> 6);
	dout[1] = (reg << 2) & 0xf0;

    /* Issure SPI Register READ cmd to SPI controller */
    omap3_evm_spi_read ( ks, dout, (u8 *)&din, 2, 2 );

	return din;

}

/*
 * Read a register by dword width
 */
static u32 ks_r32(ks_dev_t *ks, const u8 reg)
{
    u8  dout[2];
    u32 din;


	dout[0] = SPI_REG_READ | (SPI_DWORD_ACCESS << 2);
	dout[0] |= (reg >> 6);
	dout[1] = (reg << 2) & 0xf0;

    /* Issure SPI Register READ cmd to SPI controller */
    omap3_evm_spi_read ( ks, dout, (u8 *)&din, 2, 4 );

	return din;

}

/*
 * Write a value to register by byte width
 */
static void ks_w8(ks_dev_t *ks, const u8 reg, const u8 data)
{
    u8  bEnable;
    u8  dout[3];


	bEnable = SPI_BYTE_ACCESS << (reg & 0x03);
	dout[0] = SPI_REG_WRITE | (bEnable << 2);
	dout[0] |= (reg >> 6);
	dout[1] = (reg << 2) & 0xf0;
    //printf("%s: dout[0]: 0x%02x\n", __func__, dout[0]);
    //printf("%s: dout[1]: 0x%02x\n", __func__, dout[1]);

	dout[2] = data;

    /* Issure SPI Register WRITE cmd to SPI controller */
    omap3_evm_spi_write (ks, dout, 3 );

}

/*
 * Write a value to register by word width
 */
static void ks_w16(ks_dev_t *ks, const u8 reg, const u16 data)
{
    u8  bEnable;
    u8  dout[4];
	u8  *write_data;
	int i;


	bEnable = SPI_WORD_ACCESS << (reg & 0x02);
	dout[0] = SPI_REG_WRITE | (bEnable << 2);
	dout[0] |= (reg >> 6);
	dout[1] = (reg << 2) & 0xf0;

	write_data = (u8 *)&data;
	for (i=0; i<2; i++)
		dout[2+i] = *write_data++;

    /* Issure SPI Register WRITE cmd to SPI controller */
    omap3_evm_spi_write (ks, dout, 4 );
}

/*
 * Write a value to register by dword width
 */
static void ks_w32(ks_dev_t *ks, const u8 reg, const u32 data)
{
    u8  dout[4];
	u8  *write_data;
	int i;


	dout[0] = SPI_REG_WRITE | (SPI_DWORD_ACCESS << 2);
	dout[0] |= (reg >> 6);
	dout[1] = (reg << 2) & 0xf0;

	write_data = (u8 *)&data;
	for (i=0; i<4; i++)
		dout[2+i] = *write_data++;

    /* Issure SPI Register WRITE cmd to SPI controller */
    omap3_evm_spi_write (ks, dout, 6 );
}


/*
 * Read a frame from SPI device
 *
 * Read the received data from hardware packet memory to receive buffer
 * (1). Start transfer.
 * (2). Dummy read out first 2-byte.
 * (3). Dummy read out 4-byte frame header.
 * (4). Continue read packet data.
 * (5). Stop transfer.
 */
static void ks_rbuf(ks_dev_t *ks, volatile void *buf, int len)
{
	u8  *data = (u8 *)buf;
	int length_in_dwordalign;
    u8  dout;
    u8  din[10];


    length_in_dwordalign = ( (len + 3) & ~0x03) >> 2  ;
    length_in_dwordalign *= 4;
    length_in_dwordalign -= 2;

	dout = SPI_RXQ_READ;

	/* KS8851 DMA operation start */
    ks_w16(ks, REG_RX_ADDR_PTR, ADDR_PTR_AUTO_INC );
    ks_w16(ks, REG_RXQ_CMD, (RXQ_CMD_CNTL | RXQ_START));

    /* Receive from SPI controller */
	spi_xfer(ks->slave, (1*8), &dout, NULL, SPI_XFER_BEGIN);
	spi_xfer(ks->slave, (10*8), NULL, din, 0);
	spi_xfer(ks->slave, (length_in_dwordalign*8), NULL, data, SPI_XFER_END);

	/* KS8851 DMA operation stop */
    ks_w16(ks, REG_RXQ_CMD, RXQ_CMD_CNTL);
}


/*
 * Write a frame to SPI device

 * Now write the data out to the Ethernet chip into ks8851 packet memory
 * (1). Start transfer.
 * (2). Write 4-byte frame header.
 * (3). Continue write packet data.
 * (4). Stop transfer.
 */
static void ks_wbuf(ks_dev_t *ks, volatile void *packet, int len)
{
	u8  *data = (u8 *)packet;
	int  length_in_dwordalign;
    u8   dout[5];

    length_in_dwordalign = ( (len + 3) & ~0x03) >> 2  ;
    length_in_dwordalign *=4 ;

	dout[0] = SPI_TXQ_WRITE;
	dout[1] = (u8)(TX_CTRL_INTERRUPT_ON);
	dout[2] = (u8)(TX_CTRL_INTERRUPT_ON >> 8);
	dout[3] = (u8)(len);
	dout[4] = (u8)(len >> 8);

	/* KS8851 DMA operation start */
    ks_w16(ks, REG_RXQ_CMD, (RXQ_CMD_CNTL | RXQ_START));

    /* Send out to SPI controller */
	spi_xfer(ks->slave, (5*8), dout, NULL, SPI_XFER_BEGIN);
	spi_xfer(ks->slave, (length_in_dwordalign*8), data, NULL, SPI_XFER_END);

	/* KS8851 DMA operation stop */
    ks_w16(ks, REG_RXQ_CMD, RXQ_CMD_CNTL);
}

/*---------------------------------------------------------------------*/

static void print_packet(u8 *data, int len, int flag)
{
    int	j = 0, k;

    if  ( (flag < 1)  || (flag > 2) )
        return;

    printf ("%s On port \n", (flag==1) ?  "Tx" : "Rx" );
    printf("Pkt Len=%d \n", len);
    printf("DA=%02x:%02x:%02x:%02x:%02x:%02x \n",
				*data, *(data + 1), *(data + 2), *(data + 3), *(data + 4), *(data + 5));
	printf("SA=%02x:%02x:%02x:%02x:%02x:%02x \n",
				*(data + 6), *(data + 7), *(data + 8), *(data + 9), *(data + 10), *(data + 11));
    do {
        printf("\n %04x   ", j);
        for (k = 0; (k < 16 && len); k++, data++, len--)
            printf("%02x  ", *data);
        j += 16;
    } while (len > 0);
    printf("\n");
}

static int ks8851_receive_status (PMIB_CONFIG  pmib, u16   rx_status )
{
    int    status=0;


    /* Update transmit error statistics. */
    if ( ( rx_status & RX_VALID ) )
    {
        if ( ( rx_status & RX_ERRORS ) )
        {
            status = -1;

            /* Update receive error statistics. */
            pmib->m_cnCounter[ OID_COUNTER_RCV_ERROR ] += 1;

            if ( rx_status & RX_PHY_ERROR )
                pmib->m_cnCounter[ OID_COUNTER_RCV_ERROR_MII ] += 1;

            if ( rx_status & RX_RUNT_ERROR )
                pmib->m_cnCounter[ OID_COUNTER_RCV_ERROR_RUNT ] += 1;

            if ( rx_status & RX_BAD_CRC )
                pmib->m_cnCounter[ OID_COUNTER_RCV_ERROR_CRC ] += 1;

            if ( rx_status & RX_TOO_LONG )
                pmib->m_cnCounter[ OID_COUNTER_RCV_ERROR_TOOLONG ] += 1;

            if ( rx_status & RX_ICMP_ERROR )
                pmib->m_cnCounter[ OID_COUNTER_RCV_ERROR_ICMP ] += 1;

            if ( rx_status & RX_IP_ERROR )
                pmib->m_cnCounter[ OID_COUNTER_RCV_ERROR_IP ] += 1;

            if ( rx_status & RX_TCP_ERROR )
                pmib->m_cnCounter[ OID_COUNTER_RCV_ERROR_TCP ] += 1;

            if ( rx_status & RX_UDP_ERROR )
                pmib->m_cnCounter[ OID_COUNTER_RCV_ERROR_UDP ] += 1;
        }

        /*
         *  Receive without error.
         */
        else
        {
            if  ( ( rx_status & RX_MULTICAST ) || ( rx_status & RX_BROADCAST ) )
            {
                if ( rx_status & RX_BROADCAST )
                    pmib->m_cnCounter[ OID_COUNTER_BROADCAST_FRAMES_RCV ] += 1;
                else
                    pmib->m_cnCounter[ OID_COUNTER_MULTICAST_FRAMES_RCV ] += 1;
            }
            if ( rx_status & RX_UNICAST )
                pmib->m_cnCounter[ OID_COUNTER_UNICAST_FRAMES_RCV ] += 1;
        }
    }
    else
    {
        status = 0;
        pmib->m_cnCounter[ OID_COUNTER_RCV_INVALID_FRAME ] += 1;
    }

    return(status);			/* Indicate we got good packet or not */

}  /* ks8851_receive_status */


static void ks8851_reset (struct eth_device *dev)
{
    ks_dev_t *ks = dev->priv;

    /* Write 1 to reset device (GRR) */
    ks_w16(ks, REG_RESET_CTRL, GLOBAL_SOFTWARE_RESET );
    udelay (20*1000);

    /* Write 0 to clear device reset */
    ks_w16(ks, REG_RESET_CTRL, 0 );
    udelay (20*1000);

}


static int ks8851_selftest (struct eth_device *dev)
{
    ks_dev_t *ks = dev->priv;


    u16 reg_data;
    int i;
	int status=0;
	int max_count=0xffff;


    /* test chip QMU read\write */
	printf( "%s: SPI interface selftest ", ks->dev->name);
	status = 0;
    for ( i=0; i<max_count; i++ ) {
        ks_w16(ks, REG_MAC_ADDR_0, i );
        reg_data = ks_r16(ks, REG_MAC_ADDR_0);
        if ( reg_data != i ) {
			status = -1;
            printf( "\n register MARL read data value %x is not equal to write data %x\n",
                        reg_data, i );
            break;
        }
    }
	printf( "%s.\n", (status==0) ? "OK": "FAIL");

	return ( status );
}

static void ks8851_phy_configure(struct eth_device *dev)
{
    ks_dev_t *ks = dev->priv;
    u16   reg_data;
    u16   link_status;
	int   timeout=5000;

    /* Restart auto-negotiation, set to 100BT/half duplex if auto-nego fail */
    reg_data = ks_r16(ks, REG_PORT_CTRL);
    reg_data |= (PORT_AUTO_NEG_ENABLE | PORT_AUTO_NEG_RESTART);
    reg_data &= ~(PORT_FORCE_FULL_DUPLEX);
    ks_w16(ks, REG_PORT_CTRL, reg_data );
    do {
		mdelay(1);
		if ((timeout--) == 0)
			goto err_out;
        link_status = ks_r16(ks, REG_PORT_STATUS) & PORT_STATUS_LINK_GOOD;
	} while ( --timeout && (link_status != PORT_STATUS_LINK_GOOD));

    return;

err_out:
	printf("%s: autonegotiation timed out\n", ks->dev->name);

}


/*
    ks8851_read_mib_count

    Description:
        This routine reads a MIB counter of the port.

    Parameters:
        USHORT wAddr
            The address of the counter.

        PULONGLONG pqData
            Buffer to store the counter.

    Return (None):
*/

static void ks8851_read_mib_count
(
    ks_dev_t *ks,
    u16       wAddr,
    u32      *pqData
)
{
    u32    dwData;
    u16    wCtrlAddr;


    wCtrlAddr = ( TABLE_MIB | TABLE_READ | (wAddr & TABLE_ENTRY_MASK) );
    ks_w16( ks, REG_IND_IACR, wCtrlAddr );

    dwData = ks_r32(ks, REG_IND_DATA_LOW );
    *pqData += dwData;

}  /* ks8851_read_mib_count */


/*
    ks8851_read_mib

    Description:
        This routine is used to read the counters of the port periodically to
        avoid counter overflow.  The hardware should be acquired first before
        calling this routine.

    Parameters:
        PMIB_CONFIG  pMib;
            Pointer to MIB counter struct.

    Return (None):
*/

static void ks8851_read_mib(ks_dev_t *ks)
{
    PMIB_CONFIG  pMib = ks->pmib;
    int i=0;

    while ( i < PORT_COUNTER_NUM ) {
        ks8851_read_mib_count(ks, i, &pMib->cnCounter[ i ]);
        i++;
    }
}  /* ks8851_read_mib */

/*
    ks8851_clear_mib

    Description:
        This routine is used to clear all counters to zero.

    Parameters:
        PMIB_CONFIG  pMib;
            Pointer to MIB counter struct.

    Return (None):
*/

static void ks8851_clear_mib(ks_dev_t *ks)
{
    PMIB_CONFIG  pMib = ks->pmib;

    int i=0;

    /* Clear device MIB counter by reading them out */
    do
    {
        ks8851_read_mib_count(ks, i, &pMib->cnCounter[ i ]);
        i++;
    } while ( i < PORT_COUNTER_NUM );

    memset(( void* ) &pMib->cnCounter[0], 0,
           (sizeof( u32 ) * PORT_COUNTER_NUM ) );

    /* Clear driver statistics counter */
    memset(( void* ) &pMib->m_cnCounter[0], 0,
           (sizeof( u32 ) * OID_COUNTER_LAST) );

}  /* ks8851_clear_mib */


/*****************************************************************************
*
* ks8851_set_mac_address - Set ks8851 Ethernet (IEEE station) address
*
* This routine provides an interface for accessing an ks8851 device
* Ethernet (IEEE station address) address in the device.
* The 6-byte IEEE station address, specified by the <env_enetaddr> parameter, 
* will be wrote to ks8851 device MAC address registers .
*
*
* RETURNS: OK, or ERROR if could not be obtained.
*
* NOMANUAL
*/
static int ks8851_set_mac_address(struct eth_device *dev, bd_t * bd)
{
    ks_dev_t *ks = dev->priv;
	uchar *m = dev->enetaddr;
    uchar i;
    int   status=0;
    uchar ksMacAddress[6];


	if ( *(( int* ) dev->enetaddr ) == 0 ) {
		memcpy( dev->enetaddr, default_eth_mac, 6 );
	    m = dev->enetaddr;
    }

	/* Set MAC address into device MAC registers */
    for ( i=0; i<MAC_ADDRESS_LENGTH; i++ )
        ks_w8(ks, (u8)( REG_MAC_ADDR_0 + i ), m[MAC_ADDR_ORDER(i)]);

	/* Get MAC address from the device MAC registers */
    for ( i=0; i<MAC_ADDRESS_LENGTH; i++ )
        ksMacAddress[i] = ks_r8(ks, (u8)( REG_MAC_ADDR_0 + i ));

    for (i=0; i<MAC_ADDRESS_LENGTH; i++) {
        if (ksMacAddress[i] != m[MAC_ADDR_ORDER(i)] ) {
           status = -1;
		   break;
        }
    }

    printf("%s MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", ks->dev->name,
                ksMacAddress[5], ksMacAddress[4],
                ksMacAddress[3], ksMacAddress[2],
                ksMacAddress[1], ksMacAddress[0] );

    return (status);
}


static int ks8851_init (struct eth_device *dev, bd_t * bd)
{
    ks_dev_t *ks = dev->priv;
    u16   reg_data;



    /* Initialize KS8851 only once */

    /* Reset ksz8851 */
    ks8851_reset (dev);

    /* ksz8851 test SPI interface */
    if (ks8851_selftest(dev) == -1)
	     return (-1);	/* SPI interface not stable  */

    /* check Chip ID */
    reg_data = ks_r16(ks, REG_CHIP_ID);
    if ( (reg_data & CHIP_ID_MASK) != CHIP_ID_8851_16) {
	     printf("%s: CHIP ID (%04x) error...\n", ks->dev->name, reg_data);
	     return (-1);	/* chip error, Stop here  */
    }


    /* Set ks8851 station MAC address */
    if ( ks8851_set_mac_address( dev, bd ) == -1 ) {
        printf("%s: fail to set MAC address to device. \n", ks->dev->name);
        return (-1);	/* chip error, Stop here  */
    }

    /*
     * Configure QMU Transmit
     */

    /* Setup Transmit Frame Data Pointer Auto-Increment (TXFDPR) */
    ks_w16(ks,  REG_TX_ADDR_PTR, ADDR_PTR_AUTO_INC );

    /* Setup QMU Transmit (TXCR). */
    ks_w16(ks,  REG_TX_CTRL, DEFAULT_TX_CTRL );

    /*
     * Configure QMU Receive
     */

    ks_w16(ks,  REG_RX_HIGH_WATERMARK, 0x400);  /* High water mark to 4KB */
    ks_w16(ks,  REG_RX_LOW_WATERMARK, 0x600);  /* Low water mark to 6KB */

    /* Setup Receive Frame Data Pointer Auto-Increment (RXFDPR) */
    ks_w16(ks,  REG_RX_ADDR_PTR, ADDR_PTR_AUTO_INC );

    /* Setup Receive Frame Threshold - 1 frame (RXFCTFC) */
    ks_w16(ks,  REG_RX_FRAME_CNT_THRES, ( 1 & RX_FRAME_THRESHOLD_MASK) );

    /* Setup RxQ Command Control (RXQCR) */
    ks_w16(ks, REG_RXQ_CMD, RXQ_CMD_CNTL );

    /* Setup QMU Receive (RXCR1). */
    ks_w16(ks, REG_RX_CTRL1, UNICAST_RX_CTRL1 );

    /* Setup Receive Control 2 (RXCR2) */
    ks_w16(ks, REG_RX_CTRL2, (DEFAULT_RX_CTRL2 | RX_SPI_BURST_FRAME));

    /* Clear ISR */
    ks_w16(ks, REG_INT_STATUS, 0xffff);

    /* Initialize the PHY */
    ks8851_phy_configure(dev);

    /* Clear MIB's Counters */
    if ( !ks->pmib ) {
        printf("%s: ks->mib is NULL. \n", ks->dev->name);
        return (-1);
    }

    ks8851_clear_mib( ks );

    return (0);
}


/*---------------------------------------------------------------------*/

static int ks8851snl_init(struct eth_device *dev, bd_t * bd)
{
    ks_dev_t *ks = dev->priv;

	printf("%s: detected controller\n", DRIVERNAME);

    /* Initialize KS8851 only once */
    if ( !ks->initstate )  {

       if ( ks8851_init(dev, bd) == 0 )
          ks->initstate = 1;
       else
          return (-1);
    }

    if ( ks->initstate ) {

       /* Clear the interrupts status of the hardware. */
       ks_w16(ks, REG_INT_STATUS, 0xFFFF );

       /* Enables QMU Transmit (TXCR). */
       ks_w16(ks, REG_TX_CTRL, (DEFAULT_TX_CTRL | TX_CTRL_ENABLE));

       /* Enables QMU Receive (RXCR1). */
       ks_w16(ks, REG_RX_CTRL1, (UNICAST_RX_CTRL1 | RX_CTRL_ENABLE));

   }
   return (0);

}

static void ks8851snl_halt(struct eth_device *dev)
{
    ks_dev_t *ks = dev->priv;

    if ( ks->initstate ) {
        /* Disables QMU Transmit (TXCR). */
        ks_w16(ks, REG_TX_CTRL, DEFAULT_TX_CTRL );

        /* Disables QMU Receive (RXCR1). */
        ks_w16(ks, REG_RX_CTRL1, UNICAST_RX_CTRL1 );
    }
}


static int ks8851snl_send(struct eth_device *dev,
			volatile void *packet, int length)
{
    ks_dev_t   *ks = dev->priv;
	u8         *data = (u8*)packet;
    PMIB_CONFIG pmib = ks->pmib;
	u16         wstatus;


    /* Checking ks8851 TXQ memory available for this packet */
    wstatus = ks_r16(ks, REG_TX_MEM_INFO);
    if ( (wstatus & TX_MEM_AVAILABLE_MASK) < (length + 4) ) {
        /* Update transmit statistics. */
        pmib->m_cnCounter[ OID_COUNTER_XMIT_ALLOC_FAIL ] += 1;
        wstatus = ks_r16(ks, REG_TX_MEM_INFO);
        printf("%s: device TXQ memory not available! wstatus=%04x\n",
                 ks->dev->name, wstatus);
        return -1;
    }

    /* Write a frame to SPI device */
    ks_wbuf(ks, packet, length);

    /* Issue TXQ Command (Enqueue Tx frame from TX buffer into TXQ for transmit) */
    ks_w16(ks, REG_TXQ_CMD, TXQ_ENQUEUE );

    /* Update driver transmit counters (include CRC) */
    pmib->m_cnCounter[ OID_COUNTER_DIRECTED_FRAMES_XMIT ]++;
    pmib->m_cnCounter[ OID_COUNTER_DIRECTED_BYTES_XMIT ] += length+4;

    /* dump transmit data to the console port for debug */
    if ( ks->fdump_tx ) {
        /* for debug use */
        printf("\nTx Cntl Word: 0x%04x, Byte Count: 0x%04x \n",
                    (TX_CTRL_INTERRUPT_ON), (length) );
        print_packet( data, length, 1 );
    }

	return 0;
}

static int ks8851snl_rx(struct eth_device *dev)
{
    ks_dev_t *ks = dev->priv;
	u8       *packet = (u8 *)NetRxPackets[0];
    PMIB_CONFIG pmib = ks->pmib;
    u16       int_status;
    u16       rx_status;
    u16       rx_length;
    u16       reg_data;
    int       packet_len = 0;


    /* Read the device interrupt status register */
    int_status = ks_r16(ks, REG_INT_STATUS);

    if( int_status & INT_RX_OVERRUN_ETH)
    {
        /* Clear the interrupt INT_RX_STOPPED status */
        ks_w16(ks, REG_INT_STATUS, INT_RX_OVERRUN_ETH );
        pmib->m_cnCounter[ OID_COUNTER_RCV_INT_OVERRUN ]++;
    }

    if ( ks->rx_frame_count == 0)
    {
        /* Receive the completed packet. */
        if ( ( int_status & INT_RX_ETH ) )
        {
            /* Acknowledge the RX interrupt. */
            ks_w16(ks, REG_INT_STATUS, INT_RX_ETH );
            pmib->m_cnCounter[ OID_COUNTER_RCV_INT ]++;

            /* Get current total amount of received frame count */
            reg_data = ks_r16(ks, REG_RX_FRAME_CNT_THRES);
            ks->rx_frame_count = (reg_data >> 8) & 0xff;
         }
    }

    if ( ks->rx_frame_count > 0)
    {
        /* Checking Received packet status */
        rx_status = ks_r16(ks, REG_RX_FHR_STATUS);

        /* Get received packet length from hardware packet memory */
        rx_length = ks_r16(ks, REG_RX_FHR_BYTE_CNT);

        /* Update receive statistic if receive valid frame */
        if ( ks8851_receive_status (pmib, rx_status) == 0) {

            /* The packet length is included 4-byte CRC */
            packet_len = (rx_length & RX_BYTE_CNT_MASK);

            /* Read a frame from SPI device */
            ks_rbuf(ks, packet, packet_len);

            /*
             * Because "Receive IP Header Two-Byte Offset" feature is enabled
             * which will make IP header is in DWORD aligment in the memory address.
             * there are two extra bytes before the valid frame data, and
             * two extra bytes count is included in the frame header 'Byte Count'.
             * Also, 'Byte Count' also contains 4-byte CRC length.
             * Pass the packet up to the protocol layers without CRC.
             */
            packet_len -= 6;
            NetReceive (packet, packet_len);

            /* Update receive statistics. */
            pmib->m_cnCounter[ OID_COUNTER_DIRECTED_FRAMES_RCV ]++;
            pmib->m_cnCounter[ OID_COUNTER_DIRECTED_BYTES_RCV ] += packet_len;

            /* dump received data to the console port for debug */
            if ( ks->fdump_rx ) {
                printf("\nRx Status Word: 0x%04x, Byte Count: 0x%04x\n",
                         rx_status, rx_length );
                if ( packet_len > 0 )
                    print_packet( (u8 *)packet, packet_len, 2 );
            }
        }
        else {
            /* Manual DeQueue this error frame (free up this frame memory) */
            ks_w16(ks, REG_RXQ_CMD, (RXQ_CMD_CNTL | RXQ_CMD_FREE_PACKET));

            /*  received Packet with errors  */
            printf("%s: Receive error packet! rx_status=0x%04x, rx_length=0x%04x (%d)\n",
                     ks->dev->name, rx_status, rx_length, rx_length );

        } /* if ( ks8851_receive_status (pmib, rx_status) == 0) */

        ks->rx_frame_count--;

    } /* if ( ks->rx_frame_count ) { */

	return 0;
}

int ks8851snl_initialize(unsigned int bus, unsigned int cs,
	                     unsigned int max_hz, unsigned int spi_mode)
{
	struct eth_device *dev;
	ks_dev_t *ks;
    PMIB_CONFIG  pmib;

	int rc;


	dev = malloc(sizeof(*dev));
	if (!dev) {
		return -1;
	}
	memset(dev, 0, sizeof(*dev));

	/*dev->iobase = base_addr; */

	/* try to allocate, check and clear ks_dev_t object */
	ks = malloc(sizeof(*ks));
	if (!ks) {
		free(dev);
		return -1;
	}
	memset(ks, 0, sizeof(*ks));

	pmib = malloc(sizeof(*pmib));
	if (!pmib) {
		free(dev);
		return -1;
	}
	memset(pmib, 0, sizeof(*pmib));
	ks->pmib = pmib;

	/* try to setup the SPI slave */
	ks->slave = spi_setup_slave(bus, cs, max_hz, spi_mode);

	if (!ks->slave) {
		printf("%s: invalid SPI device %i:%i\n", DRIVERNAME, bus, cs);
		free(ks);
		free(dev);
		return 0;
	}
	rc = spi_claim_bus(ks->slave);
	if (rc)
		printf("%s: failed to claim SPI bus\n", DRIVERNAME);

	ks->dev = dev;

	/* now fill the eth_device object */
	dev->priv = ks;
	dev->init = ks8851snl_init;
	dev->halt = ks8851snl_halt;
	dev->send = ks8851snl_send;
	dev->recv = ks8851snl_rx;
	sprintf(dev->name, "%s-%i.%i", DRIVERNAME, bus, cs);

	eth_register(dev);

	ks->initstate = 0;
    ks->rx_frame_count = 0;
    ks->fdump_tx = 0;
    ks->fdump_rx = 0;

	/* record it for debugging */
    g_dev = dev;

	return 1;
}

/*------------------------------------------------------------------------*/
/*
 * export functions for debug KSZ8851SNL driver
 */

/*****************************************************************
*
* Command: ksr
*
* Format: "ksr  RegAddr Width"
*
******************************************************************/
void ksread
(
   ulong   RegAddr,
   ulong   Width
)
{
   ks_dev_t *ks;

   if (g_dev == NULL) {
	   printf("g_dev is NULL\n");
	   return;
   }

   ks = g_dev->priv;

   switch (Width)
   {
        case 1: /* read by BYTE unit */
            {
            u8 reg_data = ks_r8(ks, (const u8) RegAddr);
            printf( "%02x\n", reg_data);
            }
            break;
        case 4: /* read by DWORD unit */
            {
            u32 reg_data = ks_r32(ks, (const u8) RegAddr);
            printf( "%08x\n", reg_data);
            }
            break;
        case 2: /* read by WORD unit */
        default:
            {
            u16 reg_data = ks_r16(ks, (const u8) RegAddr);
            printf( "%04x\n", reg_data);
            }
            break;
   }

}

/*****************************************************************
*
* Command: ksw
*
* Format: "ksw RegAddr RegData Width"
*
******************************************************************/
void kswrite
(
   ulong  RegAddr,
   ulong  RegData,
   ulong  Width
)
{
   ks_dev_t *ks;

   if (g_dev == NULL) {
	   printf("g_dev is NULL\n");
	   return;
   }

   ks = g_dev->priv;

   switch (Width)
   {
        case 1: /* write by BYTE unit */
            {
            u8  WriteRegData = (u8)RegData;
            ks_w8(ks, (const u8)RegAddr, (const u8)WriteRegData);
            }
            break;
        case 4: /* write by DWORD unit */
            {
            u32  WriteRegData = (u32)RegData;
            ks_w32(ks, (const u8)RegAddr, (const u32)WriteRegData);
            }
            break;
        case 2: /* write by WORD unit */
        default:
            {
            u16  WriteRegData = (u16)RegData;
            ks_w16(ks, (const u8)RegAddr, (const u16)WriteRegData);
            }
            break;
   }
   printf("done\n");
}

/*****************************************************************
*
* Command: ksdumprx
*
* Format: "ksdumprx  DumpFlag"
*
******************************************************************/
void ksdumprx( u32 DumpFlag )
{
   ks_dev_t *ks;

   if (g_dev == NULL) {
	   printf("g_dev is NULL\n");
	   return;
   }

   ks = g_dev->priv;

   ks->fdump_rx = (int) DumpFlag;
   printf( "%s dumpping receive packets.\n\n", (DumpFlag) ? "Start" : "Stop" );
}

/*****************************************************************
*
* Command: ksdumptx
*
* Format: "ksdumptx  DumpFlag"
*
******************************************************************/
void ksdumptx( u32 DumpFlag )
{
   ks_dev_t *ks;

   if (g_dev == NULL) {
	   printf("g_dev is NULL\n");
	   return;
   }

   ks = g_dev->priv;

   ks->fdump_tx = (int) DumpFlag;
   printf( "%s dumpping trasnmit packets.\n\n", (DumpFlag) ? "Start" : "Stop" );
}

/*****************************************************************
*
* Command: ksspi
*
* Format: "ksspi bus, cs max_hz spi_mode"
*
******************************************************************/
void ksspi
(
    ulong  bus,
    ulong  cs,
    ulong  max_hz,
    ulong  spi_mode
)
{
    ks_dev_t *ks;
	int rc;

    if (g_dev == NULL) {
	    printf("g_dev is NULL\n");
	    return;
    }

    ks = g_dev->priv;

	if (ks->slave)
	    spi_release_bus(ks->slave);

	if (ks->slave)
        spi_free_slave(ks->slave);

	printf("%s: bus=%d, cs=%d, max_hz=%d, spi_mode=%d \n", __func__, bus, cs, max_hz, spi_mode);

    ks->slave = spi_setup_slave(bus, cs, max_hz, spi_mode);
    if (!ks->slave) {
		printf("%s: invalid SPI device %i:%i\n", DRIVERNAME, bus, cs);
    }

	if (ks->slave)
	   rc = spi_claim_bus(ks->slave);
	if (rc)
		printf("%s: failed to claim SPI bus\n", ks->dev->name);

}

void ksclearmib()
{
    ks_dev_t *ks;

    if (g_dev == NULL) {
	    printf("g_dev is NULL\n");
	    return;
    }

    ks = g_dev->priv;
    ks8851_clear_mib( ks );
}

static void showMibTable
(
     PMIB_CONFIG  pMib
)
{
     printf( "\n                        MIB        Driver\n");

     printf( "RxByte               : %08ld    %08ld    RxByte\n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_BYTE ],
          (long) pMib->m_cnCounter[ OID_COUNTER_DIRECTED_BYTES_RCV ] ) ;

     printf( "RxUndersizePkt       : %08ld    %08ld    RxBroadcastPkts \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_UNDERSIZE ],
          (long) pMib->m_cnCounter[ OID_COUNTER_BROADCAST_FRAMES_RCV ] ) ;

     printf( "RxFragments          : %08ld    %08ld    RxMulticastPkts \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_FRAGMENT ],
          (long) pMib->m_cnCounter[ OID_COUNTER_MULTICAST_FRAMES_RCV ] ) ;

     printf( "RxOversize           : %08ld    %08ld    RxUnicastPkts \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_OVERSIZE ],
          (long) pMib->m_cnCounter[ OID_COUNTER_UNICAST_FRAMES_RCV ] ) ;

     printf( "RxJabbers            : %08ld    %08ld    RxTotalPkts \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_JABBER ],
          (long) pMib->m_cnCounter[ OID_COUNTER_DIRECTED_FRAMES_RCV ] ) ;

     printf( "RxSymbolError        : %08ld    %08ld    RxCRCError \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_SYMBOL_ERR ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_ERROR_CRC ] ) ;

     printf( "RxCRCError           : %08ld    %08ld    RxMIIError \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_CRC_ERR ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_ERROR_MII ] ) ;

     printf( "RxAlignmentError     : %08ld    %08ld    RxTooLong \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_ALIGNMENT_ERR ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_ERROR_TOOLONG ] ) ;

     printf( "RxCtrl8808Pkts       : %08ld    %08ld    RxRuntFrame \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_CTRL_8808 ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_ERROR_RUNT ] ) ;

     printf( "RxPausePkts          : %08ld    %08ld    RxInvalidFrame \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_PAUSE ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_INVALID_FRAME ] ) ;

     printf( "RxBroadcastPkts      : %08ld    %08ld    RxICMPChecksumError \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_BROADCAST ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_ERROR_ICMP ] ) ;

     printf( "RxMulticastPkts      : %08ld    %08ld    RxIPChecksumError \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_MULTICAST ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_ERROR_IP ] ) ;

     printf( "RxUnicastPkts        : %08ld    %08ld    RxTCPChecksumError \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_UNICAST ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_ERROR_TCP ] ) ;

     printf( "Rx64Octets           : %08ld    %08ld    RxUDPChecksumError \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_OCTET_64 ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_ERROR_UDP ] ) ;

     printf( "Rx65to127Octets      : %08ld    %08ld    RxTotalError \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_OCTET_65_127 ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_ERROR ] ) ;

     printf( "Rx128to255Octets     : %08ld    %08ld    RxOSDroppedPkts \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_OCTET_128_255 ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_DROPPED ] ) ;

     printf( "Rx256to511Octets     : %08ld    %08ld    RxOverrunInterrupt \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_OCTET_256_511 ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_INT_OVERRUN ] ) ;

     printf( "Rx512to1023Octets    : %08ld    %08ld    RxStopInterrupt \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_OCTET_512_1023 ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_INT_STOP ] ) ;

     printf( "Rx1024to1521ctets    : %08ld    %08ld    RxInterrupt \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_OCTET_1024_1521 ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_INT ] ) ;

     printf( "Rx1522to2000Octets   : %08ld    %08ld    RxWakeupFrame \n",
          (long) pMib->cnCounter[ MIB_COUNTER_RX_OCTET_1522_2000 ],
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_INT_WOL ] ) ;

     printf( "                                   %08ld    RxMagicPacket \n",
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_INT_MAGIC ] ) ;

     printf( "                                   %08ld    RxWOLLinkup \n",
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_INT_LINKUP ] ) ;

     printf( "                                   %08ld    RxWOLEnergy \n",
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_INT_ENERGY ] ) ;

     printf( "                                   %08ld    RxWOLDelayEnergy \n",
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_INT_ENERGY_DELAY ] ) ;

     printf( "                                   %08ld    RxErrorLen \n",
          (long) pMib->m_cnCounter[ OID_COUNTER_RCV_ERROR_LEN ] ) ;

     printf( "\n");

     printf( "TxByte               : %08ld    %08ld    TxByte \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_BYTE ],
          (long) pMib->m_cnCounter[ OID_COUNTER_DIRECTED_BYTES_XMIT ] ) ;

     printf( "TxPausePkts          : %08ld    %08ld    TxTotalPkts \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_PAUSE ],
          (long) pMib->m_cnCounter[ OID_COUNTER_DIRECTED_FRAMES_XMIT ] ) ;

     printf( "TxLateCollision      : %08ld    %08ld    TxLateCollision \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_LATE_COLLISION ],
          (long) pMib->m_cnCounter[ OID_COUNTER_XMIT_LATE_COLLISION ] ) ;

     printf( "TxBroadcastPkts      : %08ld    %08ld    TxMaximumCollision \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_BROADCAST ],
          (long) pMib->m_cnCounter[ OID_COUNTER_XMIT_MORE_COLLISIONS ] ) ;

     printf( "TxMulticastPkts      : %08ld    %08ld    TxTotalError \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_MULTICAST ],
          (long) pMib->m_cnCounter[ OID_COUNTER_XMIT_ERROR ] ) ;

     printf( "TxUnicastPkts        : %08ld    %08ld    TxAllocMemFail \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_UNICAST ],
          (long) pMib->m_cnCounter[ OID_COUNTER_XMIT_ALLOC_FAIL ] ) ;

     printf( "TxDeferred           : %08ld    %08ld    TxOSDroppedPkts \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_DEFERRED ],
          (long) pMib->m_cnCounter[ OID_COUNTER_XMIT_DROPPED ] ) ;

     printf( "TxTotalCollision     : %08ld    %08ld    TxSpaceInterrupt \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_TOTAL_COLLISION ],
          (long) pMib->m_cnCounter[ OID_COUNTER_XMIT_SPACE_INT ] ) ;

     printf( "TxExcessiveCollision : %08ld    %08ld    TxStopInterrupt \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_EXCESS_COLLISION ],
          (long) pMib->m_cnCounter[ OID_COUNTER_XMIT_INT_STOP ] ) ;

     printf( "TxSingleCollision    : %08ld    %08ld    TxDoneInterrupt \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_SINGLE_COLLISION ],
          (long) pMib->m_cnCounter[ OID_COUNTER_XMIT_INT ] ) ;

     printf( "TxMultiCollision     : %08ld    %08ld    PhyInterrupt \n",
          (long) pMib->cnCounter[ MIB_COUNTER_TX_MULTI_COLLISION ],
          (long) pMib->m_cnCounter[ OID_COUNTER_PHY_INT ] ) ;
     printf( "\n");

}

void ksshowmib()
{
    ks_dev_t *ks;

    if (g_dev == NULL) {
	    printf("g_dev is NULL\n");
	    return;
    }

    ks = g_dev->priv;
    ks8851_read_mib( ks );
    showMibTable ( ks->pmib );
}

/*------------------------------------------------------------------------*/
