#ifndef NEURAL_OP_H_
#define NEURAL_OP_H_
#include <stdio.h>

#include "../connection/NodeNetwork.h"
#include "Rss_Op.h"
#include "sys/time.h"

// neural network operations
void im2col(Lint** res, Lint** a, uint channels, uint height, uint width, uint ksize, uint stride, uint batch_size, int *map, NodeNetwork* nodeNet);

void ReLU(Lint** res, Lint** a,  uint size, uint ring_size, int *map, NodeNetwork* nodeNet);

void MaxPool(Lint** res, Lint** a, uint c, uint m, uint n, uint batch_size, uint ring_sze, int *map, NodeNetwork* nodeNet);

void add_biases(Lint** res, Lint **a, Lint **b, uint m, uint n, uint batch_size,  int *map, NodeNetwork* nodeNet);

#endif
