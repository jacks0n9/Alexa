################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
i2cSample_main.obj: ../i2cSample_main.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"/opt/ti/ccsv5/tools/compiler/c6000/bin/cl6x" -mv64+ -g --include_path="/opt/ti/ccsv5/tools/compiler/c6000/include" --include_path="/home/user/Downloads/dspdev/dsp/biospsp/packages/" --include_path="/home/user/Downloads/dspdev/dsp/biospsp/packages/ti/pspiom/examples/dm3730/prcm_i2c_test/Debug" --include_path="/opt/ti/bios_5_41_11_38/packages/ti/bios/include" --include_path="/opt/ti/bios_5_41_11_38/packages/ti/rtdx/include/c6000" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="i2cSample_main.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

i2c_testcfg.cmd: ../i2c_test.tcf
	@echo 'Building file: $<'
	@echo 'Invoking: TConf Script Compiler'
	"/opt/ti/xdctools_3_22_04_46/tconf" -b -Dconfig.importPath="/opt/ti/bios_5_41_11_38/packages;" "$<"
	@echo 'Finished building: $<'
	@echo ' '

i2c_testcfg.s??: i2c_testcfg.cmd
i2c_testcfg_c.c: i2c_testcfg.cmd
i2c_testcfg.h: i2c_testcfg.cmd
i2c_testcfg.h??: i2c_testcfg.cmd
i2c_test.cdb: i2c_testcfg.cmd

i2c_testcfg.obj: ./i2c_testcfg.s?? $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"/opt/ti/ccsv5/tools/compiler/c6000/bin/cl6x" -mv64+ -g --include_path="/opt/ti/ccsv5/tools/compiler/c6000/include" --include_path="/home/user/Downloads/dspdev/dsp/biospsp/packages/" --include_path="/home/user/Downloads/dspdev/dsp/biospsp/packages/ti/pspiom/examples/dm3730/prcm_i2c_test/Debug" --include_path="/opt/ti/bios_5_41_11_38/packages/ti/bios/include" --include_path="/opt/ti/bios_5_41_11_38/packages/ti/rtdx/include/c6000" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="i2c_testcfg.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

i2c_testcfg_c.obj: ./i2c_testcfg_c.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"/opt/ti/ccsv5/tools/compiler/c6000/bin/cl6x" -mv64+ -g --include_path="/opt/ti/ccsv5/tools/compiler/c6000/include" --include_path="/home/user/Downloads/dspdev/dsp/biospsp/packages/" --include_path="/home/user/Downloads/dspdev/dsp/biospsp/packages/ti/pspiom/examples/dm3730/prcm_i2c_test/Debug" --include_path="/opt/ti/bios_5_41_11_38/packages/ti/bios/include" --include_path="/opt/ti/bios_5_41_11_38/packages/ti/rtdx/include/c6000" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="i2c_testcfg_c.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


