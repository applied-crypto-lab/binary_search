#include "init.h"

// key stuff
uint8_t raw_key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
__m128i * prg_key = offline_prg_keyschedule(raw_key);
//setup prg seed(k1, k2, k3)
uint8_t key1[] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
uint8_t key2[] = {0xa2, 0x34, 0x6f, 0x67, 0x10, 0x1b, 0x13, 0xa3, 0x56, 0x45, 0x90, 0xb2, 0x13, 0xe3, 0x23, 0x24};




void splitData(Lint **output, long long  *input, uint m, uint n,  uint ring_size, NodeNetwork* nodeNet){

    uint i, j, k;
    int pid = nodeNet->getID();

    Lint **splitInput = new Lint *[3];
    for(i = 0; i< 3; i++){
        splitInput[i] = new Lint [(m*n)];
        memset(splitInput[i],0,sizeof(Lint)*(m*n));
    }

    for (i = 0; i < m*n; i++) {
        prg_aes_ni(splitInput[0]+i, key1, prg_key);
        prg_aes_ni(splitInput[1]+i, key1, prg_key);

        splitInput[0][i] = splitInput[0][i] & nodeNet->SHIFT[ring_size];
        splitInput[1][i] = splitInput[1][i] & nodeNet->SHIFT[ring_size]; 
        // memset(splitInput[0], 0, sizeof(Lint)*(m*n)); //USED FOR TESTING
        // memset(splitInput[1], 0, sizeof(Lint)*(m*n)); //USED FOR TESTING
        
        splitInput[2][i] = ((input[i] & ((Lint(1) << Lint(ring_size)) - Lint(1))) - splitInput[0][i] - splitInput[1][i]) & nodeNet->SHIFT[ring_size];

        // splitInput[2][i] = ((input[i] & ((Lint(1) << Lint(ring_size)) - Lint(1))) - splitInput[0][i] - splitInput[1][i]);



        // splitInput[2][i] = (input[i]  - splitInput[0][i] - splitInput[1][i]) & nodeNet->SHIFT[ring_size];



    }

    switch(pid){
        case 1:
            output[0] = splitInput[1];
            output[1] = splitInput[2];
            break;
        case 2:
            output[0] = splitInput[2];
            output[1] = splitInput[0];
            break;
        case 3:
            output[0] = splitInput[0];
            output[1] = splitInput[1];
            break;
    }

}

void readFile(long long  *output, uint rows, uint cols, Lint fractional, std::string path){
    
    uint i=0,j=0,k=0,len,last=0;
    std::ifstream file(path); 
    std::string num = "";
    std::string line;
    if(file.is_open())
    {
        while ( getline(file,line) ) { 

           k=0,last=0,j=0;
           len=line.length();

            while(k!=len){
                if(line[k]==',' || k == len-1){

                    //for converting string into number
                    // output[i*cols+j] = roundf(atof(num.append(line,last,k).c_str()))*fractional;
                    output[i*cols+j] = roundf((atof(num.append(line,last,k).c_str()))*fractional);
                    // output[i*cols+j] = (long long )(atof(num.append(line,last,k).c_str())*fractional);
                    // printf("%f\n", output[i][j]);

                    //Emtying string for getting next data
                    num="";
                    //increasing column number after saving data
                    j++;

                    if(k!=len-1)
                        last=k+1;
                }
                k++;
            }
            //increase row number for array
            i++;
        }
        file.close();
    }
    else printf("Input file error!\n");


}

void splitX(Lint **output, long long  *input, uint size, uint batch_size, uint tracker, uint ring_size, NodeNetwork* nodeNet){


    uint i, j, k;
    int pid = nodeNet->getID();

    Lint **splitInput = new Lint *[3];
    for(i = 0; i< 3; i++){
        splitInput[i] = new Lint [(size*batch_size)];
        memset(splitInput[i],0,sizeof(Lint)*(size*batch_size));
    }

    // for (i = 0; i < size; i++) {
    //     // printf("res[%i] : %llu \n", i, res[i] ); //& nodeNet->SHIFT[48]);
    //     printf("input[%i] : %llu \n", i, input[tracker*size*batch_size + i] ); //& nodeNet->SHIFT[48]);
    //     // print_binary(res[i], final_ring_size);
    // }
    for (i = 0; i < size*batch_size; i++) {
        prg_aes_ni(splitInput[0]+i, key1, prg_key);
        prg_aes_ni(splitInput[1]+i, key1, prg_key);

        splitInput[0][i] = splitInput[0][i] & nodeNet->SHIFT[ring_size];
        splitInput[1][i] = splitInput[1][i] & nodeNet->SHIFT[ring_size]; 
        // memset(splitInput[0], 0, sizeof(Lint)*(m*n)); //USED FOR TESTING
        // memset(splitInput[1], 0, sizeof(Lint)*(m*n)); //USED FOR TESTING
        
        splitInput[2][i] = ((input[tracker*size*batch_size + i] & ((Lint(1) << Lint(ring_size)) - Lint(1))) - splitInput[0][i] - splitInput[1][i]) & nodeNet->SHIFT[ring_size];
        // splitInput[2][i] = ((input[i] & ((Lint(1) << Lint(ring_size)) - Lint(1))) - splitInput[0][i] - splitInput[1][i]) & nodeNet->SHIFT[ring_size];

        // splitInput[2][i] = ((input[i] & ((Lint(1) << Lint(ring_size)) - Lint(1))) - splitInput[0][i] - splitInput[1][i]);



        // splitInput[2][i] = (input[i]  - splitInput[0][i] - splitInput[1][i]) & nodeNet->SHIFT[ring_size];



    }

    switch(pid){
        case 1:
            output[0] = splitInput[1];
            output[1] = splitInput[2];
            break;
        case 2:
            output[0] = splitInput[2];
            output[1] = splitInput[0];
            break;
        case 3:
            output[0] = splitInput[0];
            output[1] = splitInput[1];
            break;
    }

}


void readX(long long  *output, uint rows, uint cols,  std::string path){
// void readX(long long  **output, uint rows, uint cols, Lint fractional, std::string path){
    
    uint i=0,j=0,k=0,len,last=0;
    std::ifstream file(path); 
    std::string num = "";
    std::string line;
    if(file.is_open())
    {
        while ( getline(file,line) ) { 

           k=0,last=0,j=0;
           len=line.length();

            while(k!=len){
                if(line[k]==',' || k == len-1){

                    //for converting string into number
                    // output[i][j] = roundf(atof(num.append(line,last,k).c_str()))*fractional;
                    output[i*cols+j] = roundf(atof(num.append(line,last,k).c_str()));
                    // output[i*cols+j] = (long long )(atof(num.append(line,last,k).c_str())*fractional);
                    // printf("%f\n", output[i][j]);

                    //Emtying string for getting next data
                    num="";
                    //increasing column number after saving data
                    j++;

                    if(k!=len-1)
                        last=k+1;
                }
                k++;
            }
            //increase row number for array
            i++;
        }
        file.close();
    }
    else printf("Input file error!\n");


}


void readB(long long  *output, uint rows, uint cols, Lint fractional, std::string path, uint n){
    
    uint i=0,j=0,k=0,len,last=0;
    std::ifstream file(path); 
    std::string num = "";
    std::string line;
    if(file.is_open())
    {
        while ( getline(file,line) ) { 

           k=0,last=0,j=0;
           len=line.length();

            while(k!=len){
                if(line[k]==',' || k == len-1){

                    //for converting string into number
                    // output[i*cols+j] = roundf(atof(num.append(line,last,k).c_str()))*fractional;
                    output[i*cols+j] = roundf((atof(num.append(line,last,k).c_str()))*pow(fractional,n));
                    // output[i*cols+j] = (long long )(atof(num.append(line,last,k).c_str())*fractional);
                    // printf("%f\n", output[i][j]);

                    //Emtying string for getting next data
                    num="";
                    //increasing column number after saving data
                    j++;

                    if(k!=len-1)
                        last=k+1;
                }
                k++;
            }
            //increase row number for array
            i++;
        }
        file.close();
    }
    else printf("Input file error!\n");


}
