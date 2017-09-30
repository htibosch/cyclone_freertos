################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/NetworkInterface.c \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/cyclone_dma.c \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/cyclone_emac.c \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/uncached_memory.c 

O_SRCS += \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/NetworkInterface.o 

OBJS += \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/NetworkInterface.o \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/cyclone_dma.o \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/cyclone_emac.o \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/uncached_memory.o 

C_DEPS += \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/NetworkInterface.d \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/cyclone_dma.d \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/cyclone_emac.d \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/uncached_memory.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/%.o: ../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Cyclone_V_SoC/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-altera-eabi-gcc -I"E:\Home\cyclone_hein\FreeRTOS\FreeRTOS_v9.0.0\include" -I"E:\Home\cyclone_hein\FreeRTOS\utilities\include" -I"E:\Home\cyclone_hein\FreeRTOS\FreeRTOS-Plus-TCP\portable\NetworkInterface" -I"E:\Home\cyclone_hein\FreeRTOS\FreeRTOS-Plus-TCP\portable\NetworkInterface\Cyclone_V_SoC" -I"E:\Home\cyclone_hein\FreeRTOS\FreeRTOS-Plus-TCP\portable\Compiler\GCC" -I"E:\Home\cyclone_hein\FreeRTOS\FreeRTOS-Plus-TCP\include" -I"E:\Home\cyclone_hein\FreeRTOS\FreeRTOS-Plus-TCP\protocols\include" -I"E:\Home\cyclone_hein\Demo\Common\include" -I"E:\Home\cyclone_hein\FreeRTOS\FreeRTOS_v9.0.0\FreeRTOS-Plus\Source\FreeRTOS-Plus-CLI" -I"E:\Home\cyclone_hein\Altera_Code\SoCSupport\include" -I"E:\Home\cyclone_hein\Altera_Code\HardwareLibrary\include" -I"E:\Home\cyclone_hein" -I"E:\Home\cyclone_hein\FreeRTOS\FreeRTOS_v9.0.0\portable\GCC\ARM_CA9" -O0 -g3 -Wall -c -fmessage-length=0 -mfloat-abi=softfp -mtune=cortex-a9 -mcpu=cortex-a9 -march=armv7-a -mfpu=neon -std=c99 -fdata-sections -ffunction-sections -g3 -DALT_INT_PROVISION_VECTOR_SUPPORT=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


