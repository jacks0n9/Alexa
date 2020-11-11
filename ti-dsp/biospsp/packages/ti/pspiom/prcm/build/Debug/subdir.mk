################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Prcm.c 

OBJS += \
./Prcm.obj 

C_DEPS += \
./Prcm.pp 

C_DEPS__QUOTED += \
"Prcm.pp" 

OBJS__QUOTED += \
"Prcm.obj" 

C_SRCS__QUOTED += \
"../Prcm.c" 


# Each subdirectory must supply rules for building sources it contributes
Prcm.obj: ../Prcm.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: TMS470 Compiler'
	"C:/Program Files (x86)/ccsv5/tools/compiler/tms470/bin/cl470" -mv7A8 -g --include_path="C:\Users\a0271467\Documents\PSP_1_30_00_05\ti\pspiom\prcm" --include_path="C:/Program Files (x86)/bios_5_41_10_36/packages/ti/bios/include" --include_path="C:/Program Files (x86)/ccsv5/tools/compiler/c6000/include" --diag_warning=225 -me --enum_type=packed --abi=eabi --code_state=32 --preproc_with_compile --preproc_dependency="Prcm.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


