#ifndef init_H
#define init_H_
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include "sys/time.h"
#include "cmath"
#include "Rss_Op.h"
extern "C"{
#include "aes_ni.h"
}


void readFile(long long *output, uint rows, uint cols, Lint fractional, std::string path);
void readB(long long *output, uint rows, uint cols, Lint fractional, std::string path, uint n);
void readX(long long *output, uint rows, uint cols, std::string path);
// void readX(long long **output, uint rows, uint cols, Lint fractional, std::string path);


void splitData(Lint **output, long long  *input, uint m, uint n, uint ring_size, NodeNetwork* nodeNet);

void splitX(Lint **output, long long  *input, uint size, uint batch_size, uint tracker, uint ring_size, NodeNetwork* nodeNet);

#endif
