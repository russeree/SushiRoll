################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32f038xx.s 

C_SRCS += \
../startup/syscalls.c 

OBJS += \
./startup/startup_stm32f038xx.o \
./startup/syscalls.o 

C_DEPS += \
./startup/syscalls.d 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	@echo $(PWD)
	arm-none-eabi-as -mcpu=cortex-m0 -mthumb -mfloat-abi=soft -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

startup/%.o: ../startup/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F038xx -I"D:/Google Drive Backup/Sushi Board - FetBoard - Parallel/SushiRoll/Inc" -I"D:/Google Drive Backup/Sushi Board - FetBoard - Parallel/SushiRoll/Drivers/STM32F0xx_HAL_Driver/Inc" -I"D:/Google Drive Backup/Sushi Board - FetBoard - Parallel/SushiRoll/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy" -I"D:/Google Drive Backup/Sushi Board - FetBoard - Parallel/SushiRoll/Drivers/CMSIS/Device/ST/STM32F0xx/Include" -I"D:/Google Drive Backup/Sushi Board - FetBoard - Parallel/SushiRoll/Drivers/CMSIS/Include"  -Os -g3 -Wall -fmessage-length=0 -specs=nano.specs -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


