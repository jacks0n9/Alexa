/* 
 * Copyright (c) 2011, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */


//  ---------- Operating System headers

#if defined(WIN32)

#include <stdio.h>
#include <windows.h>
#include <winbase.h>

#define __D(fmt, ...) RETAILMSG(1, (L##fmt, __VA_ARGS__))

#define sleep Sleep

#else

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define __D printf

#endif

//  ---------- ti.bios.power headers
#include <ti/bios/power/lpm.h>

// define the usage statement
#define USAGE_TEXT "\n\
Usage:\n\
    lpmON [options]\n\
\n\
Options:\n\
    e           : exit without close handling\n\
    h           : print this help message\n\
    n           : normal reference count handling\n\
    s           : spin, useful to kill testing\n\
\n"
static char *g_Usage = USAGE_TEXT;


int main(int argc, char *argv[])
{
    char *arg;
    int opt;
    int c, cp;
    LPM_Status  lpmStat = LPM_SOK;
    LPM_Handle  hLPM = NULL;
    int doExit = 0;
    int useRefCount = 0;
    int doSpin = 0;

    __D("==== LPM ON Test ====\n");

    /* parse the command line options */
    for (opt = 1; (opt < argc) && (argv[opt][0] == '-'); opt++) {
        for (c = 0, cp = 1; argv[opt][cp] != '\0'; cp++) {
            c = (c << 8) | (int)argv[opt][cp];
        }
        arg = argv[opt+1];

        switch (c) {
            case 'e':
                doExit = 1;
                break;

            case 'h': /* -h */
                __D("%s", g_Usage);
                exit(0);

            case 'n':
                useRefCount = 1;
                break;

            case 's':
                doSpin = 1;
                break;

            default:
                __D("Error: invalid option: -%c\n", c);
                exit(-1);
        }
    }

    // Initialize the LPM module.
    LPM_init();

    // Open the LPM object for the given resource.
    __D("app: LPM_open\n");
    lpmStat = LPM_open("/dev/lpm0", &hLPM);

    if (lpmStat != LPM_SOK) {
        __D("app: Error: LPM_open failed\n");
        goto fail;
    }

    if (!useRefCount) {
        // Set the reference override control bit
        __D("app: LPM_set(LPM_CTRL_REFCOUNTOVR)\n");
        lpmStat = LPM_set(hLPM, LPM_CTRL_REFCOUNTOVR);

        if (lpmStat != LPM_SOK) {
            __D("app: Error: LPM_set failed\n");
            goto fail;
        }
    }

    // Turn on power for the resource.
    __D("app: LPM_on\n");
    lpmStat = LPM_on(hLPM);

    if (lpmStat != LPM_SOK) {
        __D("app: Error: LPM_on failed\n");
    }

    // delay here to observe power transition
    if (useRefCount) {
        sleep(2);
    }

    // exit without proper finalizing
    if (doExit) {
        __D("app: exiting without finalizing\n");
        exit(0);
    }


    // spin here so user can kill application
    if (doSpin) {
        __D("app: entering spin loop, must kill application\n");
        while (1);
    }

    // close the LPM object
    __D("app: LPM_close\n");
    lpmStat = LPM_close(hLPM);

    if (lpmStat != LPM_SOK) {
        __D("app: Error: LPM_close failed\n");
    }

    // Finalize the LPM module.
    LPM_exit();

    __D("==== LPM ON completed ====\n\n");

fail:
    return 0;
}
/*
 *  @(#) ti.bios.power.utils; 1, 0, 0,2; 7-13-2011 17:57:00; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

