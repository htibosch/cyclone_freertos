################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/NetworkInterface.c \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/uncached_memory.c \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_dma.c \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_hw.c \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_physpeed.c 

O_SRCS += \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/NetworkInterface.o \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_dma.o \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_hw.o \
../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_physpeed.o 

OBJS += \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/NetworkInterface.o \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/uncached_memory.o \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_dma.o \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_hw.o \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_physpeed.o 

C_DEPS += \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/NetworkInterface.d \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/uncached_memory.d \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_dma.d \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_hw.d \
./FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/x_emacpsif_physpeed.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/%.o: ../FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/Zynq/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-altera-eabi-gcc -I"E:\Home\cyclone\FreeRTOS\FreeRTOS_v9.0.0\include" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\portable\Compiler\GCC" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\include" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\protocols\include" -I"E:\Home\cyclone\Demo\Common\include" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS_v9.0.0\FreeRTOS-Plus\Source\FreeRTOS-Plus-CLI" -I"E:\Home\cyclone\Altera_Code\SoCSupport\include" -I"E:\Home\cyclone\Altera_Code\HardwareLibrary\include" -I"E:\Home\cyclone" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS_v9.0.0\portable\GCC\ARM_CA9" -O0 -g3 -Wall -c -fmessage-length=0 -mfloat-abi=softfp -mtune=cortex-a9 -mcpu=cortex-a9 -march=armv7-a -mfpu=neon -std=c99 -fdata-sections -ffunction-sections -g3 -DALT_INT_PROVISION_VECTOR_SUPPORT=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


