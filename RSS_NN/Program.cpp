#include "Program.h"

bool checkbytes(Lint a, Lint b, int bytes) {

    Lint temp_a = 0;
    Lint temp_b = 0;
    memcpy(&temp_a, &a, bytes);
    memcpy(&temp_b, &b, bytes);
    if (temp_a == temp_b) {
        return true;
    }
    return false;
}

bool checkbytes_byte(uint8_t a, uint8_t b, int bytes) {

    uint8_t temp_a = 0;
    uint8_t temp_b = 0;
    memcpy(&temp_a, &a, bytes);
    memcpy(&temp_b, &b, bytes);
    if (temp_a == temp_b) {
        return true;
    }
    return false;
}

void test(NodeNetwork *nodeNet, NodeConfiguration *nodeConfig, uint size, uint batch_size) {
    int bits;
    int i, j, k;
    int pid = nodeConfig->getID();
    int flag = 0;

    int total;
    int ring_size = nodeNet->RING;

    printf("ring_size = %i\n", ring_size);
    printf("8*sizeof(Lint) = %i\n", 8 * sizeof(Lint));
    printf("batch_size = %i\n", batch_size); 

    // checking if we can access this built-in function
    // uint aaa = _pext_u32((unsigned) ring_size, 0x55555555);


    int bytes = (ring_size + 2 + 8 - 1) / 8;

    struct timeval start;
    struct timeval end;
    unsigned long timer;

    printf("hello, I am %d\n", pid);

    int map[2];
    switch (pid) {
    case 1:
        map[0] = 3;
        map[1] = 2;
        break;
    case 2:
        map[0] = 1;
        map[1] = 3;
        break;
    case 3:
        map[0] = 2;
        map[1] = 1;
        break;
    }

    //setup prg key (will be used by all parties, only for data makeup)
    __m128i *key_prg;
    uint8_t key_raw[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    key_prg = offline_prg_keyschedule(key_raw);
    //setup prg seed(k1, k2, k3)
    uint8_t k1[] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    uint8_t k2[] = {0xa2, 0x34, 0x6f, 0x67, 0x10, 0x1b, 0x13, 0xa3, 0x56, 0x45, 0x90, 0xb2, 0x13, 0xe3, 0x23, 0x24};

    //make up data
    Lint **a = new Lint *[2];
    Lint **b = new Lint *[2];
    Lint **c = new Lint *[2];
    Lint **d = new Lint *[2];

    for (int i = 0; i < 2; i++) {
        c[i] = new Lint[size];
        memset(c[i], 0, sizeof(Lint) * size);
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }

    Lint **Data1;
    Lint **Data2;

    Data1 = new Lint *[3];
    for (int i = 0; i < 3; i++) {
        Data1[i] = new Lint[size];
        memset(Data1[i], 0, sizeof(Lint) * size);
    }

    Data2 = new Lint *[3];
    for (int i = 0; i < 3; i++) {
        Data2[i] = new Lint[size];
        memset(Data2[i], 0, sizeof(Lint) * size);
    }

    for (int i = 0; i < size; i++) {
        prg_aes_ni(Data1[0] + i, k1, key_prg);
        prg_aes_ni(Data1[1] + i, k1, key_prg);
        prg_aes_ni(Data2[0] + i, k2, key_prg);
        prg_aes_ni(Data2[1] + i, k2, key_prg);
        Data1[2][i] = i - Data1[0][i] - Data1[1][i];
        Data2[2][i] = i - Data2[0][i] - Data2[1][i];
        // printf("Data[2][%i] : %i\n", i, Data2[2][i]);
    }
    // printf("Data1[0][%i] : %llu\n", 0, Data1[0][0] );
    // printf("Data1[1][%i] : %llu\n", 0, Data1[1][0] );
    free(key_prg);

    Lint *res = new Lint[size];
    memset(res, 0, sizeof(Lint) * size);
    Lint *res2 = new Lint[size];
    memset(res2, 0, sizeof(Lint) * size);
    Lint *res3 = new Lint[size];
    memset(res3, 0, sizeof(Lint) * size);

    Lint *res_check = new Lint[size];
    memset(res_check, 0, sizeof(Lint) * size);
    //assign data
    switch (pid) {
    case 1:
        a[0] = Data1[1];
        a[1] = Data1[2];
        b[0] = Data2[1];
        b[1] = Data2[2];
        break;
    case 2:
        a[0] = Data1[2];
        a[1] = Data1[0];
        b[0] = Data2[2];
        b[1] = Data2[0];
        break;
    case 3:
        a[0] = Data1[0];
        a[1] = Data1[1];
        b[0] = Data2[0];
        b[1] = Data2[1];
        break;
    }


    Lint* target = new Lint [2];
    switch(pid){
    case 1:
      target[0] = 2;
      target[1] = 3;
      break;
    case 2:
      target[0] = 3;
      target[1] = 1;
      break;
    case 3:
      target[0] = 1;
      target[1] = 2;
      break;
    }
    Lint ** toBeOpen;
    toBeOpen = new Lint *[2];

    toBeOpen[0] = new Lint [1];
    memset(toBeOpen[0],0,sizeof(Lint));
    toBeOpen[1] = new Lint [1];
    memset(toBeOpen[1],0,sizeof(Lint));

    toBeOpen[0][0] = target[0];
    toBeOpen[1][0] = target[1];

    Rss_Open(res, toBeOpen, 1, map, ring_size, nodeNet);
    print_1283(res[0]);

    // computation test open:
    gettimeofday(&start, NULL); //start timer here
    Rss_Open(res, a, size, map, ring_size, nodeNet);
    gettimeofday(&end, NULL); //stop timer here

    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("runtime for open with data size %d = %ld us\n", size, timer);

    //check open
    for (int i = 0; i < size; i++) {

        res_check[i] = Data1[0][i] + Data1[1][i] + Data1[2][i];
        res_check[i] = res_check[i] & nodeNet->SHIFT[ring_size];

        if (!checkbytes(res[i], res_check[i], bytes) && flag == 0) {
            printf("Open ERROR at %d !=   \n", i);
            print_1283(res[i]);
            print_1283(res_check[i]);
            flag = 1;
        }
    }
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "Open Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "Open error!\n" ANSI_COLOR_RESET );
    }
    /*
    total = 100;
    
    gettimeofday(&start, NULL); //start timer here
    //computation test Mult:
    for (j = 0; j < total; j++) {
        // gettimeofday(&start,NULL); //start timer here
        Rss_Mult(c, a, b, size, ring_size, map, nodeNet);
        // gettimeofday(&end,NULL);//stop timer here
    }
    gettimeofday(&end, NULL); //stop timer here
    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("runtime for mult with data size %d = %.6lf us\n", size, (double)(timer)/total); //double check time calculation is correct
    printf("runtime for mult with data size %d = %.6lf ms\n", size, (double)(timer * 0.001) / total); //double check time calculation is correct
    // printf("Ratio for %d = %.6lf us\n", size,  (double)(timer/size)/total); //double check time calculation is correct
    printf("Ratio for %d = %.6lf ms\n", size, (double)(timer * 0.001 / size) / total); //double check time calculation is correct
    // it should be though

    gettimeofday(&start, NULL); //start timer here
    Rss_Mult(c, a, b, size, ring_size, map, nodeNet);
    gettimeofday(&end, NULL); //stop timer here

    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("runtime for mult with data size %d = %ld us\n", size, timer);

    Rss_Open(res, c, size, map, ring_size, nodeNet);

    //check Mult
    flag = 0;
    for (Lint i = 0; i < size; i++) {

        res_check[i] = (i * i) & nodeNet->SHIFT[ring_size];
	if (!checkbytes(res[i], res_check[i], bytes) && flag == 0) {
	    printf("Mult ERROR at %d !=   \n", i);

            print_1283(res[i]);
            print_1283(res_check[i]);
            flag = 1;
	}
    }
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "Mult Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "Mult error!\n" ANSI_COLOR_RESET );
    }
    
    //computation test MultPub:
    printf("Testing MultPub...\n");

    gettimeofday(&start, NULL);                            //start timer here
    Rss_MultPub(res, a, a, size, map, ring_size, nodeNet); // Mulpub squaring
    gettimeofday(&end, NULL);                              //stop timer here

    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("runtime for multpub with data size %d = %ld us\n", size, timer);
    //check MultPub
    flag = 0;
    for (Lint i = 0; i < size; i++) {
        res_check[i] = (i * i) & nodeNet->SHIFT[ring_size];
        if (!checkbytes(res[i], res_check[i], bytes) && flag == 0) {
            printf("MultPub ERROR at %d !=   ", i);
            print_1283(res[i]);
            print_1283(res_check[i]);
            flag = 1;
        }
    }
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "MultPub Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "MultPub error!\n" ANSI_COLOR_RESET );
    }
    */
    
    printf("Testing RandBit...\n");
    total = 100;

    gettimeofday(&start, NULL); //start timer here
    //computation test Mult:
    for (j = 0; j < total; j++) {
        Rss_RandBit(c, size, ring_size, map, nodeNet);
    }
    gettimeofday(&end, NULL); //stop timer here
    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("Runtime for randbit with data size %d = %.6lf ms\n", size, (double)(timer * 0.001) / total);
    printf("Ratio for randbit %d = %.6lf ms\n", size, (double)(timer * 0.001 / size) / total);

    gettimeofday(&start, NULL); //start timer here
    Rss_RandBit(c, size, ring_size, map, nodeNet);
    gettimeofday(&end, NULL); //stop timer here

    memset(res, 0, sizeof(Lint) * size);

    // timer = (1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec);
    // printf("runtime for RandBit with data size %d = %f ms\n", size, (float)(timer/total)/1000);

    // Rss_Open(res, c, size, map, ring_size - 10, nodeNet);
    Rss_Open(res, c, size, map, ring_size, nodeNet);
    for (int i = 0; i < size; i++) {
        // printf("res[%i]  : %llu\n", i, res[i]);
        //if ((res[i] != 0) && (res[i] != 1)) {
      //printf("res[%i]  : %llu\n", i, res[i]);
            //printf("RandBit Error at %u\n", i);
            //flag = 1;
	    //}
    }
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "RandBit Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "RandBit error!\n" ANSI_COLOR_RESET );
    }
    /*
    printf("Testing edaBit...\n");
    // gettimeofday(&start,NULL); //start timer here
    // Rss_edaBit(c, d, size/ring_size, ring_size, map, nodeNet);
    // gettimeofday(&end, NULL); //stop timer here
    total = 100;

    gettimeofday(&start, NULL); //start timer here
    for (j = 0; j < total; j++) {
        Rss_edaBit(c, d, size, ring_size, map, nodeNet);
        // Rss_RandBit(c, size, ring_size, map, nodeNet);
    }

    gettimeofday(&end, NULL); //stop timer here

    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    printf("Runtime for edaBit with data size  %d = %.6lf ms\n", (size), (double)(timer * 0.001) / total);
    printf("Ratio for edaBit %d = %.6lf ms\n", size, (double)(timer * 0.001 / (size)) / total);

    Rss_Open(res, c, size, map, ring_size, nodeNet);
    Rss_Open_Bitwise(res2, d, size, map, ring_size, nodeNet);

    flag = 0;

    for (int i = 0; i < size; i++) {
        // printf("res[%i]  : %llu\n", i, res[i]);
        // print_binary(res[i], ring_size);
        // printf("res2[%i]  : %llu\n", i, res2[i]);
        // print_binary(res2[i], ring_size);
        // res_check[i] = GET_BIT_TEST(res_check[i], ring_size - 1);
        if (!(res[i] == res2[i])) {
            // printf("res[%i]  : %llu\n", i, res[i]);
            // print_binary(res[i], ring_size);
            // printf("res2[%i]  : %llu\n", i, res2[i]);
            // print_binary(res2[i], ring_size);
            // printf("edaBit ERROR at %d \n", i);
            flag = 1;
        }
    }
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "edaBit Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "edaBit error!\n" ANSI_COLOR_RESET );
    }

    total = 100;
    gettimeofday(&start, NULL); //start timer here
    for (j = 0; j < total; j++) {
        Rss_MSB(c, a, size, ring_size, map, nodeNet);
    }
    gettimeofday(&end, NULL); //stop timer here
    timer = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec);

    printf("Runtime for MSB with data size  %d = %.6lf ms\n", (size), (double)(timer * 0.001) / total);
    printf("Ratio for MSB %d = %.6lf ms\n", size, (double)(timer * 0.001 / (size)) / total);
    Rss_Open(res, c, size, map, ring_size, nodeNet);

    Rss_Open(res_check, a, size, map, ring_size, nodeNet);
    flag = 0;
    for (int i = 0; i < size; i++) {

        res_check[i] = GET_BIT_TEST(res_check[i], ring_size - 1);
        if (!(res[i] == res_check[i])) {
            // printf("MSB ERROR at %d \n", i);
            // printf("res[%i]  : %llu\n", i, res[i]);
            // print_binary(res[i], ring_size);
            // printf("res_check[%i]  : %llu\n", i, res_check[i]);
            // print_binary(res_check[i], ring_size);
            flag = 1;
        }
    }
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "MSB Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "MSB error!\n" ANSI_COLOR_RESET );
    }

    total = 100;

    gettimeofday(&start, NULL); //start timer here
    for (j = 0; j < total; j++) {
        new_Rss_MSB(c, a, size, ring_size, map, nodeNet);
    }
    gettimeofday(&end, NULL); //stop timer here
    timer = (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec);

    printf("Runtime for newMSB with data size  %d = %.6lf ms\n", (size), (double)(timer * 0.001) / total);
    printf("Ratio for newMSB %d = %.6lf ms\n", size, (double)(timer * 0.001 / (size)) / total);
    Rss_Open(res, c, size, map, ring_size, nodeNet);

    Rss_Open(res_check, a, size, map, ring_size, nodeNet);
    flag = 0;
    for (int i = 0; i < size; i++) {

        res_check[i] = GET_BIT_TEST(res_check[i], ring_size - 1);
        if (!(res[i] == res_check[i])) {
            // printf("newMSB ERROR at %d \n", i);
            // printf("res[%i]  : %llu\n", i, res[i]);
            // printf("res_check[%i]  : %llu\n", i, res_check[i]);
            flag = 1;
        }
    }
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "newMSB Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "newMSB error!\n" ANSI_COLOR_RESET );
    }


    printf("Testing LT.....\n");
    gettimeofday(&start, NULL); //start timer here
    Rss_LT(c, a, b, size, ring_size, map, nodeNet);
    gettimeofday(&end, NULL); //stop timer here

    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("runtime for LT with data size %d = %ld us\n", size, timer);
    Rss_Open(res, a, size, map, ring_size, nodeNet);
    Rss_Open(res2, b, size, map, ring_size, nodeNet);

    Rss_Open(res3, c, size, map, ring_size, nodeNet);

    flag = 0;

    for (int i = 0; i < size; i++) {
        // Getting MSB of a - b
        Lint c_check = GET_BIT(res[i] - res2[i], ring_size);
        res_check[i] = c_check * (res2[i] - res[i]) + res[i];
        if (!(res3[i] == res_check[i])) {
            // printf("LT ERROR at %d \n", i);
            flag = 1;
        }
    }
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "LT Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "LT error!\n" ANSI_COLOR_RESET );
    }

    printf("Testing newLT.....\n");
    gettimeofday(&start, NULL); //start timer here
    new_Rss_LT(c, a, b, size, ring_size, map, nodeNet);
    gettimeofday(&end, NULL); //stop timer here

    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("runtime for newLT with data size %d = %ld us\n", size, timer);
    Rss_Open(res, a, size, map, ring_size, nodeNet);
    Rss_Open(res2, b, size, map, ring_size, nodeNet);

    Rss_Open(res3, c, size, map, ring_size, nodeNet);

    flag = 0;

    for (int i = 0; i < size; i++) {
        // Getting MSB of a - b
        Lint c_check = GET_BIT(res[i] - res2[i], ring_size);
        res_check[i] = c_check * (res2[i] - res[i]) + res[i];
        if (!(res3[i] == res_check[i])) {
            // printf("newLT ERROR at %d \n", i);
            flag = 1;
        }
    }
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "newLT Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "newLT error!\n" ANSI_COLOR_RESET );
    }

    memset(res, 0, sizeof(Lint) * size);

    printf("Testing Conversion.....\n");
    // NOTE ///////////////////
    // Must used unsiged long if we want to test more than +1 (assuming ring size of 30)
    gettimeofday(&start, NULL); //start timer here
    Rss_Convert(c, a, size, ring_size, ring_size + 1, map, nodeNet);
    gettimeofday(&end, NULL); //stop timer here

    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("runtime for Conversion with data size %d = %ld us\n", size, timer);

    Rss_Open(res, c, size, map, ring_size + 1, nodeNet);
    for (Lint i = 0; i < size; i++) {

        res_check[i] = Data1[0][i] + Data1[1][i] + Data1[2][i];
        res_check[i] = res_check[i] & nodeNet->SHIFT[ring_size];
        // printf("res[%i]  : %llu\n", i, res[i]);
        // printf("res_check[%i]  : %llu\n", i, res_check[i]);
        if (!(res[i] == res_check[i])) {
            // if(!checkbytes(res[i], res_check[i], bytes) && flag == 0) {
            // printf("Conversion ERROR at %d\n", i);
            // printf("res[%i]  : %llu\n", i, res[i]);
            // printf("res_check[%i]  : %llu\n\n", i, res_check[i]);
            flag = 1;
        }
    }
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "Conversion Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "Conversion error! " ANSI_COLOR_RESET );
        printf(ANSI_COLOR_RED "Did you use an appropriate ring size?\n" ANSI_COLOR_RESET );
    }


    printf("Testing newConversion.....\n");
    // NOTE ///////////////////
    // Must used unsiged long if we want to test more than +1 (assuming ring size of 30)
    gettimeofday(&start, NULL); //start timer here
    new_Rss_Convert(c, a, size, ring_size, ring_size , map, nodeNet);
    gettimeofday(&end, NULL); //stop timer here

    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("runtime for newConversion with data size %d = %ld us\n", size, timer);

    Rss_Open(res, c, size, map, ring_size, nodeNet);
    flag = 0;
    for (Lint i = 0; i < size; i++) {

        res_check[i] = Data1[0][i] + Data1[1][i] + Data1[2][i];
        res_check[i] = res_check[i] & nodeNet->SHIFT[ring_size];
        if (!(res[i] == res_check[i])) {
            // printf("newConversion ERROR at %d\n", i);
            // printf("res[%i]  : %llu\n", i, res[i]);
            // printf("res_check[%i]  : %llu\n\n", i, res_check[i]);
            flag = 1;
        }
    }
    // odd iterations are working correctly????
    if (flag == 0) {
        printf(ANSI_COLOR_GREEN "newConversion Correct!\n" ANSI_COLOR_RESET );
    } else {
        printf(ANSI_COLOR_RED "newConversion error! " ANSI_COLOR_RESET );
        printf(ANSI_COLOR_RED "Did you use an appropriate ring size?\n" ANSI_COLOR_RESET );
    }
    */
    // printf("Testing MaxPool.....\n");
    // gettimeofday(&start,NULL); //start timer here
    // MaxPool(c, a, 16, 24, 24, ring_size, map, nodeNet);
    // // MaxPool(c, a, size, ring_size, ring_size + 10, map, nodeNet);
    // gettimeofday(&end,NULL);//stop timer here

    // timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    // printf("runtime for Conversion with data size %d = %ld us\n", size, timer);
    // Rss_Open(res, c, 16*12*12, map, ring_size, nodeNet);
    // for(int i =0; i< 16*12*12; i++) {
    //  printf("res[%i]  : %llu\n", i, res[i]);
    // }

    // total = 0;

    // printf("Testing SVM.....\n");
    // gettimeofday(&start,NULL); //start timer here
    // for (j = 0; j < total; j++) {

    //  Rss_MatMultArray_batch(c, a, b, 463, 128, 1, ring_size, batch_size, 0, map, nodeNet);

    //  // for (i = 0; i < 463; i++) {
    //  //  Rss_MatMultArray_batch(c, a, b, 1, 128, 1, ring_size, batch_size, 0, map, nodeNet);

    //  // }
    //  arg_max(c, b, a, ring_size, 463, batch_size, map, nodeNet);

    // }
    // gettimeofday(&end,NULL);//stop timer here
    // timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    // // printf("runtime for SVM with data size %d = %ld us\n", size, (timer/batch_size)/total); //double check time calculation is correct
    // // it should be though

    // // printf("runtime for mult with data size %d = %.6lf us\n", size, (double)(timer)/total); //double check time calculation is correct
    // printf("\nruntime for SVM with data size %d = %.6lf ms\n", total*batch_size, (double)(timer*0.001)); //double check time calculation is correct
    // printf("Average prediction time for data size %d = %.6lf ms\n", total*batch_size, (double)(timer*0.001)/(total*batch_size)); //double check time calculation is correct
    // // printf("Ratio for %d = %.6lf us\n", size,  (double)(timer/size)/total); //double check time calculation is correct
    // // printf("Ratio for SVM %d = %.6lf ms\n\n", size,  (double)(timer*0.001/size)/total); //double check time calculation is correct
    // // // it should be though

    // printf("Testing Max.....\n");
    // gettimeofday(&start,NULL); //start timer here
    // // maximum(c, a, ring_size, size/batch_size, batch_size, map, nodeNet);
    // arg_max(b, c, a, ring_size, size/batch_size, batch_size, map, nodeNet);
    // // MaxPool(c, a, size, ring_size, ring_size + 10, map, nodeNet);
    // gettimeofday(&end,NULL);//stop timer here

    // timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    // printf("runtime for Argmax with data size %d = %ld us\n", size, timer);
    // Rss_Open(res, c, batch_size, map, ring_size, nodeNet);
    // for(int i =0; i< batch_size; i++) {
    //  printf("res[%i]  : %llu\n", i, res[i]);
    // }

    // printf("Testing matmult...\n");

    // // total = 1000;
    // total = 250;

    // gettimeofday(&start,NULL); //start timer here
    // //computation test Mult:
    // for (j = 0; j < total; j++) {
    //  // gettimeofday(&start,NULL); //start timer here
    //  Rss_MatMultArray(c, a, b, sqrt(size), sqrt(size), sqrt(size), ring_size, map, nodeNet);

    //  // Rss_MSB(c, a, size, ring_size, map, nodeNet);
    //  // gettimeofday(&end,NULL);//stop timer here

    // }
    // gettimeofday(&end,NULL);//stop timer here
    // timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    // // printf("runtime for mult with data size %d = %.6lf us\n", size, (double)(timer)/total); //double check time calculation is correct
    // printf("\nruntime for matmult with data size %f = %.6lf ms\n", sqrt(size), (double)(timer*0.001)/total); //double check time calculation is correct
    // // printf("Ratio for %d = %.6lf us\n", size,  (double)(timer/size)/total); //double check time calculation is correct
    // printf("Ratio for matmult %f = %.6lf ms\n\n", sqrt(size),  (double)(timer*0.001/size)/total); //double check time calculation is correct
    // // it should be though

    delete[] res;
    delete[] res2;
    delete[] res3;
    delete[] res_check;
    for (int i = 0; i < 3; i++) {

        delete[] Data1[i];
        delete[] Data2[i];
    }

    delete[] Data1;
    delete[] Data2;
    delete[] a;
    delete[] b;

    delete[] c[0];
    delete[] c[1];
    delete[] c;

    delete[] d[0];
    delete[] d[1];
    delete[] d;
    // for (i = 0; i < 2; i++) {
    //     delete [] c[i];
    //     delete [] d[i];
    // }
    // delete [] c;
    // delete [] d;

    // nodeNet->prg_clean();
    //*************************************************************************
    //random performance:

    //int byte = nodeNet->getbytes();
    // int size_radnom = 1000000;
    //  Lint *v_b = new Lint [size_radnom];
    // //memset(v_b,0,sizeof(Lint)*size);

    // gettimeofday(&start,NULL); //start timer here
    // uint8_t *buffer = new uint8_t [bytes*size_radnom];
    // nodeNet->prg_getrandom(1, bytes, size_radnom, buffer);

    // for(int i = 0 ; i<size_radnom; i++){
    //  //printf("call getrandom ");
    //  memcpy(v_b+i, buffer+bytes*i, bytes);

    // }
    // delete [] buffer;
    // delete [] v_b;
    // gettimeofday(&end,NULL);//stop timer here
    // timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    // printf("runtime for getradnom with data size %d = %ld us\n", size_radnom, timer);
}
