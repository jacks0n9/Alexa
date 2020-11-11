################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/Prcm.c 

OBJS += \
./lib/Prcm.obj 

C_DEPS += \
./lib/Prcm.pp 

C_DEPS__QUOTED += \
"lib\Prcm.pp" 

OBJS__QUOTED += \
"lib\Prcm.obj" 

C_SRCS__QUOTED += \
"../lib/Prcm.c" 


# Each subdirectory must supply rules for building sources it contributes
lib/Prcm.obj: ../lib/Prcm.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files (x86)/ccsv5/tools/compiler/c6000/bin/cl6x" -mv6400 -g --include_path="C:/Program Files (x86)/ccsv5/tools/compiler/c6000/include" --include_path="C:\Program Files (x86)\bios_5_41_10_36\packages\ti\bios\include" --include_path="C:\Users\a0271467\Documents\PSP_1_30_00_05\ti\pspiom\prcm" --diag_warning=225 --abi=eabi --preproc_with_compile --preproc_dependency="lib/Prcm.pp" --obj_directory="lib" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


