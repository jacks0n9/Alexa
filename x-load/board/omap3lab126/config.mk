# (C) Copyright 2006
# Texas Instruments, <www.ti.com>
#
# OMAP3LAB126 board uses DM3725 (ARM-CortexA8) cpu
# see http://www.ti.com/ for more information on Texas Instruments#
#
# OMAP3LAB126 has 1 bank of 128MB mPOP-SDRAM on CS0
# Physical Address:
# 8000'0000 (bank0)

# For use if you want X-Loader to relocate from SRAM to DDR
#TEXT_BASE = 0x80e80000

# For XIP in 64K of SRAM or debug (GP device has it all available)
# SRAM 40200000-4020FFFF base
# initial stack at 0x4020fffc used in s_init (below xloader).
# The run time stack is (above xloader, size = 4k)
# If any globals exist there needs to be room for them also
TEXT_BASE = 0x40201000
PLATFORM_RELFLAGS	+=	-DFOUR_BIT_ERROR_CORRECT
