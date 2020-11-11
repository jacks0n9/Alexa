################################################################################
# Automatically-generated file. Do not edit!
################################################################################
-include ../../Rules.make

# Each subdirectory must supply rules for building sources it contributes
src/Prcm.obj: ../src/Prcm.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	$(CODEGEN_INSTALL_DIR)/bin/cl6x -mv64+ -g --include_path=$(CODEGEN_INSTALL_DIR)"/include" --include_path=$(BIOSPSP_BUILD_ROOT)"/packages/" --include_path=$(BIOS_INSTALL_DIR)"/packages/ti/bios/include"  --diag_warning=225 --preproc_with_compile --preproc_dependency="src/Prcm.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


