#include "../Rss_Op.h"
extern "C" {
#include "../aes_ni.h"
}

void new_Rss_MSB(Lint **res, Lint **a, uint size, uint ring_size, int *map, NodeNetwork *nodeNet) {
    int pid = nodeNet->getID();
    uint i, j, k, index; // used for loops

    // only need to generate a single random bit per private value
    Lint **b = new Lint *[2];
    Lint **sum = new Lint *[2];
    Lint **u_2 = new Lint *[2];
    Lint **edaBit_r = new Lint *[2];
    Lint **edaBit_b_2 = new Lint *[2];

    Lint *c = new Lint[size];
    memset(c, 0, sizeof(Lint) * size);
    Lint *e = new Lint[size];
    memset(e, 0, sizeof(Lint) * size);

    Lint *res_check = new Lint[size];
    Lint *r_2_open = new Lint[size];
    Lint *u_2_open = new Lint[size];

    for (i = 0; i < 2; i++) {
        b[i] = new Lint[size];
        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        u_2[i] = new Lint[size];
        memset(u_2[i], 0, sizeof(Lint) * size);
    }

    Lint a1 = 0;
    Lint a2 = 0;
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

    Rss_edaBit(edaBit_r, edaBit_b_2, size, ring_size, ring_size - 2, map, nodeNet);
    Rss_RandBit(b, size, ring_size, map, nodeNet);

    // sum = 2(a _ )
    for (i = 0; i < size; i++) {
        // sum[0][i] = (a[0][i] + edaBit_r[0][i]);
        // sum[1][i] = (a[1][i] + edaBit_r[1][i]);
        sum[0][i] = (a[0][i] + edaBit_r[0][i]) << Lint(1);
        sum[1][i] = (a[1][i] + edaBit_r[1][i]) << Lint(1);
    }

    Rss_Open(c, sum, size, map, ring_size, nodeNet);

    // c = c/2
    for (i = 0; i < size; i++) {
        c[i] = c[i] >> Lint(1);
    }

    // Rss_Open_Bitwise(r_2_open, edaBit_b_2, size, map, ring_size, nodeNet);
    // // this part is still correct
    // however, the edaBit_b_2 shares do get modified
    // which may not be desierable
    Rss_BitLT(u_2, c, edaBit_b_2, ring_size, size, map, nodeNet);

    // Rss_Open_Bitwise(u_2_open, u_2, size, map, ring_size, nodeNet);

    // for (int i = 0; i < size; i++) {

    //     res_check[i] = (c[i] < r_2_open[i]);
    //     if (!(u_2_open[i] == res_check[i])) {
    //         printf("[%i] c < r_2 : %u   --- expected: %u\n", i, u_2_open[i], res_check[i]);
    //         printf("c = %u --- edaBit_b_2 = %u\n", c[i], r_2_open[i]);
    //         printf("BitLT ERROR at %d \n", i);
    //     }
    // }

    // need to figure out if we need a second edabit here
    // or if we can reuse the first one from the beginning
    for (i = 0; i < size; ++i) {
        sum[0][i] = a[0][i] - c[i] * a1 + edaBit_r[0][i] - (u_2[0][i] << Lint(ring_size - 1)) + (b[0][i] << Lint(ring_size - 1));
        sum[1][i] = a[1][i] - c[i] * a2 + edaBit_r[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    }
    // opening sum
    Rss_Open(e, sum, size, map, ring_size, nodeNet);

    Lint e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        res[0][i] = e_bit * a1 + b[0][i] - (e_bit * b[0][i] << Lint(1));
        res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));
    }

    // cleanup
    delete[] c;
    delete[] e;
    delete[] res_check;
    delete[] r_2_open;
    delete[] u_2_open;

    for (i = 0; i < 2; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] b[i];
        delete[] sum[i];
        delete[] u_2[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] b;
    delete[] sum;
    delete[] u_2;
}

void Rss_MSB(Lint **res, Lint **a, uint size, uint ring_size, int *map, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j, k, index; // used for loops
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint n_rand_bits = size * (ring_size + 1);

    Lint **r_shares = new Lint *[2];
    for (i = 0; i < 2; i++) {
        r_shares[i] = new Lint[n_rand_bits];
    }

    Lint **b = new Lint *[2];
    Lint **r = new Lint *[2];
    Lint **sum = new Lint *[2];
    Lint **rprime = new Lint *[2];
    Lint **r_2 = new Lint *[2];
    Lint **u_2 = new Lint *[2];

    Lint *c = new Lint[size];
    memset(c, 0, sizeof(Lint) * size);
    Lint *e = new Lint[size];
    memset(e, 0, sizeof(Lint) * size);

    // Lint *res_check = new Lint[size];
    // Lint *r_2_open = new Lint[size];
    // Lint *u_2_open = new Lint[size];

    for (i = 0; i < 2; i++) {
        b[i] = new Lint[size];
        r[i] = new Lint[size];
        sum[i] = new Lint[size];
        rprime[i] = new Lint[size];
        memset(rprime[i], 0, sizeof(Lint) * size);
        r_2[i] = new Lint[size];
        memset(r_2[i], 0, sizeof(Lint) * size);
        u_2[i] = new Lint[size];
        memset(u_2[i], 0, sizeof(Lint) * size);
    }

    Lint a1 = 0;
    Lint a2 = 0;
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

    // offline component start
    Rss_RandBit(r_shares, n_rand_bits, ring_size, map, nodeNet);
    for (i = 0; i < size; i++) {
        b[0][i] = r_shares[0][size * ring_size + i];
        b[1][i] = r_shares[1][size * ring_size + i];
    }

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size - 1; k++) {
            // this is for step 3
            index = j * ring_size + k;
            rprime[0][j] = rprime[0][j] + (r_shares[0][index] << Lint(k));
            rprime[1][j] = rprime[1][j] + (r_shares[1][index] << Lint(k));
        }
        index = j * ring_size + k;
        r[0][j] = (rprime[0][j] + ((r_shares[0][index]) << Lint(k)));
        r[1][j] = (rprime[1][j] + ((r_shares[1][index]) << Lint(k)));
    }
    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size - 1; k++) {
            index = j * ring_size + k;

            r_2[0][j] = Lint(SET_BIT(r_2[0][j], Lint(k), GET_BIT(r_shares[0][index], Lint(0))));
            r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][index], Lint(0))));
        }
    }
    // offline component ends
    // step 2
    for (i = 0; i < size; i++) {
        sum[0][i] = (a[0][i] + r[0][i]); // & nodeNet->SHIFT[1] ;
        sum[1][i] = (a[1][i] + r[1][i]); // & nodeNet->SHIFT[1] ;
    }

    Rss_Open(c, sum, size, map, ring_size, nodeNet);

    // step 3 -- getting the k-1 lowest bits of c[i]
    for (i = 0; i < size; i++) {
        c[i] = c[i] & nodeNet->SHIFT[ring_size - 1];
    }

    // Rss_Open_Bitwise(r_2_open, r_2, size, map, ring_size, nodeNet);
    // calculating c <? r_2, where r_2 is bitwise shared
    // gettimeofday(&start,NULL); //start timer here
    Rss_BitLT(u_2, c, r_2, ring_size, size, map, nodeNet);
    // gettimeofday(&end,NULL);//stop timer here
    // timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    // printf("runtime for BitLT with data size %d = %ld us\n", size, timer);

    // Rss_Open_Bitwise(u_2_open, u_2, size, map, ring_size, nodeNet);

    // for (int i = 0; i < size; i++) {

    //     res_check[i] = (c[i] < r_2_open[i]);
    //     if (!(u_2_open[i] == res_check[i])) {
    //         printf("[%i] c < r_2 : %u   --- expected: %u\n", i, u_2_open[i], res_check[i]);
    //         printf("c = %u --- r_2_open = %u\n", c[i], r_2_open[i]);
    //         printf("BitLT ERROR at %d \n", i);
    //     }
    // }

    for (i = 0; i < size; ++i) {
        sum[0][i] = a[0][i] - c[i] * a1 + rprime[0][i] - (u_2[0][i] << Lint(ring_size - 1)) + (b[0][i] << Lint(ring_size - 1));
        sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    }
    // opening sum
    Rss_Open(e, sum, size, map, ring_size, nodeNet);

    Lint e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        res[0][i] = e_bit * a1 + b[0][i] - (e_bit * b[0][i] << Lint(1));
        res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));
    }

    // cleanup
    delete[] c;
    delete[] e;
    // delete[] res_check;
    // delete[] r_2_open;
    // delete[] u_2_open;

    for (i = 0; i < 2; i++) {
        delete[] r_shares[i];
        delete[] b[i];
        delete[] r[i];
        delete[] sum[i];
        delete[] rprime[i];
        delete[] r_2[i];
        delete[] u_2[i];
    }
    delete[] r_shares;
    delete[] b;
    delete[] r;
    delete[] sum;
    delete[] rprime;
    delete[] r_2;
    delete[] u_2;
}
