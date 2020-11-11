################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
TCF_SRCS += \
../i2c_test.tcf 

S??_SRCS += \
./i2c_testcfg.s?? 

C_SRCS += \
../i2cSample_main.c \
./i2c_testcfg_c.c 

OBJS += \
./i2cSample_main.obj \
./i2c_testcfg.obj \
./i2c_testcfg_c.obj 

GEN_SRCS += \
./i2c_testcfg.cmd \
./i2c_testcfg.s?? \
./i2c_testcfg_c.c 

GEN_MISC_FILES += \
./i2c_testcfg.h \
./i2c_testcfg.h?? \
./i2c_test.cdb 

S??_DEPS += \
./i2c_testcfg.pp 

C_DEPS += \
./i2cSample_main.pp \
./i2c_testcfg_c.pp 

GEN_CMDS += \
./i2c_testcfg.cmd 

GEN_SRCS__QUOTED += \
"i2c_testcfg.cmd" \
"$(shell echo i2c_testcfg.s??)" \
"i2c_testcfg_c.c" 

GEN_MISC_FILES__QUOTED += \
"i2c_testcfg.h" \
"$(shell echo i2c_testcfg.h??)" \
"i2c_test.cdb" 

C_DEPS__QUOTED += \
"i2cSample_main.pp" \
"i2c_testcfg_c.pp" 

S??_DEPS__QUOTED += \
"i2c_testcfg.pp" 

OBJS__QUOTED += \
"i2cSample_main.obj" \
"i2c_testcfg.obj" \
"i2c_testcfg_c.obj" 

C_SRCS__QUOTED += \
"../i2cSample_main.c" \
"./i2c_testcfg_c.c" 

TCF_SRCS__QUOTED += \
"../i2c_test.tcf" 

GEN_CMDS__FLAG += \
-l"./i2c_testcfg.cmd" 

S??_SRCS__QUOTED += \
"./i2c_testcfg.s??" 

S??_OBJS__QUOTED += \
"i2c_testcfg.obj" 


