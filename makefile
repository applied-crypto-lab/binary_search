# All of the sources participating in the build are defined here
CC = gcc

-include connection/connection.mk
-include RSS_NN/RSS_NN.mk
-include objects.mk

#Add test program to the build variables
CPP_SRCS += \
#test-code.cpp 
#../RSS_NN/Program.c \

OBJS += \
#./RSS_NN/Program.o \
#test-code.o 

#gcc -c AESNI_EN.c  -march=native

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif



all: Rss_nn_test bs
#Mult_100000 Mult_10000 Mult_1000 Mult_100 Mult_10 Mult_1 
#multit: $(OBJS) $(USER_OBJS) $(RSS_OBJS)
#	g++ -c -w multithread_trans.cpp
#	g++ -o multit multithread_trans.o -I ./ompi/include/ompi $(OBJS) $(USER_OBJS) $(RSS_OBJS) $(LIBS) -L ./ompi/lib/ompi -L ./ompi/lib/ompi/default libort.a -lrt 
#	g++ -o multit multithread_trans.o $(OBJS) $(USER_OBJS) $(LIBS

bs: $(OBJS) $(USER_OBJS) $(RSS_OBJS)
	g++ -c -g -O3 -w -msse4.1 -maes -mbmi2 bs.cpp
	g++ -o  bs bs.o $(RSS_OBJS) $(OBJS) $(USER_OBJS) $(LIBS)

Rss_nn_test: $(OBJS) $(USER_OBJS) $(RSS_OBJS)
	g++ -O3 -c -w -msse4.1 -maes -mbmi2 Rss_nn_test.cpp
	g++ -O3 -o  Rss_nn_test Rss_nn_test.o $(RSS_OBJS) $(OBJS) $(USER_OBJS) $(LIBS)




clean:
	-$(RM) *.o Rss_nn_test

#Clean-Everything
clean-all:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS)$(RSS_OBJS) *.o Rss_nn_test

