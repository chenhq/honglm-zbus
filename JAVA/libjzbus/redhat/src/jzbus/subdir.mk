################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/jzbus/Connection.cpp \
../src/jzbus/Context.cpp \
../src/jzbus/Worker.cpp \
../src/jzbus/jzbus.cpp 

OBJS += \
./src/jzbus/Connection.o \
./src/jzbus/Context.o \
./src/jzbus/Worker.o \
./src/jzbus/jzbus.o 

CPP_DEPS += \
./src/jzbus/Connection.d \
./src/jzbus/Context.d \
./src/jzbus/Worker.d \
./src/jzbus/jzbus.d 


# Each subdirectory must supply rules for building sources it contributes
src/jzbus/%.o: ../src/jzbus/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


