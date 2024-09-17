#include "../Rss_Op.h"
extern "C" {
#include "../aes_ni.h"
}

// converts shares of a_k to shares of a_k'
// k --> ring_size
// k' --> ring_size_prime
void new_Rss_Convert(Lint **res, Lint **a, uint size, uint ring_size, uint ring_size_prime, int *map, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j, k, index; // used for loops

    uint n_rand_bits = size * ring_size;
    // first size*ring_size random bits used for first part of share conversion
    // 2nd half used for 2tok'
    Lint a1 = 0, a2 = 0;
    switch (pid) {
    case 1:
        a1 = 1;
        a2 = 0;
        break;
    case 2:
        a1 = 0;
        a2 = 0;
        break;
    case 3:
        a1 = 0;
        a2 = 1;
        break;
    }

    Lint **r_shares = new Lint *[2];
    for (i = 0; i < 2; i++) {
        r_shares[i] = new Lint[n_rand_bits];
    }
    // used for B2A component
    Rss_RandBit(r_shares, n_rand_bits, ring_size_prime, map, nodeNet);

    Lint **edaBit_r = new Lint *[2];
    Lint **edaBit_b_2 = new Lint *[2];
    Lint **r_2 = new Lint *[2];
    Lint **sum = new Lint *[2];
    Lint **a_2 = new Lint *[2];

    // will hold the k shares of a in (k_prime)
    Lint **a_k_prime = new Lint *[2];
    Lint *c = new Lint[size];
    
    Lint *res_check = new Lint[size];

    for (i = 0; i < 2; i++) {
        r_2[i] = new Lint[size];
        a_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        a_k_prime[i] = new Lint[n_rand_bits];

        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
    }

    // only need ring_size bit-length values for both parts of computation
    // first protection and b2a
    Rss_edaBit(edaBit_r, edaBit_b_2, size, ring_size, map, nodeNet);

    for (i = 0; i < size; i++) {
        sum[0][i] = (a[0][i] - edaBit_r[0][i]);
        sum[1][i] = (a[1][i] - edaBit_r[1][i]);
    }

    // Rss_Open(c, sum, size, map, ring_size_prime, nodeNet);
    Rss_Open(c, sum, size, map, ring_size, nodeNet);

    Rss_BitAdd(a_2, c, edaBit_b_2, ring_size, size, map, nodeNet);
    // Rss_BitAdd(res, c, edaBit_b_2, ring_size, size, map, nodeNet);

    // 2tok' component (B2A)
    // a_2 is k-bits long

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;

            r_2[0][j] = Lint(SET_BIT(r_2[0][j], Lint(k), GET_BIT(r_shares[0][index], Lint(0))));
            r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][index], Lint(0))));
        }
    }

    // 2tok' component
    // a_2 is k-bits long

    for (i = 0; i < size; i++) {
        sum[0][i] = (a_2[0][i] ^ r_2[0][i]);
        sum[1][i] = (a_2[1][i] ^ r_2[1][i]);
    }

    // should this be bitwise-open? (like Open_Byte)
    // and what should the ring_size be?
    // memset(c, 0, sizeof(Lint) * size); // clearing c
    Rss_Open_Bitwise(c, sum, size, map, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (j = 0; j < ring_size; j++) {
            index = i * ring_size + j;

            a_k_prime[0][index] = a1 * GET_BIT(c[i], Lint(j)) + r_shares[0][index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[0][index];
            a_k_prime[1][index] = a2 * GET_BIT(c[i], Lint(j)) + r_shares[1][index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[1][index];
        }
    }

    // constructing the final shares of a
    for (j = 0; j < size; j++) {
        res[0][j] = 0, res[1][j] = 0;

        for (k = 0; k < ring_size; k++) {
            // this is for step 3
            index = j * ring_size + k;
            res[0][j] += (a_k_prime[0][index] << Lint(k));
            res[1][j] += (a_k_prime[1][index] << Lint(k));
        }
    }

    delete[] c;
    delete[] res_check;

    for (i = 0; i < 2; i++) {
        delete[] r_shares[i];
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
        delete[] r_2[i];
        delete[] a_2[i];
        delete[] a_k_prime[i];
    }
    delete[] r_shares;
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
    delete[] r_2;
    delete[] a_2;
    delete[] a_k_prime;
}

void Rss_Convert(Lint **res, Lint **a, uint size, uint ring_size, uint ring_size_prime, int *map, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j, k, index; // used for loops

    uint n_rand_bits = 2 * size * (ring_size);
    // first size*ring_size random bits used for first part of share conversion
    // 2nd half used for 2tok'
    Lint a1 = 0, a2 = 0;
    switch (pid) {
    case 1:
        a1 = 1;
        a2 = 0;
        break;
    case 2:
        a1 = 0;
        a2 = 0;
        break;
    case 3:
        a1 = 0;
        a2 = 1;
        break;
    }

    Lint **r_shares = new Lint *[2];
    for (i = 0; i < 2; i++) {
        r_shares[i] = new Lint[n_rand_bits];
    }
    // generating shares in k' since we will be using them at a later point in the computation
    Rss_RandBit(r_shares, n_rand_bits, ring_size_prime, map, nodeNet);
    // Rss_RandBit(r_shares, n_rand_bits, ring_size, map, nodeNet);
    Lint **r = new Lint *[2];
    Lint **r_2 = new Lint *[2];
    Lint **sum = new Lint *[2];
    Lint **a_2 = new Lint *[2];

    // will hold the k shares of a in (k_prime)
    Lint **a_k_prime = new Lint *[2];

    Lint *c = new Lint[size];
    memset(c, 0, sizeof(Lint) * size);

    Lint *res_check = new Lint[size];
    // memset(res_check, 0, sizeof(Lint) * size);

    for (i = 0; i < 2; i++) {
        r[i] = new Lint[size];
        memset(r[i], 0, sizeof(Lint) * size);
        r_2[i] = new Lint[size];
        a_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        a_k_prime[i] = new Lint[(n_rand_bits >> 1)];
    }

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;
            r[0][j] = r[0][j] + (r_shares[0][index] << Lint(k));
            r[1][j] = r[1][j] + (r_shares[1][index] << Lint(k));
        }
    }
    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;

            r_2[0][j] = Lint(SET_BIT(r_2[0][j], Lint(k), GET_BIT(r_shares[0][index], Lint(0))));
            r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][index], Lint(0))));
        }
    }

    for (i = 0; i < size; i++) {
        sum[0][i] = (a[0][i] - r[0][i]);
        sum[1][i] = (a[1][i] - r[1][i]);
    }

    // Rss_Open(c, sum, size, map, ring_size_prime, nodeNet);
    Rss_Open(c, sum, size, map, ring_size, nodeNet);

    Rss_BitAdd(a_2, c, r_2, ring_size, size, map, nodeNet);

    // Rss_Open_Bitwise(res_check, a_2, size, map, ring_size, nodeNet);
    // for (i = 0; i < size; i++) {
    //  // printf("res_check[%i] : %llu\n", i, res_check[i] & nodeNet->SHIFT[ring_size]);
    // }

    // FIX the r_2
    // resetting r_2 with fresh random bits
    for (i = 0; i < 2; i++) {
        memset(r_2[i], 0, sizeof(Lint) * size);
    }

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;

            r_2[0][j] = Lint(SET_BIT(r_2[0][j], Lint(k), GET_BIT(r_shares[0][size * ring_size + index], Lint(0))));
            r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][size * ring_size + index], Lint(0))));
        }
    }

    // 2tok' component
    // a_2 is k-bits long

    for (i = 0; i < size; i++) {
        sum[0][i] = (a_2[0][i] ^ r_2[0][i]);
        sum[1][i] = (a_2[1][i] ^ r_2[1][i]);
    }

    // should this be bitwise-open? (like Open_Byte)
    // and what should the ring_size be?
    memset(c, 0, sizeof(Lint) * size); // clearing c
    Rss_Open_Bitwise(c, sum, size, map, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (j = 0; j < ring_size; j++) {
            index = i * ring_size + j;

            a_k_prime[0][index] = a1 * GET_BIT(c[i], Lint(j)) + r_shares[0][size * ring_size + index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[0][size * ring_size + index];
            a_k_prime[1][index] = a2 * GET_BIT(c[i], Lint(j)) + r_shares[1][size * ring_size + index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[1][size * ring_size + index];
        }
    }

    // constructing the final shares of a
    for (j = 0; j < size; j++) {

        res[0][j] = 0, res[1][j] = 0;
        // using temps so we don't have to worry about res being cleared1

        for (k = 0; k < ring_size; k++) {
            // this is for step 3
            index = j * ring_size + k;

            res[0][j] += (a_k_prime[0][index] << Lint(k));
            res[1][j] += (a_k_prime[1][index] << Lint(k));

        }

    }

    delete[] c;
    delete[] res_check;

    for (i = 0; i < 2; i++) {
        delete[] r_shares[i];
        delete[] r[i];
        delete[] sum[i];
        delete[] r_2[i];
        delete[] a_2[i];
        delete[] a_k_prime[i];
    }
    delete[] r_shares;
    delete[] r;
    delete[] sum;
    delete[] r_2;
    delete[] a_2;
    delete[] a_k_prime;
}