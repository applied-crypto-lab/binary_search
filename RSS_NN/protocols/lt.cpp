#include "../Rss_Op.h"
extern "C"{
#include "../aes_ni.h"
}

// returns the higher value
void Rss_LT(Lint **res, Lint** a, Lint** b, uint size, uint ring_size, int *map, NodeNetwork* nodeNet){

    uint i; // used for loops
    Lint **c = new Lint*[2];
    Lint **diff = new Lint*[2];
    Lint **d = new Lint*[2];

    for (i = 0; i < 2; i++) {
        c[i] = new Lint [size];
        diff[i] = new Lint [size];
        d[i] = new Lint [size];
    }

    for (i = 0; i < size; i++) {
        diff[0][i] = a[0][i] - b[0][i];
        diff[1][i] = a[1][i] - b[1][i];
    }
    Rss_MSB(c, diff, size, ring_size, map, nodeNet);

    for (i = 0; i < size; i++) {
        diff[0][i] = b[0][i] - a[0][i];
        diff[1][i] = b[1][i] - a[1][i];
    }
    Rss_Mult(d, c, diff, size, ring_size, map, nodeNet);

    // [d] = [c] * ([a] - [b]); [c] = MSB([a] - [b])
    // [c] * [b] + (1 - [c]) * [a] =  [c] * ([b] - [a]) + [a]
    for (i = 0; i < size; i++) {
        res[0][i] = d[0][i] + a[0][i];
        res[1][i] = d[1][i] + a[1][i]; 
    }


    for(i = 0; i < 2; i++){
        delete [] c[i];
        delete [] diff[i];
        delete [] d[i];
    }

    delete [] c;
    delete [] d;
    delete [] diff;
}

void Rss_JustLT(Lint **res, Lint** a, Lint** b, uint size, uint ring_size, int *map, NodeNetwork* nodeNet){

    uint i; // used for loops
    Lint **diff = new Lint*[2];
    Lint **d = new Lint*[2];

    for (i = 0; i < 2; i++) {
        diff[i] = new Lint [size];
        d[i] = new Lint [size];
    }

    for (i = 0; i < size; i++) {
        diff[0][i] = a[0][i] - b[0][i];
        diff[1][i] = a[1][i] - b[1][i];
    }
    Rss_MSB(res, diff, size, ring_size, map, nodeNet);

    for(i = 0; i < 2; i++){
        delete [] diff[i];
        delete [] d[i];
    }
    delete [] d;
    delete [] diff;
}



void new_Rss_LT(Lint **res, Lint** a, Lint** b, uint size, uint ring_size, int *map, NodeNetwork* nodeNet){

    uint i; // used for loops
    Lint **c = new Lint*[2];
    Lint **diff = new Lint*[2];
    Lint **d = new Lint*[2];

    for (i = 0; i < 2; i++) {
        c[i] = new Lint [size];
        diff[i] = new Lint [size];
        d[i] = new Lint [size];
    }

    for (i = 0; i < size; i++) {
        diff[0][i] = a[0][i] - b[0][i];
        diff[1][i] = a[1][i] - b[1][i];
    }
    new_Rss_MSB(c, diff, size, ring_size, map, nodeNet);

    for (i = 0; i < size; i++) {
        diff[0][i] = b[0][i] - a[0][i];
        diff[1][i] = b[1][i] - a[1][i];
    }
    Rss_Mult(d, c, diff, size, ring_size, map, nodeNet);

    // [d] = [c] * ([a] - [b]); [c] = MSB([a] - [b])
    // [c] * [b] + (1 - [c]) * [a] =  [c] * ([b] - [a]) + [a]
    for (i = 0; i < size; i++) {
        res[0][i] = d[0][i] + a[0][i];
        res[1][i] = d[1][i] + a[1][i]; 
    }


    for(i = 0; i < 2; i++){
        delete [] c[i];
        delete [] diff[i];
        delete [] d[i];
    }

    delete [] c;
    delete [] d;
    delete [] diff;
}
