################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/pyzbus/pyzbus.cpp 

OBJS += \
./src/pyzbus/pyzbus.o 

CPP_DEPS += \
./src/pyzbus/pyzbus.d 


# Each subdirectory must supply rules for building sources it contributes
src/pyzbus/%.o: ../src/pyzbus/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I$(PYTHON_INCLUDE) -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


