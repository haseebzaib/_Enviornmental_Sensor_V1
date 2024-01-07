################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libs/Utils.c \
../Libs/internal_adc.c \
../Libs/scd4x_i2c.c \
../Libs/sensirion_common.c \
../Libs/sensirion_i2c.c \
../Libs/sensirion_i2c_hal.c \
../Libs/sensirion_shdlc.c \
../Libs/sensirion_uart_implementation.c \
../Libs/sps30.c 

OBJS += \
./Libs/Utils.o \
./Libs/internal_adc.o \
./Libs/scd4x_i2c.o \
./Libs/sensirion_common.o \
./Libs/sensirion_i2c.o \
./Libs/sensirion_i2c_hal.o \
./Libs/sensirion_shdlc.o \
./Libs/sensirion_uart_implementation.o \
./Libs/sps30.o 

C_DEPS += \
./Libs/Utils.d \
./Libs/internal_adc.d \
./Libs/scd4x_i2c.d \
./Libs/sensirion_common.d \
./Libs/sensirion_i2c.d \
./Libs/sensirion_i2c_hal.d \
./Libs/sensirion_shdlc.d \
./Libs/sensirion_uart_implementation.d \
./Libs/sps30.d 


# Each subdirectory must supply rules for building sources it contributes
Libs/%.o Libs/%.su Libs/%.cyclo: ../Libs/%.c Libs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/Embedded_CLI" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/Internal_RTC_Handling" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/Libs" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/SDcard_Filehandling" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libs

clean-Libs:
	-$(RM) ./Libs/Utils.cyclo ./Libs/Utils.d ./Libs/Utils.o ./Libs/Utils.su ./Libs/internal_adc.cyclo ./Libs/internal_adc.d ./Libs/internal_adc.o ./Libs/internal_adc.su ./Libs/scd4x_i2c.cyclo ./Libs/scd4x_i2c.d ./Libs/scd4x_i2c.o ./Libs/scd4x_i2c.su ./Libs/sensirion_common.cyclo ./Libs/sensirion_common.d ./Libs/sensirion_common.o ./Libs/sensirion_common.su ./Libs/sensirion_i2c.cyclo ./Libs/sensirion_i2c.d ./Libs/sensirion_i2c.o ./Libs/sensirion_i2c.su ./Libs/sensirion_i2c_hal.cyclo ./Libs/sensirion_i2c_hal.d ./Libs/sensirion_i2c_hal.o ./Libs/sensirion_i2c_hal.su ./Libs/sensirion_shdlc.cyclo ./Libs/sensirion_shdlc.d ./Libs/sensirion_shdlc.o ./Libs/sensirion_shdlc.su ./Libs/sensirion_uart_implementation.cyclo ./Libs/sensirion_uart_implementation.d ./Libs/sensirion_uart_implementation.o ./Libs/sensirion_uart_implementation.su ./Libs/sps30.cyclo ./Libs/sps30.d ./Libs/sps30.o ./Libs/sps30.su

.PHONY: clean-Libs

