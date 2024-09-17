#include "../Rss_Op.h"
extern "C" {
#include "../aes_ni.h"
}

void Rss_edaBit(Lint **r, Lint **b_2, uint size, uint ring_size, int *map, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint numParties = nodeNet->getNumParties();
    // printf("numParties : %u\n",numParties);
    struct timeval start;
    struct timeval end;
    unsigned long timer;

    uint i;
    // need to multiply by the number of parties in the computation
    uint new_size = numParties * size;

    Lint **r_bitwise = new Lint *[2];
    for (i = 0; i < 2; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(b_2[i], 0, sizeof(Lint) * size);
    }
    // gettimeofday(&start, NULL); //start timer here

    Rss_GenerateRandomShares(r, r_bitwise, ring_size, size, map, nodeNet);
    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    // printf("Runtime for Rss_GenerateRandomShares with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));

    // gettimeofday(&start, NULL); //start timer here

    Rss_nBitAdd(b_2, r_bitwise, ring_size, size, map, nodeNet);

    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    // printf("Runtime for Rss_nBitAdd with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));

    for (i = 0; i < 2; i++) {
        delete[] r_bitwise[i];
    }
    delete[] r_bitwise;
}

void Rss_GenerateRandomShares(Lint **res, Lint **res_bitwise, uint ring_size, uint size, int *map, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = nodeNet->getNumParties();
    Lint temp0, temp1;

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    uint new_size = 2 * size;

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    Lint ***r_values = new Lint **[2];
    for (i = 0; i < 2; i++) {
        r_values[i] = new Lint *[numParties];
        for (j = 0; j < numParties; j++) {
            r_values[i][j] = new Lint[new_size];
            memset(r_values[i][j], 0, sizeof(Lint) * new_size);
        }
    }

    int gamma[2];
    switch (pid) {
    case 1:
        gamma[0] = 1;
        gamma[1] = 2;
        break;
    case 2:
        gamma[0] = 2;
        gamma[1] = 0;
        break;
    case 3:
        gamma[0] = 0;
        gamma[1] = 1;
        break;
    }

    Lint *r_bits = new Lint[size];
    memset(r_bits, 0, sizeof(Lint) * size);

    uint8_t *buffer = new uint8_t[bytes * new_size];
    // each party generating a unique random value
    nodeNet->prg_getrandom(bytes, size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(r_bits + i, buffer + i * bytes, bytes);
        // is this what we need to do to ensure we have a shorter value?
        // or do we need to do something at the end of the computation
        // r_bits[i] = r_bits[i] & nodeNet->SHIFT[ring_size];
    }

    nodeNet->prg_getrandom(1, bytes, new_size, buffer);

    // store arithmetic and bitwise representation sequentially
    // calculating p_i's own individual shares
    for (i = 0; i < size; i++) {
        memcpy(r_values[1][p_index] + 2 * i, buffer + (2 * i) * bytes, bytes);
        memcpy(r_values[1][p_index] + 2 * i + 1, buffer + (2 * i + 1) * bytes, bytes);
        // r_values[0][p_index][2 * i] = r_bits[i] ;
        r_values[0][p_index][2 * i] = r_bits[i] - r_values[1][p_index][2 * i];
        // r_values[0][p_index][2 * i + 1] = r_bits[i];
        r_values[0][p_index][2 * i + 1] = r_bits[i] ^ r_values[1][p_index][2 * i + 1];
    }

    // need to generate more random shares so that binary and arithmetic representations are different
    nodeNet->prg_getrandom(0, bytes, new_size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(r_values[0][gamma[1]] + (2 * i), buffer + (2 * i) * bytes, bytes);
        memcpy(r_values[0][gamma[1]] + (2 * i + 1), buffer + (2 * i + 1) * bytes, bytes);
    }

    //  sending r_values[0][p_index], receiving r_values[1][gamma[0]],
    nodeNet->SendAndGetDataFromPeer(map[0], map[1], r_values[0][p_index], r_values[1][gamma[0]], new_size, ring_size);

    for (i = 0; i < size; i++) {
        for (j = 0; j < numParties; j++) {
            // adding all the parties arithmetic shares together
            // memcpy(res[0] + (3 * i + j), r_values[0][j] + (2 * i), sizeof(Lint));
            // memcpy(res[1] + (3 * i + j), r_values[1][j] + (2 * i), sizeof(Lint));
            res[0][i] += r_values[0][j][2 * i];
            res[1][i] += r_values[1][j][2 * i];

            memcpy(res_bitwise[0] + (numParties * i + j), r_values[0][j] + (2 * i + 1), sizeof(Lint));
            memcpy(res_bitwise[1] + (numParties * i + j), r_values[1][j] + (2 * i + 1), sizeof(Lint));
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < numParties; j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_bits;
}


void Rss_edaBit(Lint **r, Lint **b_2, uint size, uint ring_size, uint bit_length, int *map, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint numParties = nodeNet->getNumParties();
    // printf("numParties : %u\n",numParties);

    uint i;
    // need to multiply by the number of parties in the computation
    uint new_size = numParties * size;

    Lint **r_bitwise = new Lint *[2];
    for (i = 0; i < 2; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(b_2[i], 0, sizeof(Lint) * size);
    }

    Rss_GenerateRandomShares(r, r_bitwise, ring_size, bit_length, size, map, nodeNet);

    Rss_nBitAdd(b_2, r_bitwise, ring_size, size, map, nodeNet);


    for (i = 0; i < 2; i++) {
        delete[] r_bitwise[i];
    }
    delete[] r_bitwise;
}

void Rss_GenerateRandomShares(Lint **res, Lint **res_bitwise, uint ring_size, uint bit_length, uint size, int *map, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = nodeNet->getNumParties();
    Lint temp0, temp1;

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    uint new_size = 2 * size;

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    Lint ***r_values = new Lint **[2];
    for (i = 0; i < 2; i++) {
        r_values[i] = new Lint *[numParties];
        for (j = 0; j < numParties; j++) {
            r_values[i][j] = new Lint[new_size];
            memset(r_values[i][j], 0, sizeof(Lint) * new_size);
        }
    }

    int gamma[2];
    switch (pid) {
    case 1:
        gamma[0] = 1;
        gamma[1] = 2;
        break;
    case 2:
        gamma[0] = 2;
        gamma[1] = 0;
        break;
    case 3:
        gamma[0] = 0;
        gamma[1] = 1;
        break;
    }

    Lint *r_bits = new Lint[size];
    memset(r_bits, 0, sizeof(Lint) * size);

    uint8_t *buffer = new uint8_t[bytes * new_size];
    // each party generating a unique random value
    nodeNet->prg_getrandom(bytes, size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(r_bits + i, buffer + i * bytes, bytes);
        // is this what we need to do to ensure we have a shorter value?
        // or do we need to do something at the end of the computation
        r_bits[i] = r_bits[i] & nodeNet->SHIFT[bit_length];
        // r_bits[i] = r_bits[i] & nodeNet->SHIFT[bit_length];
    }

    nodeNet->prg_getrandom(1, bytes, new_size, buffer);

    // store arithmetic and bitwise representation sequentially
    // calculating p_i's own individual shares
    for (i = 0; i < size; i++) {
        memcpy(r_values[1][p_index] + 2 * i, buffer + (2 * i) * bytes, bytes);
        memcpy(r_values[1][p_index] + 2 * i + 1, buffer + (2 * i + 1) * bytes, bytes);
        // r_values[0][p_index][2 * i] = r_bits[i] ;
        r_values[0][p_index][2 * i] = r_bits[i] - r_values[1][p_index][2 * i];
        // r_values[0][p_index][2 * i + 1] = r_bits[i];
        r_values[0][p_index][2 * i + 1] = r_bits[i] ^ r_values[1][p_index][2 * i + 1];
    }

    // need to generate more random shares so that binary and arithmetic representations are different
    nodeNet->prg_getrandom(0, bytes, new_size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(r_values[0][gamma[1]] + (2 * i), buffer + (2 * i) * bytes, bytes);
        memcpy(r_values[0][gamma[1]] + (2 * i + 1), buffer + (2 * i + 1) * bytes, bytes);
    }

    //  sending r_values[0][p_index], receiving r_values[1][gamma[0]],
    nodeNet->SendAndGetDataFromPeer(map[0], map[1], r_values[0][p_index], r_values[1][gamma[0]], new_size, ring_size);

    for (i = 0; i < size; i++) {
        for (j = 0; j < numParties; j++) {
            // adding all the parties arithmetic shares together
            // memcpy(res[0] + (3 * i + j), r_values[0][j] + (2 * i), sizeof(Lint));
            // memcpy(res[1] + (3 * i + j), r_values[1][j] + (2 * i), sizeof(Lint));
            res[0][i] += r_values[0][j][2 * i];
            res[1][i] += r_values[1][j][2 * i];

            memcpy(res_bitwise[0] + (numParties * i + j), r_values[0][j] + (2 * i + 1), sizeof(Lint));
            memcpy(res_bitwise[1] + (numParties * i + j), r_values[1][j] + (2 * i + 1), sizeof(Lint));
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < numParties; j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_bits;
}