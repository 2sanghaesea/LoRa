################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32f103rbtx.s 

OBJS += \
./Core/Startup/startup_stm32f103rbtx.o 

S_DEPS += \
./Core/Startup/startup_stm32f103rbtx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m3 -g3 -DDEBUG -c -I"C:/Users/user/STM32CubeIDE/sx1276/SX1276_DHT11_SSD/Core/Inc" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32f103rbtx.d ./Core/Startup/startup_stm32f103rbtx.o

.PHONY: clean-Core-2f-Startup
