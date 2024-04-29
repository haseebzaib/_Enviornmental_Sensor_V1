################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Embedded_CLI/console.c \
../Embedded_CLI/embedded_cli.c 

OBJS += \
./Embedded_CLI/console.o \
./Embedded_CLI/embedded_cli.o 

C_DEPS += \
./Embedded_CLI/console.d \
./Embedded_CLI/embedded_cli.d 


# Each subdirectory must supply rules for building sources it contributes
Embedded_CLI/%.o Embedded_CLI/%.su Embedded_CLI/%.cyclo: ../Embedded_CLI/%.c Embedded_CLI/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/git/Enviornmental_Sensor/Embedded_CLI" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/git/Enviornmental_Sensor/Internal_RTC_Handling" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/git/Enviornmental_Sensor/Libs" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/git/Enviornmental_Sensor/SDcard_Filehandling" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Embedded_CLI

clean-Embedded_CLI:
	-$(RM) ./Embedded_CLI/console.cyclo ./Embedded_CLI/console.d ./Embedded_CLI/console.o ./Embedded_CLI/console.su ./Embedded_CLI/embedded_cli.cyclo ./Embedded_CLI/embedded_cli.d ./Embedded_CLI/embedded_cli.o ./Embedded_CLI/embedded_cli.su

.PHONY: clean-Embedded_CLI

