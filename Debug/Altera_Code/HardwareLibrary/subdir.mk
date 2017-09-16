################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Altera_Code/HardwareLibrary/alt_16550_uart.c \
../Altera_Code/HardwareLibrary/alt_address_space.c \
../Altera_Code/HardwareLibrary/alt_bridge_manager.c \
../Altera_Code/HardwareLibrary/alt_cache.c \
../Altera_Code/HardwareLibrary/alt_clock_manager.c \
../Altera_Code/HardwareLibrary/alt_dma.c \
../Altera_Code/HardwareLibrary/alt_dma_program.c \
../Altera_Code/HardwareLibrary/alt_ecc.c \
../Altera_Code/HardwareLibrary/alt_fpga_manager.c \
../Altera_Code/HardwareLibrary/alt_generalpurpose_io.c \
../Altera_Code/HardwareLibrary/alt_globaltmr.c \
../Altera_Code/HardwareLibrary/alt_i2c.c \
../Altera_Code/HardwareLibrary/alt_interrupt.c \
../Altera_Code/HardwareLibrary/alt_mmu.c \
../Altera_Code/HardwareLibrary/alt_nand.c \
../Altera_Code/HardwareLibrary/alt_qspi.c \
../Altera_Code/HardwareLibrary/alt_reset_manager.c \
../Altera_Code/HardwareLibrary/alt_sdmmc.c \
../Altera_Code/HardwareLibrary/alt_spi.c \
../Altera_Code/HardwareLibrary/alt_system_manager.c \
../Altera_Code/HardwareLibrary/alt_timers.c \
../Altera_Code/HardwareLibrary/alt_watchdog.c 

OBJS += \
./Altera_Code/HardwareLibrary/alt_16550_uart.o \
./Altera_Code/HardwareLibrary/alt_address_space.o \
./Altera_Code/HardwareLibrary/alt_bridge_manager.o \
./Altera_Code/HardwareLibrary/alt_cache.o \
./Altera_Code/HardwareLibrary/alt_clock_manager.o \
./Altera_Code/HardwareLibrary/alt_dma.o \
./Altera_Code/HardwareLibrary/alt_dma_program.o \
./Altera_Code/HardwareLibrary/alt_ecc.o \
./Altera_Code/HardwareLibrary/alt_fpga_manager.o \
./Altera_Code/HardwareLibrary/alt_generalpurpose_io.o \
./Altera_Code/HardwareLibrary/alt_globaltmr.o \
./Altera_Code/HardwareLibrary/alt_i2c.o \
./Altera_Code/HardwareLibrary/alt_interrupt.o \
./Altera_Code/HardwareLibrary/alt_mmu.o \
./Altera_Code/HardwareLibrary/alt_nand.o \
./Altera_Code/HardwareLibrary/alt_qspi.o \
./Altera_Code/HardwareLibrary/alt_reset_manager.o \
./Altera_Code/HardwareLibrary/alt_sdmmc.o \
./Altera_Code/HardwareLibrary/alt_spi.o \
./Altera_Code/HardwareLibrary/alt_system_manager.o \
./Altera_Code/HardwareLibrary/alt_timers.o \
./Altera_Code/HardwareLibrary/alt_watchdog.o 

C_DEPS += \
./Altera_Code/HardwareLibrary/alt_16550_uart.d \
./Altera_Code/HardwareLibrary/alt_address_space.d \
./Altera_Code/HardwareLibrary/alt_bridge_manager.d \
./Altera_Code/HardwareLibrary/alt_cache.d \
./Altera_Code/HardwareLibrary/alt_clock_manager.d \
./Altera_Code/HardwareLibrary/alt_dma.d \
./Altera_Code/HardwareLibrary/alt_dma_program.d \
./Altera_Code/HardwareLibrary/alt_ecc.d \
./Altera_Code/HardwareLibrary/alt_fpga_manager.d \
./Altera_Code/HardwareLibrary/alt_generalpurpose_io.d \
./Altera_Code/HardwareLibrary/alt_globaltmr.d \
./Altera_Code/HardwareLibrary/alt_i2c.d \
./Altera_Code/HardwareLibrary/alt_interrupt.d \
./Altera_Code/HardwareLibrary/alt_mmu.d \
./Altera_Code/HardwareLibrary/alt_nand.d \
./Altera_Code/HardwareLibrary/alt_qspi.d \
./Altera_Code/HardwareLibrary/alt_reset_manager.d \
./Altera_Code/HardwareLibrary/alt_sdmmc.d \
./Altera_Code/HardwareLibrary/alt_spi.d \
./Altera_Code/HardwareLibrary/alt_system_manager.d \
./Altera_Code/HardwareLibrary/alt_timers.d \
./Altera_Code/HardwareLibrary/alt_watchdog.d 


# Each subdirectory must supply rules for building sources it contributes
Altera_Code/HardwareLibrary/%.o: ../Altera_Code/HardwareLibrary/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-altera-eabi-gcc -I"E:\Home\cyclone\FreeRTOS\FreeRTOS_v9.0.0\include" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\portable\NetworkInterface" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\portable\NetworkInterface\Cyclone_V_SoC" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\portable\Compiler\GCC" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\include" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\protocols\include" -I"E:\Home\cyclone\Demo\Common\include" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS_v9.0.0\FreeRTOS-Plus\Source\FreeRTOS-Plus-CLI" -I"E:\Home\cyclone\Altera_Code\SoCSupport\include" -I"E:\Home\cyclone\Altera_Code\HardwareLibrary\include" -I"E:\Home\cyclone" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS_v9.0.0\portable\GCC\ARM_CA9" -O0 -g3 -Wall -c -fmessage-length=0 -mfloat-abi=softfp -mtune=cortex-a9 -mcpu=cortex-a9 -march=armv7-a -mfpu=neon -std=c99 -fdata-sections -ffunction-sections -g3 -DALT_INT_PROVISION_VECTOR_SUPPORT=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


