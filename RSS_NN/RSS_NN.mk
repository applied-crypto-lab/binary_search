RSS_SRCS += \
../RSS_NN/BS.cpp \
../RSS_NN/Program.cpp \
../RSS_NN/Neural.cpp \
../RSS_NN/init.cpp \
../RSS_NN/aes_ni.c \
../RSS_NN/protocols/open.cpp \
../RSS_NN/protocols/misc.cpp \
../RSS_NN/protocols/matMult.cpp \
../RSS_NN/protocols/mult.cpp \
../RSS_NN/protocols/msb.cpp \
../RSS_NN/protocols/randBit.cpp \
../RSS_NN/protocols/lt.cpp \
../RSS_NN/protocols/neural_ops.cpp \
../RSS_NN/protocols/svm_ops.cpp \
../RSS_NN/protocols/edaBit.cpp \
../RSS_NN/protocols/conversion.cpp \
../RSS_NN/protocols/bitAdd.cpp  \
../RSS_NN/protocols/bitbitLTAdd.cpp  \


RSS_OBJS += \
./RSS_NN/BS.o \
./RSS_NN/Program.o \
./RSS_NN/Neural.o \
./RSS_NN/init.o \
./RSS_NN/aes_ni.o \
./RSS_NN/protocols/open.o \
./RSS_NN/protocols/misc.o \
./RSS_NN/protocols/matMult.o \
./RSS_NN/protocols/mult.o \
./RSS_NN/protocols/msb.o \
./RSS_NN/protocols/randBit.o \
./RSS_NN/protocols/lt.o \
./RSS_NN/protocols/neural_ops.o \
./RSS_NN/protocols/svm_ops.o \
./RSS_NN/protocols/edaBit.o \
./RSS_NN/protocols/conversion.o \
./RSS_NN/protocols/bitAdd.o \
./RSS_NN/protocols/bitLT.o \


CPP_DEPS += \
../RSS_NN/BS.d \
./RSS_NN/Program.d \
./RSS_NN/Neural.d \
./RSS_NN/init.d \
./RSS_NN/aes_ni.d \
./RSS_NN/protocols/open.d \
./RSS_NN/protocols/misc.d \
./RSS_NN/protocols/matMult.d \
./RSS_NN/protocols/mult.d \
./RSS_NN/protocols/msb.d \
./RSS_NN/protocols/randBit.d \
./RSS_NN/protocols/lt.d \
./RSS_NN/protocols/neural_ops.d \
./RSS_NN/protocols/svm_ops.d \
./RSS_NN/protocols/edaBit.d \
./RSS_NN/protocols/conversion.d \
./RSS_NN/protocols/bitAdd.d  \
./RSS_NN/protocols/bitLT.d  \


CFLAGS = -g -maes -msse4.1 -mbmi2

#Supply the rules for building the source
RSS_NN/%.o: ../RSS_NN/%.c
	@echo 'Building file:'
	@echo 'Invoking: GCC C++ Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
