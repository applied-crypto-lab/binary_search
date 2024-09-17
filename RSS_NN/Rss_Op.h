#ifndef RSS_OP_H_
#define RSS_OP_H_
#include <stdio.h>

#include "../connection/NodeNetwork.h"
#include "sys/time.h"
#include "cmath"

void print_binary(Lint n, uint size);

void Rss_Open(Lint *res, Lint **a, uint size, int *map, uint ring_size, NodeNetwork *nodeNet);

void Rss_Open_Bitwise(Lint *res, Lint **a, uint size, int *map, uint ring_size, NodeNetwork *nodeNet);
void Rss_Open_s(Lint *res, Lint *a_0, Lint *a_1, uint size, int *map, uint ring_size, NodeNetwork *nodeNet);
void Rss_Open_Byte(uint8_t *res, uint8_t **a, uint size, int *map, uint ring_size, NodeNetwork *nodeNet);

void Rss_Mult(Lint** c, Lint** a, Lint** b, uint size, uint ring_size, int *map, NodeNetwork *nodeNet);
void Rss_Mult_Bitwise(Lint** c, Lint** a, Lint** b, uint size, uint ring_size, int *map, NodeNetwork *nodeNet);
void Rss_Mult_Byte(uint8_t** c, uint8_t** a, uint8_t** b, uint size, int *map, NodeNetwork *nodeNet);
void Rss_MultPub(Lint * c, Lint** a_a, Lint** b, uint size, int *map, uint ring_size, NodeNetwork* nodeNet);

void Rss_Dot(Lint* c, Lint** a, Lint** b, uint n, uint ring_size, int *map, NodeNetwork *nodeNet);
void Rss_DotArray(Lint** c, Lint** a, Lint** d, uint n, uint chunkSize, uint ring_size,  int *map, NodeNetwork *nodeNet, unsigned long* timer);

void Rss_MatMult(Lint*** c, Lint*** a, Lint*** b, uint m, uint n, uint s, uint ring_size, int *map, NodeNetwork *nodeNet);

void Rss_MatMultArray(Lint** c, Lint** a, Lint** b, uint m, uint n, uint s, uint ring_size, int *map, NodeNetwork *nodeNet);

void Rss_MatMultArray_batch(Lint** c, Lint** a, Lint** b, uint m, uint n, uint s, uint ring_size, uint batch_size, uint weight_flag, int *map, NodeNetwork *nodeNet);

void Rss_RandBit(Lint** b, uint size, uint ring_size, int *map, NodeNetwork* nodeNet);

void Rss_edaBit(Lint** r, Lint** b_2, uint size, uint ring_size, int *map, NodeNetwork* nodeNet);
void Rss_edaBit(Lint** r, Lint** b_2, uint size, uint ring_size, uint bit_length, int *map, NodeNetwork* nodeNet);

void test_rssop();

void Rss_MSB(Lint **res, Lint **a, uint size, uint ring_size, int *map, NodeNetwork *nodeNet);
void new_Rss_MSB(Lint **res, Lint **a, uint size, uint ring_size, int *map, NodeNetwork *nodeNet);

void Rss_GenerateRandomShares(Lint** res, Lint** r_i_values, uint ring_size, uint size, int *map, NodeNetwork* nodeNet);
void Rss_GenerateRandomShares(Lint** res, Lint** r_i_values, uint ring_size, uint bit_length, uint size, int *map, NodeNetwork* nodeNet);


void Rss_Convert(Lint **res, Lint** a, uint size, uint ring_size, uint ring_size_prime,int *map, NodeNetwork* nodeNet);
void new_Rss_Convert(Lint **res, Lint** a, uint size, uint ring_size, uint ring_size_prime,int *map, NodeNetwork* nodeNet);

// overloading BitAdd
void Rss_BitAdd(Lint** res, Lint* a, Lint** b, uint ring_size, uint size, int *map, NodeNetwork* nodeNet);
void Rss_BitAdd(Lint** res, Lint** a, Lint** b, uint ring_size, uint size, int *map, NodeNetwork* nodeNet);


void Rss_nBitAdd(Lint** res, Lint** r_bitwise, uint ring_size, uint size, int *map, NodeNetwork* nodeNet);

void Rss_CircleOpL(Lint **d, uint r_size, uint size, int *map, NodeNetwork *nodeNet);
void Rss_CircleOpL_Lint(Lint **d, uint r_size, uint size, int *map, NodeNetwork *nodeNet);
void CarryBuffer2(Lint **buffer, Lint **d, uint **index_array, uint size, uint k);
void CarryBuffer_Lint(Lint **a_prime, Lint **b_prime, Lint **d, uint **index_array, uint size, uint k);
void OptimalBuffer_Lint(Lint **a_prime, Lint **b_prime, Lint **d, uint **index_array, uint size, uint k);

void Rss_b2kprime(Lint** res, Lint** a, uint ring_size, uint ring_size_prime, uint size, int *map, NodeNetwork* nodeNet);



void Rss_BitLT(Lint **res, Lint* a, Lint** b, uint ring_size, uint size, int *map, NodeNetwork* nodeNet);
void Rss_LT(Lint **res, Lint** a, Lint** b, uint size, uint ring_size, int *map, NodeNetwork* nodeNet);
void Rss_JustLT(Lint **res, Lint** a, Lint** b, uint size, uint ring_size, int *map, NodeNetwork* nodeNet);
void new_Rss_LT(Lint **res, Lint** a, Lint** b, uint size, uint ring_size, int *map, NodeNetwork* nodeNet);

void Rss_CarryOut(Lint** res, Lint* a, Lint** b, uint ring_size, uint size, int *map, NodeNetwork* nodeNet);

void Rss_CarryOutAux(Lint** res,  Lint** d, uint r_size, uint size, int *map, NodeNetwork* nodeNet);
void new_Rss_CarryOutAux(Lint** res,  Lint** d, uint r_size, uint size, int *map, NodeNetwork* nodeNet);
void new_Rss_CarryOutAux_Lint(Lint** res,  Lint** d, uint r_size, uint size, int *map, NodeNetwork* nodeNet);

void CarryBuffer(Lint **buffer, Lint **u, uint size, uint r_size);
void OptimalBuffer(Lint **buffer, Lint **u, uint size, uint r_size, NodeNetwork* nodeNet);

void CircleOp(Lint **res, Lint *p, Lint *g, uint size, uint r_size);



void rss_sqrt_inv(Lint *c, Lint *e, uint size, uint ring_size);

void invert(Lint *c, Lint *a, int size, int ring_size);

void rss_sqrt(Lint *c, Lint *e, int size, int ring_size);


Lint bitExtracted(Lint number, uint k);
uint8_t smallBitExtracted(uint8_t a, uint k);

#endif
