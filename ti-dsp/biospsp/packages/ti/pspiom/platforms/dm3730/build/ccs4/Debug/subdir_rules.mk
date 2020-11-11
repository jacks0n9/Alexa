################################################################################
# Automatically-generated file. Do not edit!
################################################################################
-include ../../../../../Rules.make

# Each subdirectory must supply rules for building sources it contributes
mcbsp_evmInit.obj: $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/platforms/dm3730/src/mcbsp_evmInit.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"$(CODEGEN_INSTALL_DIR)/bin/cl6x" -mv64+ -g --define=xdc_target_types__=C64P --include_path="$(CODEGEN_INSTALL_DIR)/include" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/" --include_path="$(EDMA3_LLD_INSTALL_DIR)/edma3lld_01_11_02_05" --include_path="$(BIOS_INSTALL_DIR)/packages" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/platforms/dm3730/build/ccs4/Debug" --include_path="$(BIOS_INSTALL_DIR)/packages/ti/bios/include" --include_path="$(BIOS_INSTALL_DIR)/packages/ti/rtdx/include/c6000" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/platforms/dm3730/build/ccs4/Debug" --diag_warning=225 --display_error_number --issue_remarks --gen_func_subsections --preproc_with_compile --preproc_dependency="mcbsp_evmInit.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


