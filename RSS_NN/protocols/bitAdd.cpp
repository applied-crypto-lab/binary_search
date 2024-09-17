#include "../Rss_Op.h"
extern "C" {
#include "../aes_ni.h"
}

// need way to tell which parties' bitwise shares we're adding
// e.g. 1st add p_1 and p_2 to get p'
// then add p' and p_3
void Rss_nBitAdd(Lint **res, Lint **r_bitwise, uint ring_size, uint size, int *map, NodeNetwork *nodeNet) {

    uint i, j;
    uint numParties = nodeNet->getNumParties();
    uint rounds = ceil(log2(numParties));

    Lint **a = new Lint *[2];
    Lint **b = new Lint *[2];
    for (i = 0; i < 2; i++) {
        a[i] = new Lint[size];
        b[i] = new Lint[size];
    }

    // always will be 2 rounds for 3pc
    for (j = 0; j < rounds; j++) {

        // if this is the first iteration, we copy r_bitwise into a and b
        if (j == 0) {
            // copy p_1 and p_2 into a, b respectively
            for (i = 0; i < size; i++) {
                memcpy(a[0] + i, r_bitwise[0] + 3 * i, sizeof(Lint));
                memcpy(a[1] + i, r_bitwise[1] + 3 * i, sizeof(Lint));

                memcpy(b[0] + i, r_bitwise[0] + 3 * i + 1, sizeof(Lint));
                memcpy(b[1] + i, r_bitwise[1] + 3 * i + 1, sizeof(Lint));
            }
            Rss_BitAdd(res, a, b, ring_size, size, map, nodeNet);

        } else {
            // we only need to copy r_bitwise into b
            for (i = 0; i < size; i++) {
                memcpy(a[0] + i, res[0] + i, sizeof(Lint));
                memcpy(a[1] + i, res[1] + i, sizeof(Lint));

                memcpy(b[0] + i, r_bitwise[0] + 3 * i + 2, sizeof(Lint));
                memcpy(b[1] + i, r_bitwise[1] + 3 * i + 2, sizeof(Lint));
            }
            Rss_BitAdd(res, a, b, ring_size, size, map, nodeNet);
        }
    }

    for (i = 0; i < 2; i++) {
        delete[] a[i];
        delete[] b[i];
    }
    delete[] a;
    delete[] b;
}

// alternative BitAdd implementation when both a and b are secret shared
// used in edaBit
void Rss_BitAdd(Lint **res, Lint **a, Lint **b, uint ring_size, uint size, int *map, NodeNetwork *nodeNet) {

    Lint i, j;
    int pid = nodeNet->getID();

    Lint **d = new Lint *[4];
    for (i = 0; i < 4; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }
    struct timeval start;
    struct timeval end;
    unsigned long timer;

    // gettimeofday(&start, NULL); //start timer here
    Rss_Mult_Bitwise(res, a, b, size, ring_size, map, nodeNet);
    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for Rss_Mult_Bitwise with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));

    for (i = 0; i < size; i++) {
        d[0][i] = a[0][i] ^ b[0][i];
        d[1][i] = a[1][i] ^ b[1][i];

        d[2][i] = res[0][i];
        d[3][i] = res[1][i];
    }

    // Rss_CircleOpL(d, ring_size, size, map, nodeNet);

    // gettimeofday(&start, NULL); //start timer here

    Rss_CircleOpL_Lint(d, ring_size, size, map, nodeNet); // new version w Lints
    // Rss_CircleOpL(d, ring_size, size, map, nodeNet); // original

    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for Rss_CircleOpL with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));

    for (i = 0; i < size; i++) {

        res[0][i] = (a[0][i] ^ b[0][i]) ^ (d[2][i] << 1);
        res[1][i] = (a[1][i] ^ b[1][i]) ^ (d[3][i] << 1);
    }

    for (i = 0; i < 4; i++) {
        delete[] d[i];
    }
    delete[] d;
}

// a is public, b is bitwise-shared
// res will be a bitwise shared output
void Rss_BitAdd(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, int *map, NodeNetwork *nodeNet) {

    Lint i, j;
    int pid = nodeNet->getID();

    Lint **d = new Lint *[4];
    for (i = 0; i < 4; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }

    struct timeval start;
    struct timeval end;
    unsigned long timer;

    Lint a1, a2;
    switch (pid) {
    case 1:
        a1 = -1;
        a2 = 0;
        break;
    case 2:
        a1 = 0;
        a2 = 0;
        break;
    case 3:
        a1 = 0;
        a2 = -1;
        break;
    }
    for (i = 0; i < size; i++) {
        d[0][i] = (a[i] & a1) ^ b[0][i];
        d[1][i] = (a[i] & a2) ^ b[1][i];

        d[2][i] = (a[i] & b[0][i]);
        d[3][i] = (a[i] & b[1][i]);
    }

    // gettimeofday(&start, NULL); //start timer here
    Rss_CircleOpL_Lint(d, ring_size, size, map, nodeNet);
    // Rss_CircleOpL(d, ring_size, size, map, nodeNet);
    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for Rss_CircleOpL with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));

    // we only need the values in the g position (indices 2 and 3)

    for (i = 0; i < size; i++) {
        res[0][i] = ((a[i] & a1) ^ b[0][i]) ^ (d[2][i] << 1);
        res[1][i] = ((a[i] & a2) ^ b[1][i]) ^ (d[3][i] << 1);
    }

    for (i = 0; i < 4; i++) {
        delete[] d[i];
    }
    delete[] d;
}

void Rss_CircleOpL(Lint **d, uint r_size, uint size, int *map, NodeNetwork *nodeNet) {

    Lint i, j, l, k, y, z, op_r; // used for loops

    struct timeval start;
    struct timeval end;
    unsigned long timer;

    if (r_size > 1) {

        // just three nested for-loops
        // r_size <=> k in algorithm

        uint num = ((r_size + 7) >> 3) * size;
        uint n_uints = ((r_size + 7) >> 3);
        uint t_index;
        Lint mask2, mask1m8, mask2m8;
        Lint mask1p, mask2p;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];
        Lint **buffer = new Lint *[4];
        // we need at most (r_size + 7)/8 bytes to store bits from the buffer
        uint8_t **a = new uint8_t *[2];
        uint8_t **b = new uint8_t *[2];
        uint8_t **u = new uint8_t *[2];

        for (i = 0; i < 2; i++) {
            index_array[i] = new uint[r_size_2];
            memset(index_array[i], 0, sizeof(uint) * r_size_2);
            buffer[2 * i] = new Lint[size];
            buffer[2 * i + 1] = new Lint[size];
            memset(buffer[2 * i], 0, sizeof(Lint) * size);
            memset(buffer[2 * i + 1], 0, sizeof(Lint) * size);
            // memsets are actually needed here since are ORing
            a[i] = new uint8_t[num];
            memset(a[i], 0, sizeof(uint8_t) * num);
            b[i] = new uint8_t[num];
            memset(b[i], 0, sizeof(uint8_t) * num);
            u[i] = new uint8_t[num];
            memset(u[i], 0, sizeof(uint8_t) * num);
        }

        for (i = 1; i <= rounds; i++) {
            gettimeofday(&start, NULL); //start timer here

            op_r = 0; // number of operations in a round
            // equivalent to the new_ring_size in MSB

            for (j = 1; j <= ceil(r_size_2 / pow(2, i)); j++) {

                y = uint(pow(2, i - 1) + j * pow(2, i)) % r_size_2;

                for (z = 1; z <= (pow(2, i - 1)); z++) {
                    // checking we have a valid set of indices to add to our set
                    if ((((y % r_size_2)) <= r_size) && (((y + z) % (r_size_2 + 1)) <= r_size)) {
                        // printf("y : %u\n", y);
                        // printf("y+z : %u\n", y+z);
                        index_array[0][op_r] = (y % r_size_2) - 1;
                        index_array[1][op_r] = ((y + z) % (r_size_2 + 1)) - 1;
                        op_r++;
                    }
                }
            }

            // updating parameters for optimization
            n_uints = ((2 * op_r + 7) >> 3);
            num = ((2 * op_r + 7) >> 3) * size;

            // extracting terms into buffer
            CarryBuffer2(buffer, d, index_array, size, op_r);

            // Splitting the buffer into bytes
            for (j = 0; j < size; ++j) {
                memcpy(a[0] + j * n_uints, buffer[0] + j, n_uints);
                memcpy(a[1] + j * n_uints, buffer[1] + j, n_uints);
                memcpy(b[0] + j * n_uints, buffer[2] + j, n_uints);
                memcpy(b[1] + j * n_uints, buffer[3] + j, n_uints);
            }

            // bitwise multiplication
            Rss_Mult_Byte(u, a, b, num, map, nodeNet);

            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {
                    // loop constants
                    t_index = (j >> 2) + (l * n_uints);
                    mask2 = index_array[1][j];
                    mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1) & 7;

                    d[0][l] = SET_BIT(d[0][l], mask2, GET_BIT(u[0][t_index], mask1m8));
                    d[1][l] = SET_BIT(d[1][l], mask2, GET_BIT(u[1][t_index], mask1m8));

                    // simplified from needing two separate loops
                    d[2][l] = SET_BIT(d[2][l], mask2, (GET_BIT(u[0][t_index], mask2m8) ^ GET_BIT(d[2][l], mask2)));
                    d[3][l] = SET_BIT(d[3][l], mask2, (GET_BIT(u[1][t_index], mask2m8) ^ GET_BIT(d[3][l], mask2)));
                }
            }
            gettimeofday(&end, NULL); //stop timer here
            timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
            // printf("Runtime for round with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));
        }

        for (i = 0; i < 2; i++) {
            delete[] buffer[2 * i];
            delete[] buffer[2 * i + 1];
            delete[] a[i];
            delete[] b[i];
            delete[] u[i];
            delete[] index_array[i];
        }

        delete[] a;
        delete[] b;
        delete[] u;
        delete[] index_array;
        delete[] buffer;
    }
}

void Rss_CircleOpL_Lint(Lint **d, uint r_size, uint size, int *map, NodeNetwork *nodeNet) {

    Lint i, j, l, y, z, op_r; // used for loops
    struct timeval start;
    struct timeval end;
    unsigned long timer;

    if (r_size > 1) {

        Lint mask2, mask1m8, mask2m8;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];
        Lint **a_prime = new Lint *[2];
        Lint **b_prime = new Lint *[2];
        Lint **u_prime = new Lint *[2];

        for (i = 0; i < 2; i++) {
            index_array[i] = new uint[r_size_2];
            a_prime[i] = new Lint[size];
            b_prime[i] = new Lint[size];
            u_prime[i] = new Lint[size];
        }

        for (i = 1; i <= rounds; i++) {
            gettimeofday(&start, NULL); //start timer here

            op_r = 0; // number of operations in a round
            // equivalent to the new_ring_size in MSB

            for (j = 1; j <= ceil(r_size_2 / pow(2, i)); j++) {

                y = uint(pow(2, i - 1) + j * pow(2, i)) % r_size_2;

                for (z = 1; z <= (pow(2, i - 1)); z++) {
                    // checking we have a valid set of indices to add to our set
                    if ((((y % r_size_2)) <= r_size) && (((y + z) % (r_size_2 + 1)) <= r_size)) {
                        // printf("y : %u\n", y);
                        // printf("y+z : %u\n", y+z);
                        index_array[0][op_r] = (y % r_size_2) - 1;
                        index_array[1][op_r] = ((y + z) % (r_size_2 + 1)) - 1;
                        op_r++;
                    }
                }
            }

            // extracting terms into a_prime and b_prime directly
            CarryBuffer_Lint(a_prime, b_prime, d, index_array, size, op_r);

            // bitwise multiplication
            Rss_Mult_Bitwise(u_prime, a_prime, b_prime, size, r_size, map, nodeNet);

            // printf("adding g2j+1\n");
            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {

                    // loop constants
                    // DO NOT REMOVE mask2
                    // putting it directly in next  operations
                    // of SET BIT causes it to FAIL
                    // DONT ASK WHY

                    mask2 = index_array[1][j]; // CHECK
                    mask1m8 = (2 * j);         // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1);

                    d[0][l] = SET_BIT(d[0][l], mask2, GET_BIT(u_prime[0][l], mask1m8));
                    d[1][l] = SET_BIT(d[1][l], mask2, GET_BIT(u_prime[1][l], mask1m8));

                    d[2][l] = SET_BIT(d[2][l], mask2, (GET_BIT(u_prime[0][l], mask2m8) ^ GET_BIT(d[2][l], mask2)));
                    d[3][l] = SET_BIT(d[3][l], mask2, (GET_BIT(u_prime[1][l], mask2m8) ^ GET_BIT(d[3][l], mask2)));
                }
            }
            gettimeofday(&end, NULL); //stop timer here
            timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
            // printf("Runtime for round with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));
        }
        for (i = 0; i < 2; i++) {
            delete[] a_prime[i];
            delete[] b_prime[i];
            delete[] u_prime[i];
            delete[] index_array[i];
        }
        delete[] a_prime;
        delete[] b_prime;
        delete[] u_prime;
        delete[] index_array;
    }
}

void CarryBuffer2(Lint **buffer, Lint **d, uint **index_array, uint size, uint k) {
    // prepares input u for multiplication
    // extracts p2i, p2i-1, and g2i
    // buffer and d are the same size (4 x size)

    Lint i, j, mask1, mask2, mask1p, mask2p;

    for (i = 0; i < size; i++) {
        for (j = 0; j < k; j++) {

            // used to set the bits in the correct positions in buffer
            mask1 = 2 * j;
            mask2 = 2 * j + 1;

            // used to get the correct bits from d
            mask1p = index_array[0][j];
            mask2p = index_array[1][j];

            buffer[0][i] = SET_BIT(buffer[0][i], mask1, GET_BIT(d[0][i], mask1p));
            buffer[1][i] = SET_BIT(buffer[1][i], mask1, GET_BIT(d[1][i], mask1p));
            buffer[0][i] = SET_BIT(buffer[0][i], mask2, GET_BIT(d[2][i], mask1p));
            buffer[1][i] = SET_BIT(buffer[1][i], mask2, GET_BIT(d[3][i], mask1p));

            buffer[2][i] = SET_BIT(buffer[2][i], mask1, GET_BIT(d[0][i], mask2p));
            buffer[3][i] = SET_BIT(buffer[3][i], mask1, GET_BIT(d[1][i], mask2p));
            buffer[2][i] = SET_BIT(buffer[2][i], mask2, GET_BIT(d[0][i], mask2p));
            buffer[3][i] = SET_BIT(buffer[3][i], mask2, GET_BIT(d[1][i], mask2p));
        }
    }
}

void CarryBuffer_Lint(Lint **a_prime, Lint **b_prime, Lint **d, uint **index_array, uint size, uint k) {

    Lint i, j, mask1, mask2, mask1p, mask2p;

    for (i = 0; i < size; i++) {
        for (j = 0; j < k; j++) {

            // used to set the bits in the correct positions in buffer
            mask1 = 2 * j;
            mask2 = 2 * j + 1;

            // used to get the correct bits from d
            mask1p = index_array[0][j];
            mask2p = index_array[1][j];

            a_prime[0][i] = SET_BIT(a_prime[0][i], mask1, GET_BIT(d[0][i], mask1p));
            a_prime[1][i] = SET_BIT(a_prime[1][i], mask1, GET_BIT(d[1][i], mask1p));
            a_prime[0][i] = SET_BIT(a_prime[0][i], mask2, GET_BIT(d[2][i], mask1p));
            a_prime[1][i] = SET_BIT(a_prime[1][i], mask2, GET_BIT(d[3][i], mask1p));

            b_prime[0][i] = SET_BIT(b_prime[0][i], mask1, GET_BIT(d[0][i], mask2p));
            b_prime[1][i] = SET_BIT(b_prime[1][i], mask1, GET_BIT(d[1][i], mask2p));
            b_prime[0][i] = SET_BIT(b_prime[0][i], mask2, GET_BIT(d[0][i], mask2p));
            b_prime[1][i] = SET_BIT(b_prime[1][i], mask2, GET_BIT(d[1][i], mask2p));
        }
    }
}

// this wont actually work
// fuck
// void OptimalBuffer_Lint(Lint **a_prime, Lint **b_prime, Lint **d, NodeNetwork *nodeNet, uint size, uint ring_size, uint round) {

//     Lint i, j, mask1, mask2, mask1p, mask2p;

//     for (i = 0; i < size; i++) {
//         mask1 = nodeNet->index_array[0][ring_size][round];
//         mask2 = nodeNet->index_array[1][ring_size][round];

//         a_prime[0][i] = a_prime[0][i] | (d[0][i] & mask1) ;
//         a_prime[1][i] = a_prime[1][i] | (d[1][i] & mask1) ;
//         a_prime[0][i] = a_prime[0][i] | (d[2][i] & mask1) ;
//         a_prime[1][i] = a_prime[1][i] | (d[3][i] & mask1) ;
//         b_prime[0][i] = b_prime[0][i] | (d[0][i] & mask2) ;
//         b_prime[1][i] = b_prime[1][i] | (d[1][i] & mask2) ;
//         b_prime[0][i] = b_prime[0][i] | (d[0][i] & mask2) ;
//         b_prime[1][i] = b_prime[1][i] | (d[1][i] & mask2) ;

//     }
// }