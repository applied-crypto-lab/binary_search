#ifndef SVM_OP_H_
#define SVM_OP_H_
#include <stdio.h>

#include "../connection/NodeNetwork.h"
#include "Rss_Op.h"
#include "sys/time.h"
#include "cmath"


// SVM operations
// void maximum(Lint** res, Lint **a, uint ring_size, uint size,  int *map, NodeNetwork* nodeNet);
void maximum(Lint** res, Lint **a, uint ring_size, uint size, uint batch_size, int *map, NodeNetwork* nodeNet);

void arg_max(Lint** res, Lint** res_index, Lint **a, uint ring_size, uint size,  uint batch_size, int *map, NodeNetwork* nodeNet);

void arg_max_helper(Lint** res, Lint** res_index, Lint **a, Lint **a_index, uint ring_size, uint size,  uint batch_size, int *map, NodeNetwork* nodeNet);


#endif
