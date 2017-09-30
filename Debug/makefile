################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include FreeRTOS/utilities/subdir.mk
-include FreeRTOS/FreeRTOS_v9.0.0/portable/MemMang/subdir.mk
-include FreeRTOS/FreeRTOS_v9.0.0/portable/GCC/ARM_CA9/subdir.mk
-include FreeRTOS/FreeRTOS_v9.0.0/subdir.mk
-include FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/subdir.mk
-include FreeRTOS/FreeRTOS-Plus-TCP/portable/BufferManagement/subdir.mk
-include FreeRTOS/FreeRTOS-Plus-TCP/subdir.mk
-include Altera_Code/SoCSupport/subdir.mk
-include Altera_Code/HardwareLibrary/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: RTOSDemo.elf

# Tool invocations
RTOSDemo.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	arm-altera-eabi-gcc -Xlinker -TcycloneV-dk-ram.ld -Xlinker --defsym=__cs3_isr_irq=FreeRTOS_IRQ_Handler -Xlinker --defsym=__cs3_isr_swi=FreeRTOS_SWI_Handler -Xlinker -Map=RTOSDemo.map -Xlinker --gc-sections -Xlinker --allow-multiple-definition -o "RTOSDemo.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES) $(OBJS) $(C_DEPS) RTOSDemo.elf 
	-@echo ' '

post-build:
	-arm-altera-eabi-size rtosdemo.elf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
