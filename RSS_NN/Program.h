#ifndef Program_H_
#define Program_H_
// #include <x86intrin.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include "sys/time.h"
#include "Rss_Op.h"
#include "neural_ops.h"
#include "svm_ops.h"
#include "init.h"
extern "C"{
#include "aes_ni.h"
}
#define GET_BIT_TEST(X, N) ( ( (X) >> (N) ) & 1 )

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_GREEN "\x1b[32m"

void test(NodeNetwork*, NodeConfiguration*, uint, uint);

void test2(NodeNetwork *nNet, NodeConfiguration *nodeConfig, int size);
#endif
