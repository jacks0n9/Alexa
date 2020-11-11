/*
 * Basic I2C functions
 *
 * Copyright (c) 2004 Texas Instruments
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author: Jian Zhang jzhang@ti.com, Texas Instruments
 *
 * Copyright (c) 2003 Wolfgang Denk, wd@denx.de
 * Rewritten to fit into the current U-Boot framework
 *
 * Adapted for OMAP2420 I2C, r-woodruff2@ti.com
 *
 */



#include <stdio.h>
#include <bios.h>
#include <clk.h>
#include <tsk.h>
#include "I2c.h"

#define readw(a)    (*(volatile unsigned short *)(a))
#define writew(v,a) (*(volatile unsigned short *)(a) = (v))
#define readb(a)    (*(volatile unsigned char *)(a))
#define writeb(v,a) (*(volatile unsigned char *)(a) = (v))

unsigned int i2c_base = I2C_DEFAULT_BASE;

unsigned int I2C_REV	   = (I2C_DEFAULT_BASE + 0x00);
unsigned int I2C_IE 	   = (I2C_DEFAULT_BASE + 0x04);
unsigned int I2C_STAT      = (I2C_DEFAULT_BASE + 0x08);
unsigned int I2C_IV 	   = (I2C_DEFAULT_BASE + 0x0c);
unsigned int I2C_BUF	   = (I2C_DEFAULT_BASE + 0x14);
unsigned int I2C_CNT	   = (I2C_DEFAULT_BASE + 0x18);
unsigned int I2C_DATA	   = (I2C_DEFAULT_BASE + 0x1c);
unsigned int I2C_SYSC	   = (I2C_DEFAULT_BASE + 0x20);
unsigned int I2C_CON	   = (I2C_DEFAULT_BASE + 0x24);
unsigned int I2C_OA 	   = (I2C_DEFAULT_BASE + 0x28);
unsigned int I2C_SA 	   = (I2C_DEFAULT_BASE + 0x2c);
unsigned int I2C_PSC	   = (I2C_DEFAULT_BASE + 0x30);
unsigned int I2C_SCLL	   = (I2C_DEFAULT_BASE + 0x34);
unsigned int I2C_SCLH	   = (I2C_DEFAULT_BASE + 0x38);
unsigned int I2C_SYSTEST   = (I2C_DEFAULT_BASE + 0x3c);

static void wait_for_bb (void);
static Uint16 wait_for_pin (void);
static void flush_fifo(void);



static unsigned int bus_initialized[I2C_BUS_MAX];
static unsigned int current_bus;

void my_TSK_sleep(int a)
{
	while (a > 0)
		a--;
}

void i2c_init (int speed, int slaveadd)
{
	int psc, fsscll, fssclh;
	int hsscll = 0, hssclh = 0;
	Uint32 scll, sclh;

	/* Only handle standard, fast and high speeds */
	if ((speed != OMAP_I2C_STANDARD) &&
	    (speed != OMAP_I2C_FAST_MODE) &&
	    (speed != OMAP_I2C_HIGH_SPEED)) {
		printf("Error : I2C unsupported speed %d\n", speed);
		return;
	}

	psc = I2C_IP_CLK / I2C_INTERNAL_SAMPLING_CLK;
	psc -= 1;
//	if (psc < I2C_PSC_MIN) {
//		printf("Error : I2C unsupported prescalar %d\n", psc);
//		return;
//	}

	if (speed == OMAP_I2C_HIGH_SPEED) {
		/* High speed */

		/* For first phase of HS mode */
		fsscll = fssclh = I2C_INTERNAL_SAMPLING_CLK /
			(2 * OMAP_I2C_FAST_MODE);

		fsscll -= I2C_HIGHSPEED_PHASE_ONE_SCLL_TRIM;
		fssclh -= I2C_HIGHSPEED_PHASE_ONE_SCLH_TRIM;
//		if (((fsscll < 0) || (fssclh < 0)) ||
//		    ((fsscll > 255) || (fssclh > 255))) {
//			printf("Error : I2C initializing first phase clock\n");
//			return;
//		}

		/* For second phase of HS mode */
		hsscll = hssclh = I2C_INTERNAL_SAMPLING_CLK / (2 * speed);

		hsscll -= I2C_HIGHSPEED_PHASE_TWO_SCLL_TRIM;
		hssclh -= I2C_HIGHSPEED_PHASE_TWO_SCLH_TRIM;
//		if (((fsscll < 0) || (fssclh < 0)) ||
//		    ((fsscll > 255) || (fssclh > 255))) {
//			printf("Error : I2C initializing second phase clock\n");
//			return;
//		}

		scll = (unsigned int)hsscll << 8 | (unsigned int)fsscll;
		sclh = (unsigned int)hssclh << 8 | (unsigned int)fssclh;

	} else {
		/* Standard and fast speed */
		fsscll = fssclh = I2C_INTERNAL_SAMPLING_CLK / (2 * speed);

		fsscll -= I2C_FASTSPEED_SCLL_TRIM;
		fssclh -= I2C_FASTSPEED_SCLH_TRIM;
//		if (((fsscll < 0) || (fssclh < 0)) ||
//		    ((fsscll > 255) || (fssclh > 255))) {
//			printf("Error : I2C initializing clock\n");
//			return;
//		}

		scll = (unsigned int)fsscll;
		sclh = (unsigned int)fssclh;
	}




	bus_initialized[current_bus] = 1;
	if (readw (I2C_CON) & I2C_CON_EN) {
		writew (0, I2C_CON);
		my_TSK_sleep (500000);
	}

	writew(psc, I2C_PSC);
	writew(scll, I2C_SCLL);
	writew(sclh, I2C_SCLH);

	/* own address */
	writew (slaveadd, I2C_OA);
	writew (I2C_CON_EN, I2C_CON);

	/* have to enable intrrupts or OMAP i2c module doesn't work */
	writew (I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE |
		I2C_IE_NACK_IE | I2C_IE_AL_IE, I2C_IE);
	my_TSK_sleep (10000);
	flush_fifo();
	writew (0xFFFF, I2C_STAT);
	writew (0, I2C_CNT);
}

static int i2c_read_byte (Uint8  devaddr, Uint8  regoffset, Uint8  * value)
{
        int i2c_error = 0;
        Uint16 status;

        /* wait until bus not busy */
        wait_for_bb ();

        /* one byte only */
        writew (1, I2C_CNT);
        /* set slave address */
        writew (devaddr, I2C_SA);
        /* no stop bit needed here */
        writew (I2C_CON_EN | I2C_CON_MST | I2C_CON_TRX | I2C_CON_STT, I2C_CON);
        status = wait_for_pin ();

        if (status & I2C_STAT_XRDY) {
                /* Important: have to use byte access */
                writew (regoffset, I2C_DATA);
                my_TSK_sleep (20000);
                if (readw (I2C_STAT) & I2C_STAT_NACK) {
                        i2c_error = 1;
                }
        } else {
                i2c_error = 1;
        }

        if (!i2c_error) {
                /* free bus, otherwise we can't use a combined transction */
                writew (I2C_CON_EN, I2C_CON);
                while (readw (I2C_STAT) & ((I2C_STAT_RRDY) | (I2C_STAT_ARDY))) {
                        my_TSK_sleep (10000);
                        /* Have to clear pending interrupt to clear I2C_STAT */
                        writew (0xFFFF, I2C_STAT);
                }

                /* set slave address */
                writew (devaddr, I2C_SA);
                /* read one byte from slave */
                writew (1, I2C_CNT);
                /* need stop bit here */
                writew (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP,
                        I2C_CON);

                status = wait_for_pin ();
                if (status & I2C_STAT_RRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX)
                        *value = readb (I2C_DATA);
#else
                        *value = readw (I2C_DATA);
#endif
                        my_TSK_sleep (20000);
                } else {
                        i2c_error = 1;
                }

                if (!i2c_error) {
                        writew (I2C_CON_EN, I2C_CON);
                        while (readw (I2C_STAT)
                               || (readw (I2C_CON) & I2C_CON_MST)) {
                                my_TSK_sleep (10000);
                                writew (0xFFFF, I2C_STAT);
                        }
                }
        }
        flush_fifo();
        writew (0xFFFF, I2C_STAT);
        writew (0, I2C_CNT);
        return i2c_error;
}

static int i2c_write_byte (Uint8 devaddr, Uint8 regoffset, Uint8 value)
{
        int i2c_error = 0;
        Uint16 status;

        /* wait until bus not busy */
        wait_for_bb ();

        /* two bytes */
        writew (2, I2C_CNT);
        /* set slave address */
        writew (devaddr, I2C_SA);
        /* stop bit needed here */
        writew (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX |
                I2C_CON_STP, I2C_CON);

        /* wait until state change */
        status = wait_for_pin ();


        /* wait until state change */
        status = wait_for_pin ();

        if (status & I2C_STAT_XRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_TI81XX)
                /* send out 1 byte */
                writeb (regoffset, I2C_DATA);
                writew (I2C_STAT_XRDY, I2C_STAT);

                status = wait_for_pin ();
                if ((status & I2C_STAT_XRDY)) {
                        /* send out next 1 byte */
                        writeb (value, I2C_DATA);
                        writew (I2C_STAT_XRDY, I2C_STAT);
                } else {
                        i2c_error = 1;
                }
#else
                /* send out two bytes */
                writew ((value << 8) + regoffset, I2C_DATA);
#endif
                /* must have enough delay to allow BB bit to go low */
                my_TSK_sleep (50000);
                if (readw (I2C_STAT) & I2C_STAT_NACK) {
                        i2c_error = 1;
                }
        } else {
                i2c_error = 1;
        }

        if (!i2c_error) {
                int eout = 200;

                writew (I2C_CON_EN, I2C_CON);
                while ((readw (I2C_STAT)) || (readw (I2C_CON) & I2C_CON_MST)) {
                        my_TSK_sleep (10000);
                        /* have to read to clear intrrupt */
                        writew (0xFFFF, I2C_STAT);
                        if(--eout == 0) /* better leave with error than hang */
                                break;
                }
        }
        flush_fifo();
        writew (0xFFFF, I2C_STAT);
        writew (0, I2C_CNT);
        return i2c_error;
}


static void flush_fifo(void)
{	Uint16 stat;

	/* note: if you try and read data when its not there or ready
	 * you get a bus error
	 */
	while(1){
		stat = readw(I2C_STAT);
		if(stat == I2C_STAT_RRDY){
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_TI81XX)
			readb(I2C_DATA);
#else
			readw(I2C_DATA);
#endif
			writew(I2C_STAT_RRDY,I2C_STAT);
			my_TSK_sleep(10000);
		}else
			break;
	}
}

int i2c_probe (Char chip)
{
	int res = 1; /* default = fail */

	if (chip == readw (I2C_OA)) {
		return res;
	}

	/* wait until bus not busy */
	wait_for_bb ();

	/* try to read one byte */
	writew (1, I2C_CNT);
	/* set slave address */
	writew (chip, I2C_SA);
	/* stop bit needed here */
	writew (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP, I2C_CON);
	/* enough delay for the NACK bit set */
	my_TSK_sleep (50000);

	if (!(readw (I2C_STAT) & I2C_STAT_NACK)) {
		res = 0;      /* success case */
		flush_fifo();
		writew(0xFFFF, I2C_STAT);
	} else {
		writew(0xFFFF, I2C_STAT);	 /* failue, clear sources*/
		writew (readw (I2C_CON) | I2C_CON_STP, I2C_CON); /* finish up xfer */
		my_TSK_sleep(20000);
		wait_for_bb ();
	}
	flush_fifo();
	writew (0, I2C_CNT); /* don't allow any more data in...we don't want it.*/
	writew(0xFFFF, I2C_STAT);
	return res;
}

int i2c_read (unsigned char chip, Uint8 addr, Int alen, unsigned char * buffer, Int len)
{
	int i;

	if (alen > 1) {
		printf ("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		printf ("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		if (i2c_read_byte (chip, addr + i, &buffer[i])) {
			printf ("I2C read: I/O error\n");
			i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

int i2c_write (unsigned char chip, Uint8 addr, Int alen, unsigned char * buffer, Int len)
{
	int i;

	if (alen > 1) {
		printf ("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		printf ("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		if (i2c_write_byte (chip, addr + i, buffer[i])) {
			printf ("I2C read: I/O error\n");
			i2c_init (OMAP_I2C_FAST_MODE, CONFIG_SYS_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

static void wait_for_bb (void)
{
	int timeout = 100;
	Uint16 stat;

	writew(0xFFFF, I2C_STAT);	 /* clear current interruts...*/
	while ((stat = readw (I2C_STAT) & I2C_STAT_BB) && timeout--) {
		writew (stat, I2C_STAT);
		my_TSK_sleep (10000);
	}

	if (timeout <= 0) {
		printf ("timed out in wait_for_bb: I2C_STAT=%x\n",
			readw (I2C_STAT));
	}
	writew(0xFFFF, I2C_STAT);	 /* clear delayed stuff*/
}

static Uint16 wait_for_pin (void)
{
	Uint16 status;
	int timeout = 100;

	do {
		my_TSK_sleep (10000);
		status = readw (I2C_STAT);
       } while (  !(status &
                   (I2C_STAT_ROVR | I2C_STAT_XUDF | I2C_STAT_XRDY |
                    I2C_STAT_RRDY | I2C_STAT_ARDY | I2C_STAT_NACK |
                    I2C_STAT_AL)) && timeout--);

	if (timeout <= 0) {
		printf ("timed out in wait_for_pin: I2C_STAT=%x\n",
			readw (I2C_STAT));
			writew(0xFFFF, I2C_STAT);
    }
	return status;
}


int i2c_set_bus_num(unsigned int bus)
{
	if ( bus >= I2C_BUS_MAX ) {
		printf("Bad bus: %d\n", bus);
		return -1;
	}


	if (bus == 2)
		i2c_base = I2C_BASE3;
	else if (bus == 1)
		i2c_base = I2C_BASE2;
	else
		i2c_base = I2C_BASE1;

	I2C_REV	   = (i2c_base + 0x00);
	I2C_IE 	   = (i2c_base + 0x04);
	I2C_STAT   = (i2c_base + 0x08);
	I2C_IV 	   = (i2c_base + 0x0c);
	I2C_BUF	   = (i2c_base + 0x14);
	I2C_CNT	   = (i2c_base + 0x18);
	I2C_DATA   = (i2c_base + 0x1c);
	I2C_SYSC   = (i2c_base + 0x20);
	I2C_CON	   = (i2c_base + 0x24);
	I2C_OA 	   = (i2c_base + 0x28);
	I2C_SA 	   = (i2c_base + 0x2c);
	I2C_PSC	   = (i2c_base + 0x30);
	I2C_SCLL   = (i2c_base + 0x34);
	I2C_SCLH   = (i2c_base + 0x38);
	I2C_SYSTEST = (i2c_base + 0x3c);

	current_bus = bus;

	if(!bus_initialized[current_bus])
		i2c_init(OMAP_I2C_FAST_MODE, CONFIG_SYS_I2C_SLAVE);

	return 0;
}
