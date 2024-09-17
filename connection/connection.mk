#ADD inputs and output to the build variables
CPP_SRCS += \
../connection/NodeConfiguration.cpp \
../connection/NodeNetwork.cpp 

OBJS += \
./connection/NodeConfiguration.o \
./connection/NodeNetwork.o

CPP_DEPS += \
./connection/NodeConfiguration.d \
./connection/NodeNetwork.d

CXXFLAGS = -maes -msse4.1 -mbmi2 -g -O3

#Supply the rules for building the source
smc-compute/%.o: ../connection/%.cpp
	@echo 'Building file:'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
