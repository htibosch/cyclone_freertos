################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/TCPEchoClients.c \
../FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/TCPEchoServer.c \
../FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/TwoEchoClients.c 

OBJS += \
./FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/TCPEchoClients.o \
./FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/TCPEchoServer.o \
./FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/TwoEchoClients.o 

C_DEPS += \
./FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/TCPEchoClients.d \
./FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/TCPEchoServer.d \
./FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/TwoEchoClients.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/%.o: ../FreeRTOS/FreeRTOS-Plus-TCP/EchoClients/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-altera-eabi-gcc -I"E:\Home\cyclone\FreeRTOS\FreeRTOS_v9.0.0\include" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\portable\NetworkInterface" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\portable\NetworkInterface\Cyclone_V_SoC" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\portable\Compiler\GCC" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\include" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS-Plus-TCP\protocols\include" -I"E:\Home\cyclone\Demo\Common\include" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS_v9.0.0\FreeRTOS-Plus\Source\FreeRTOS-Plus-CLI" -I"E:\Home\cyclone\Altera_Code\SoCSupport\include" -I"E:\Home\cyclone\Altera_Code\HardwareLibrary\include" -I"E:\Home\cyclone" -I"E:\Home\cyclone\FreeRTOS\FreeRTOS_v9.0.0\portable\GCC\ARM_CA9" -O0 -g3 -Wall -c -fmessage-length=0 -mfloat-abi=softfp -mtune=cortex-a9 -mcpu=cortex-a9 -march=armv7-a -mfpu=neon -std=c99 -fdata-sections -ffunction-sections -g3 -DALT_INT_PROVISION_VECTOR_SUPPORT=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


