#ifndef Nerual_H_
#define Nerual_H_
#include <stdio.h>
#include <iostream>
#include <string>
#include "sys/time.h"
#include "Rss_Op.h"
#include "neural_ops.h"
#include "svm_ops.h"
#include "init.h"
extern "C"{
#include "aes_ni.h"
}

void neural_main(NodeNetwork*, NodeConfiguration*, uint, uint);
#endif
