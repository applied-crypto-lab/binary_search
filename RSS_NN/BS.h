#ifndef BS_H_
#define BS_H_
#include <stdio.h>
#include <iostream>
#include <string>
#include "sys/time.h"
#include "Rss_Op.h"
//#include "neural_ops.h"
//#include "svm_ops.h"
#include "init.h"
extern "C"{
#include "aes_ni.h"
}
#define GET_BIT_TEST(X, N) ( ( (X) >> (N) ) & 1 )

//void test(NodeNetwork*, NodeConfiguration*, uint, uint);

//void test2(NodeNetwork *nNet, NodeConfiguration *nodeConfig, int size);
void bs5(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint* tar, Lint* res);
void bs1(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint* tar, Lint* res);
void BS(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint batch_size, uint alpha, uint alpha2);
void bs15(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint* tar, Lint* res);
void hybrid(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint* tar, Lint* res);
void Modifiedbs1(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint blockSize, uint ring_size, int* map, Lint** array, Lint* tar, Lint** res, unsigned long* timer, unsigned long* Commtimer);
void Modifiedbs5(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint blockSize, uint ring_size, int* map, Lint** array, Lint* tar, Lint** res, unsigned long* timer, unsigned long* Commtimer);
void bswrite(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint** bitarray, Lint* value, unsigned long* localtimer);
void bs1write(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint* tar, Lint* value, unsigned long* timer, unsigned long* localtimer);
void bs5write(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint* tar, Lint* value, unsigned long* timer);
void Mbs1write(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint blockSize, uint ring_size, int* map, Lint** array, Lint* tar, Lint** res, Lint** bitarray, unsigned long* timer, unsigned long* Commtimer, bool outputblock);
void hybridbswrite(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size1, uint size2, uint orig_size, uint ring_size, int* map, Lint** bitarray1, Lint** bitarray2, Lint** array, Lint* value, unsigned long* timer, unsigned long* localtimer);
#endif
