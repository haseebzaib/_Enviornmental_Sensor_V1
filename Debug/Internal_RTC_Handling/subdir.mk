################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Internal_RTC_Handling/Uhr.c 

OBJS += \
./Internal_RTC_Handling/Uhr.o 

C_DEPS += \
./Internal_RTC_Handling/Uhr.d 


# Each subdirectory must supply rules for building sources it contributes
Internal_RTC_Handling/%.o Internal_RTC_Handling/%.su Internal_RTC_Handling/%.cyclo: ../Internal_RTC_Handling/%.c Internal_RTC_Handling/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/Embedded_CLI" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/Internal_RTC_Handling" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/Libs" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/SDcard_Filehandling" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Internal_RTC_Handling

clean-Internal_RTC_Handling:
	-$(RM) ./Internal_RTC_Handling/Uhr.cyclo ./Internal_RTC_Handling/Uhr.d ./Internal_RTC_Handling/Uhr.o ./Internal_RTC_Handling/Uhr.su

.PHONY: clean-Internal_RTC_Handling

