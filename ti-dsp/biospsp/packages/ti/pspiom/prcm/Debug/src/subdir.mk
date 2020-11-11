################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Prcm.c 

OBJS += \
./src/Prcm.obj 

C_DEPS += \
./src/Prcm.pp 

C_DEPS__QUOTED += \
"src\Prcm.pp" 

OBJS__QUOTED += \
"src\Prcm.obj" 

C_SRCS__QUOTED += \
"../src/Prcm.c" 


# Each subdirectory must supply rules for building sources it contributes
src/Prcm.obj: ../src/Prcm.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files (x86)/ccsv5/tools/compiler/c6000/bin/cl6x" -mv64+ -g --include_path="C:/Program Files (x86)/ccsv5/tools/compiler/c6000/include" --include_path="C:\Program Files (x86)\bios_5_41_10_36\packages\ti\bios\include" --include_path="C:\Users\a0271467\Documents\PSP_1_30_00_05\ti\pspiom\prcm" --diag_warning=225 --preproc_with_compile --preproc_dependency="src/Prcm.pp" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


