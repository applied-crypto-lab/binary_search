#include "../Rss_Op.h"
extern "C"{
#include "../aes_ni.h"
}

void print_binary(Lint n, uint size){
    // printf("size = %u\n", size);
    // uint i;
    uint temp = size-1;
    int i = size-1;
    // printf("i %u\n", i);
    uint b;
    while(i !=-1) {
        
        b = GET_BIT(n, temp);
        // printf("i %u\n", i);
        // printf("temp %u\n", temp);
        // printf("b = %llu\n", b);
        printf("%llu", b);

        temp--;
        i -= 1;
    }
    printf("\n");

}

