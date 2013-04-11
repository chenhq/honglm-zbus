################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/zbox/hash.cpp \
../src/zbox/list.cpp \
../src/zbox/prelude.cpp \
../src/zbox/zbusapi.cpp \
../src/zbox/zmsg.cpp 

OBJS += \
./src/zbox/hash.o \
./src/zbox/list.o \
./src/zbox/prelude.o \
./src/zbox/zbusapi.o \
./src/zbox/zmsg.o 

CPP_DEPS += \
./src/zbox/hash.d \
./src/zbox/list.d \
./src/zbox/prelude.d \
./src/zbox/zbusapi.d \
./src/zbox/zmsg.d 


# Each subdirectory must supply rules for building sources it contributes
src/zbox/%.o: ../src/zbox/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I$(PYTHON_INCLUDE) -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


