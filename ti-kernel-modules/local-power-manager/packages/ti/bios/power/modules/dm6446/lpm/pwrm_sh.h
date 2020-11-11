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
 *  ======== pwrm_sh.h ========
 *
 *  GPP and DSP shared PWRM definitions.
 *
 */


#if !defined (PWRM_SH_H)
#define PWRM_SH_H

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/* event */
#define PWRM_SH_SLEEP           0x1
#define PWRM_SH_ACKNOWLEDGE     0x2
#define PWRM_SH_AWAKE           0x3
#define PWRM_SH_QUERY           0x4
#define PWRM_SH_DATA_L          0x5
#define PWRM_SH_DATA_H          0x6
#define PWRM_SH_RESUME          0x7

/* payload */
#define PWRM_SH_HIBERNATE       0x10
#define PWRM_SH_SYMBOLS_H       0x20
#define PWRM_SH_SYMBOLS_L       0x30


/*
 * symbol table format
 * -----------------------------------
 * far unsigned int PWRM_symbolTable[] = {
 *     number_of_entries_to_follow,
 *     (unsigned int)&_PWRM_restoreJouleIMem,       // warm boot address
 *     (unsigned int)&_PWRM_saveDone,               // software handshake
 *
 */


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* !defined (PWRM_SH_H) */
/*
 *  @(#) ti.bios.power; 1, 1, 1,1; 7-13-2011 17:46:31; /db/atree/library/trees/power/power-g10x/src/ xlibrary

 */

