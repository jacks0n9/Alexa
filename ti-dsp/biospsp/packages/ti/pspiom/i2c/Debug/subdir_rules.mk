################################################################################
# Automatically-generated file. Do not edit!
################################################################################
-include ../../Rules.make

# Each subdirectory must supply rules for building sources it contributes
I2c.obj: ../I2c.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"$(CODEGEN_INSTALL_DIR)/bin/cl6x" -mv64+ -g --include_path="$(CODEGEN_INSTALL_DIR)/include" --include_path="$(BIOS_INSTALL_DIR)/packages/ti/bios/include" --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="I2c.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


