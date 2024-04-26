################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SHT31/CRC.c \
../SHT31/SHT31.c \
../SHT31/board_resources.c \
../SHT31/fonts.c \
../SHT31/ssd1306.c 

OBJS += \
./SHT31/CRC.o \
./SHT31/SHT31.o \
./SHT31/board_resources.o \
./SHT31/fonts.o \
./SHT31/ssd1306.o 

C_DEPS += \
./SHT31/CRC.d \
./SHT31/SHT31.d \
./SHT31/board_resources.d \
./SHT31/fonts.d \
./SHT31/ssd1306.d 


# Each subdirectory must supply rules for building sources it contributes
SHT31/%.o SHT31/%.su SHT31/%.cyclo: ../SHT31/%.c SHT31/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WL55xx -c -I../../Core/Inc -I"C:/Users/user/STM32CubeIDE/testlora/SubGHz_Phy_PingPong/STM32CubeIDE/SHT31" -I../../SubGHz_Phy/App -I../../SubGHz_Phy/Target -I../../Utilities/trace/adv_trace -I../../Drivers/STM32WLxx_HAL_Driver/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../../Utilities/misc -I../../Utilities/sequencer -I../../Utilities/timer -I../../Utilities/lpm/tiny_lpm -I../../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../../Middlewares/Third_Party/SubGHz_Phy -I../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../../Drivers/CMSIS/Include -I../../Drivers/BSP/STM32WLxx_Nucleo -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-SHT31

clean-SHT31:
	-$(RM) ./SHT31/CRC.cyclo ./SHT31/CRC.d ./SHT31/CRC.o ./SHT31/CRC.su ./SHT31/SHT31.cyclo ./SHT31/SHT31.d ./SHT31/SHT31.o ./SHT31/SHT31.su ./SHT31/board_resources.cyclo ./SHT31/board_resources.d ./SHT31/board_resources.o ./SHT31/board_resources.su ./SHT31/fonts.cyclo ./SHT31/fonts.d ./SHT31/fonts.o ./SHT31/fonts.su ./SHT31/ssd1306.cyclo ./SHT31/ssd1306.d ./SHT31/ssd1306.o ./SHT31/ssd1306.su

.PHONY: clean-SHT31

