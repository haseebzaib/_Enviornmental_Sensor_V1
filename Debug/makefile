################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include USB_DEVICE/Target/subdir.mk
-include USB_DEVICE/App/subdir.mk
-include SDcard_Filehandling/subdir.mk
-include Middlewares/Third_Party/FatFs/src/option/subdir.mk
-include Middlewares/Third_Party/FatFs/src/subdir.mk
-include Middlewares/ST/STM32_USB_Device_Library/Core/Src/subdir.mk
-include Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Src/subdir.mk
-include Libs/subdir.mk
-include Internal_RTC_Handling/subdir.mk
-include FATFS/Target/subdir.mk
-include FATFS/App/subdir.mk
-include Embedded_CLI/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := _Enviornmental_Sensor_V1
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
_Enviornmental_Sensor_V1.elf \

MAP_FILES += \
_Enviornmental_Sensor_V1.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
_Enviornmental_Sensor_V1.list \

OBJCOPY_HEX += \
_Enviornmental_Sensor_V1.hex \

OBJCOPY_BIN += \
_Enviornmental_Sensor_V1.bin \


# All Target
all: main-build

# Main-build Target
main-build: _Enviornmental_Sensor_V1.elf secondary-outputs

# Tool invocations
_Enviornmental_Sensor_V1.elf _Enviornmental_Sensor_V1.map: $(OBJS) $(USER_OBJS) C:\Users\hzaib\Documents\Freelance\ Projects\Mughees\ proj\_Enviornmental_Sensor_V1\STM32F401RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "_Enviornmental_Sensor_V1.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\hzaib\Documents\Freelance Projects\Mughees proj\_Enviornmental_Sensor_V1\STM32F401RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="_Enviornmental_Sensor_V1.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float -u _scanf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

_Enviornmental_Sensor_V1.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "_Enviornmental_Sensor_V1.list"
	@echo 'Finished building: $@'
	@echo ' '

_Enviornmental_Sensor_V1.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "_Enviornmental_Sensor_V1.hex"
	@echo 'Finished building: $@'
	@echo ' '

_Enviornmental_Sensor_V1.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "_Enviornmental_Sensor_V1.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) _Enviornmental_Sensor_V1.bin _Enviornmental_Sensor_V1.elf _Enviornmental_Sensor_V1.hex _Enviornmental_Sensor_V1.list _Enviornmental_Sensor_V1.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
