# Each subdirectory must supply rules for building sources it contributes
Mcbsp.obj: $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/mcbsp/src/Mcbsp.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"$(CODEGEN_INSTALL_DIR)/bin/cl6x" -mv64+ -g --define=_TMS320C6X --define=Mcbsp_LOOPJOB_ENABLE  --include_path="$(CODEGEN_INSTALL_DIR)/include" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/" --include_path="$(BIOS_INSTALL_DIR)/packages/ti/bios/include"  --include_path="$(EDMA3_LLD_INSTALL_DIR)/packages" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/ti/rtdx/include/c6000" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/mcbsp/build/dm3730/ccs4/Debug" --diag_warning=225 --display_error_number --issue_remarks --gen_func_subsections --preproc_with_compile --preproc_dependency="Mcbsp.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

Mcbsp_edma.obj: $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/mcbsp/src/Mcbsp_edma.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"$(CODEGEN_INSTALL_DIR)/bin/cl6x" -MV64+ -g --DEFINE=_tms320c6x --define=Mcbsp_LOOPJOB_ENABLE --include_path="$(CODEGEN_INSTALL_DIR)/include" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/" --include_path="$(BIOS_INSTALL_DIR)/packages/ti/bios/include"  --include_path="$(EDMA3_LLD_INSTALL_DIR)/packages" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/ti/rtdx/include/c6000" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/mcbsp/build/dm3730/ccs4/Debug" --diag_warning=225 --display_error_number --issue_remarks --gen_func_subsections --preproc_with_compile --preproc_dependency="Mcbsp_edma.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

Mcbsp_ioctl.obj: $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/mcbsp/src/Mcbsp_ioctl.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"$(CODEGEN_INSTALL_DIR)/bin/cl6x" -mv64+ -g --define=_TMS320C6X --define=Mcbsp_LOOPJOB_ENABLE --include_path="$(CODEGEN_INSTALL_DIR)/include" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/" --include_path="$(BIOS_INSTALL_DIR)/packages/ti/bios/include"  --include_path="$(EDMA3_LLD_INSTALL_DIR)/packages" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/ti/rtdx/include/c6000" --include_path="$(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/mcbsp/build/dm3730/ccs4/Debug" --diag_warning=225 --display_error_number --issue_remarks --gen_func_subsections --preproc_with_compile --preproc_dependency="Mcbsp_ioctl.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


