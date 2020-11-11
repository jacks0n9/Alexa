################################################################################
# Automatically-generated file. Do not edit!
################################################################################

CG_TOOL_ROOT := /opt/ti/ccsv5/tools/compiler/c6000

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./i2c_testcfg_c.obj" \
"./i2c_testcfg.obj" \
"./i2cSample_main.obj" \
-l"libc.a" \
-l"ti.pspiom.i2c.a64P" \
-l"ti.pspiom.prcm.a64P" \

-include ../makefile.init

RM := rm -rf
RMDIR := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: prcm_i2c_test.out

# Tool invocations
prcm_i2c_test.out: $(OBJS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: C6000 Linker'
	"/opt/ti/ccsv5/tools/compiler/c6000/bin/cl6x" -mv64+ -g --display_error_number --diag_warning=225 -z -m"prcm_i2c_test.map" --warn_sections -i"/opt/ti/ccsv5/tools/compiler/c6000/lib" -i"/home/user/Downloads/dspdev/dsp/biospsp/packages/ti/pspiom/i2c/lib/dm3730/Debug" -i"/home/user/Downloads/dspdev/dsp/biospsp/packages/ti/pspiom/prcm/lib/dm3730/Debug" -i"/opt/ti/ccsv5/tools/compiler/c6000/include" -i"/opt/ti/bios_5_41_11_38/packages/ti/rtdx/lib/c6000" -i"/opt/ti/bios_5_41_11_38/packages/ti/bios/lib" --reread_libs --rom_model -o "prcm_i2c_test.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_SRCS__QUOTED)$(GEN_MISC_FILES__QUOTED)$(C6000_EXECUTABLE_OUTPUTS__QUOTED)$(GEN_CMDS__QUOTED) "prcm_i2c_test.out"
	-$(RM) "i2cSample_main.pp" "i2c_testcfg_c.pp" 
	-$(RM) "i2c_testcfg.pp" 
	-$(RM) "i2cSample_main.obj" "i2c_testcfg.obj" "i2c_testcfg_c.obj" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
