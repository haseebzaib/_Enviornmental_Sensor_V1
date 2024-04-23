################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SDcard_Filehandling/FATFSLNRequired.c \
../SDcard_Filehandling/File_Handling.c \
../SDcard_Filehandling/csv_json_handling.c 

OBJS += \
./SDcard_Filehandling/FATFSLNRequired.o \
./SDcard_Filehandling/File_Handling.o \
./SDcard_Filehandling/csv_json_handling.o 

C_DEPS += \
./SDcard_Filehandling/FATFSLNRequired.d \
./SDcard_Filehandling/File_Handling.d \
./SDcard_Filehandling/csv_json_handling.d 


# Each subdirectory must supply rules for building sources it contributes
SDcard_Filehandling/%.o SDcard_Filehandling/%.su SDcard_Filehandling/%.cyclo: ../SDcard_Filehandling/%.c SDcard_Filehandling/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/Embedded_CLI" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/Internal_RTC_Handling" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/Libs" -I"C:/Users/hzaib/Documents/Freelance Projects/Mughees proj/Enviornmental_Sensor/_Enviornmental_Sensor_V1/SDcard_Filehandling" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-SDcard_Filehandling

clean-SDcard_Filehandling:
	-$(RM) ./SDcard_Filehandling/FATFSLNRequired.cyclo ./SDcard_Filehandling/FATFSLNRequired.d ./SDcard_Filehandling/FATFSLNRequired.o ./SDcard_Filehandling/FATFSLNRequired.su ./SDcard_Filehandling/File_Handling.cyclo ./SDcard_Filehandling/File_Handling.d ./SDcard_Filehandling/File_Handling.o ./SDcard_Filehandling/File_Handling.su ./SDcard_Filehandling/csv_json_handling.cyclo ./SDcard_Filehandling/csv_json_handling.d ./SDcard_Filehandling/csv_json_handling.o ./SDcard_Filehandling/csv_json_handling.su

.PHONY: clean-SDcard_Filehandling

