################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/gpio.c \
../Src/main.c \
../Src/stm32f0xx_hal_msp.c \
../Src/stm32f0xx_it.c \
../Src/sushi_dma.c \
../Src/sushi_flash.c \
../Src/sushi_menu.c \
../Src/sushi_timer.c \
../Src/sushi_uart.c \
../Src/system_stm32f0xx.c 

OBJS += \
./Src/gpio.o \
./Src/main.o \
./Src/stm32f0xx_hal_msp.o \
./Src/stm32f0xx_it.o \
./Src/sushi_dma.o \
./Src/sushi_flash.o \
./Src/sushi_menu.o \
./Src/sushi_timer.o \
./Src/sushi_uart.o \
./Src/system_stm32f0xx.o 

C_DEPS += \
./Src/gpio.d \
./Src/main.d \
./Src/stm32f0xx_hal_msp.d \
./Src/stm32f0xx_it.d \
./Src/sushi_dma.d \
./Src/sushi_flash.d \
./Src/sushi_menu.d \
./Src/sushi_timer.d \
./Src/sushi_uart.d \
./Src/system_stm32f0xx.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F038xx -I"D:/Google Drive Backup/Sushi Board - FetBoard - Parallel/SushiRoll/Inc" -I"D:/Google Drive Backup/Sushi Board - FetBoard - Parallel/SushiRoll/Drivers/STM32F0xx_HAL_Driver/Inc" -I"D:/Google Drive Backup/Sushi Board - FetBoard - Parallel/SushiRoll/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy" -I"D:/Google Drive Backup/Sushi Board - FetBoard - Parallel/SushiRoll/Drivers/CMSIS/Device/ST/STM32F0xx/Include" -I"D:/Google Drive Backup/Sushi Board - FetBoard - Parallel/SushiRoll/Drivers/CMSIS/Include"  -Os -g3 -Wall -fmessage-length=0 -v -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


