################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS_v9.0.0/croutine.c \
../FreeRTOS_v9.0.0/event_groups.c \
../FreeRTOS_v9.0.0/list.c \
../FreeRTOS_v9.0.0/queue.c \
../FreeRTOS_v9.0.0/tasks.c \
../FreeRTOS_v9.0.0/timers.c 

O_SRCS += \
../FreeRTOS_v9.0.0/event_groups.o \
../FreeRTOS_v9.0.0/list.o \
../FreeRTOS_v9.0.0/queue.o \
../FreeRTOS_v9.0.0/tasks.o \
../FreeRTOS_v9.0.0/timers.o 

OBJS += \
./FreeRTOS_v9.0.0/croutine.o \
./FreeRTOS_v9.0.0/event_groups.o \
./FreeRTOS_v9.0.0/list.o \
./FreeRTOS_v9.0.0/queue.o \
./FreeRTOS_v9.0.0/tasks.o \
./FreeRTOS_v9.0.0/timers.o 

C_DEPS += \
./FreeRTOS_v9.0.0/croutine.d \
./FreeRTOS_v9.0.0/event_groups.d \
./FreeRTOS_v9.0.0/list.d \
./FreeRTOS_v9.0.0/queue.d \
./FreeRTOS_v9.0.0/tasks.d \
./FreeRTOS_v9.0.0/timers.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS_v9.0.0/%.o: ../FreeRTOS_v9.0.0/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-altera-eabi-gcc -I"E:\Home\cyclone\FreeRTOS_v9.0.0\include" -I"E:\Home\cyclone\Demo\Common\include" -I"E:\Home\cyclone\FreeRTOS_v9.0.0\FreeRTOS-Plus\Source\FreeRTOS-Plus-CLI" -I"E:\Home\cyclone\Altera_Code\SoCSupport\include" -I"E:\Home\cyclone\Altera_Code\HardwareLibrary\include" -I"E:\Home\cyclone" -I"E:\Home\cyclone\FreeRTOS_v9.0.0\portable\GCC\ARM_CA9" -O0 -g3 -Wall -c -fmessage-length=0 -mfloat-abi=softfp -mtune=cortex-a9 -mcpu=cortex-a9 -march=armv7-a -mfpu=neon -std=c99 -fdata-sections -ffunction-sections -g3 -DALT_INT_PROVISION_VECTOR_SUPPORT=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


