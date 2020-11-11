/* 
 *  Copyright 2011 by Texas Instruments Incorporated.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 * 
 */

/*
 *  ======== ocvc_DaVinci.c ========
 *
 */

#include <linux/version.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/delay.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
// assume Linux is from MVL5.0 for Davinci
#include <asm/arch-davinci/i2c-client.h>
#else
#include <asm-generic/gpio.h>
#endif

#include "lpm_driver.h"


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)

#define VDDIMX_ENABLE_OFFSET 161

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)

#define VDDIMX_ENABLE_OFFSET 145

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)

#define VDDIMX_ENABLE_OFFSET 88

#endif


/*
 * Debug macro: trace is set with insmod parameter.
 *
 * insmod ocvc.ko trace=1
 *
 */
#if defined(DEBUG)
#define TRACE if (trace) printk
#else
#define TRACE(...)
#endif


/* Module parameters */
static uint trace = 0;
static uint off_delay_usec = 4;
static uint on_delay_usec = 250;

module_param(trace, bool, S_IRUGO);
module_param(off_delay_usec, uint, S_IRUGO);
module_param(on_delay_usec, uint, S_IRUGO);


/* sample usage of private data */
typedef struct OCVC_Dev {
    unsigned int        offCount;
    unsigned int        onCount;
} OCVC_Dev;


/* static object for this device driver */
static OCVC_Dev OCVC_OBJ = {
    .offCount   = 0,
    .onCount    = 0,
};


/*
 * Functions which turn on/off external power supply.
 * These will be called by the lpm.ko driver.
 */
static void ocvc_off(void *hndl);
static void ocvc_on(void *hndl);


/*
 *  ======== ocvc_init ========
 */
static int __init ocvc_init(void)
{
    OCVC_Dev   *ocvc = &OCVC_OBJ;
    int         err = 0;


    TRACE("ocvc: +ocvc_init\n");
    TRACE("ocvc: off_delay_usec = %d\n", off_delay_usec);
    TRACE("ocvc: on_delay_usec = %d\n", on_delay_usec);

    /* Register the OFF function with the lpm.ko driver. */
    err = lpm_regExtOffFxn("/dev/lpm0", ocvc_off, (void *)ocvc);

    if (err) {
        TRACE("ocvc: Error: lpm_regExtOffFxn failed (err = %d)\n", err);
        goto fail01;
    }


    /* Register the ON function with the lpm.ko driver. */
    err = lpm_regExtOnFxn("/dev/lpm0", ocvc_on, (void *)ocvc);
    if (err) {
        TRACE("ocvc: Error: lpm_regExtOnFxn failed (err = %d)\n", err);
        goto fail02;
    }


fail02:
    if (err) {
        /* unregister off function */
        lpm_regExtOffFxn("/dev/lpm0", NULL, NULL);
    }

fail01:

    TRACE("ocvc: -ocvc_init: err = %d\n", err);
    return err;
}


/*
 *  ======== ocvc_exit ========
 */
static void __exit ocvc_exit(void)
{

    TRACE("ocvc: +ocvc_exit\n");

    /* unregister the OFF and ON functions */
    lpm_regExtOffFxn("/dev/lpm0", NULL, NULL);
    lpm_regExtOnFxn("/dev/lpm0", NULL, NULL);

    TRACE("ocvc: -ocvc_exit\n");
}


/*
 *  ======== ocvc_off ========
 */
static void ocvc_off(void *hndl)
{
    OCVC_Dev *obj = (OCVC_Dev*)hndl;

    TRACE("ocvc: +ocvc_off\n");

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
// assume Linux is from MVL5.0 for Davinci, which has davinci_i2c_expander_op()
    /* Use the i2c interface to turn off the DSP. */
    davinci_i2c_expander_op(0x3A, VDDIMX_EN, 0);
#else
    gpio_set_value_cansleep(VDDIMX_ENABLE_OFFSET, 0);
#endif
    obj->offCount++;

    /* Delay to allow external voltage to stabilize. */
    if (off_delay_usec) udelay(off_delay_usec);

    TRACE("ocvc: -ocvc_off: offCount = %d\n", obj->offCount);

}


/*
 *  ======== ocvc_on ========
 */
static void ocvc_on(void *hndl)
{
    OCVC_Dev *obj = (OCVC_Dev*)hndl;

    TRACE("ocvc: +ocvc_on\n");

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
// assume Linux is from MVL5.0 for Davinci, which has davinci_i2c_expander_op()
    /* Use the i2c interface to turn off the DSP. */
    davinci_i2c_expander_op(0x3A, VDDIMX_EN, 1);
#else
    gpio_set_value_cansleep(VDDIMX_ENABLE_OFFSET, 1);
#endif
    obj->onCount++;

    /* Delay to allow external voltage to stabilize. */
    if (on_delay_usec) udelay(on_delay_usec);

    TRACE("ocvc: -ocvc_on: onCount = %d\n", obj->onCount);
}


MODULE_LICENSE("GPL");
module_init(ocvc_init);
module_exit(ocvc_exit);
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:31; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

