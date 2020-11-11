################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
TCF_SRCS += \
../i2c_test.tcf 

GEN_MISC_FILES += \
./i2c_testcfg.cmd \
./i2c_testcfg.s?? \
./i2c_testcfg_c.c \
./i2c_testcfg.h \
./i2c_testcfg.h?? \
./i2c_test.cdb 

GEN_MISC_FILES__QUOTED += \
"i2c_testcfg.cmd" \
"i2c_testcfg.s??" \
"i2c_testcfg_c.c" \
"i2c_testcfg.h" \
"i2c_testcfg.h??" \
"i2c_test.cdb" 

TCF_SRCS__QUOTED += \
"../i2c_test.tcf" 


# Each subdirectory must supply rules for building sources it contributes
i2c_testcfg.cmd: ../i2c_test.tcf
	@echo 'Building file: $<'
	@echo 'Invoking: TConf Script Compiler'
	"/tconf" -b -Dconfig.importPath="/packages;" "$<"
	@echo 'Finished building: $<'
	@echo ' '

i2c_testcfg.s??: i2c_testcfg.cmd
i2c_testcfg_c.c: i2c_testcfg.cmd
i2c_testcfg.h: i2c_testcfg.cmd
i2c_testcfg.h??: i2c_testcfg.cmd
i2c_test.cdb: i2c_testcfg.cmd


