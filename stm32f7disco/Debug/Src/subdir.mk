################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/app.c \
../Src/dma2d.c \
../Src/encoder.c \
../Src/fmc.c \
../Src/gpio.c \
../Src/i2c.c \
../Src/icm20948.c \
../Src/imu_min_test.c \
../Src/lcd_bsp_test.c \
../Src/lcd_display.c \
../Src/lcd_test.c \
../Src/ltdc.c \
../Src/madgwick.c \
../Src/main.c \
../Src/microros_transport.c \
../Src/motor.c \
../Src/stm32f7xx_hal_msp.c \
../Src/stm32f7xx_it.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/system_stm32f7xx.c \
../Src/tim.c \
../Src/usart.c 

OBJS += \
./Src/app.o \
./Src/dma2d.o \
./Src/encoder.o \
./Src/fmc.o \
./Src/gpio.o \
./Src/i2c.o \
./Src/icm20948.o \
./Src/imu_min_test.o \
./Src/lcd_bsp_test.o \
./Src/lcd_display.o \
./Src/lcd_test.o \
./Src/ltdc.o \
./Src/madgwick.o \
./Src/main.o \
./Src/microros_transport.o \
./Src/motor.o \
./Src/stm32f7xx_hal_msp.o \
./Src/stm32f7xx_it.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/system_stm32f7xx.o \
./Src/tim.o \
./Src/usart.o 

C_DEPS += \
./Src/app.d \
./Src/dma2d.d \
./Src/encoder.d \
./Src/fmc.d \
./Src/gpio.d \
./Src/i2c.d \
./Src/icm20948.d \
./Src/imu_min_test.d \
./Src/lcd_bsp_test.d \
./Src/lcd_display.d \
./Src/lcd_test.d \
./Src/ltdc.d \
./Src/madgwick.d \
./Src/main.d \
./Src/microros_transport.d \
./Src/motor.d \
./Src/stm32f7xx_hal_msp.d \
./Src/stm32f7xx_it.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/system_stm32f7xx.d \
./Src/tim.d \
./Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F746xx -DUSE_MICROROS -c -I../Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/STM32746G-Discovery -I../Drivers/BSP/Components/Common -I../Drivers/BSP/Components/ft5336 -I../Drivers/BSP/Components/rk043fn48h -I../Middlewares/micro_ros_stm32cubemx_utils/microros_static_library_ide/libmicroros/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/app.cyclo ./Src/app.d ./Src/app.o ./Src/app.su ./Src/dma2d.cyclo ./Src/dma2d.d ./Src/dma2d.o ./Src/dma2d.su ./Src/encoder.cyclo ./Src/encoder.d ./Src/encoder.o ./Src/encoder.su ./Src/fmc.cyclo ./Src/fmc.d ./Src/fmc.o ./Src/fmc.su ./Src/gpio.cyclo ./Src/gpio.d ./Src/gpio.o ./Src/gpio.su ./Src/i2c.cyclo ./Src/i2c.d ./Src/i2c.o ./Src/i2c.su ./Src/icm20948.cyclo ./Src/icm20948.d ./Src/icm20948.o ./Src/icm20948.su ./Src/imu_min_test.cyclo ./Src/imu_min_test.d ./Src/imu_min_test.o ./Src/imu_min_test.su ./Src/lcd_bsp_test.cyclo ./Src/lcd_bsp_test.d ./Src/lcd_bsp_test.o ./Src/lcd_bsp_test.su ./Src/lcd_display.cyclo ./Src/lcd_display.d ./Src/lcd_display.o ./Src/lcd_display.su ./Src/lcd_test.cyclo ./Src/lcd_test.d ./Src/lcd_test.o ./Src/lcd_test.su ./Src/ltdc.cyclo ./Src/ltdc.d ./Src/ltdc.o ./Src/ltdc.su ./Src/madgwick.cyclo ./Src/madgwick.d ./Src/madgwick.o ./Src/madgwick.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/microros_transport.cyclo ./Src/microros_transport.d ./Src/microros_transport.o ./Src/microros_transport.su ./Src/motor.cyclo ./Src/motor.d ./Src/motor.o ./Src/motor.su ./Src/stm32f7xx_hal_msp.cyclo ./Src/stm32f7xx_hal_msp.d ./Src/stm32f7xx_hal_msp.o ./Src/stm32f7xx_hal_msp.su ./Src/stm32f7xx_it.cyclo ./Src/stm32f7xx_it.d ./Src/stm32f7xx_it.o ./Src/stm32f7xx_it.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/system_stm32f7xx.cyclo ./Src/system_stm32f7xx.d ./Src/system_stm32f7xx.o ./Src/system_stm32f7xx.su ./Src/tim.cyclo ./Src/tim.d ./Src/tim.o ./Src/tim.su ./Src/usart.cyclo ./Src/usart.d ./Src/usart.o ./Src/usart.su

.PHONY: clean-Src

