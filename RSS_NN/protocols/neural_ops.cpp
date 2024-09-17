#include "../neural_ops.h"
extern "C"{
#include "../aes_ni.h"
}

// does the actual "convolution" step required prior to matrix multiplication
// ksize = kernelsize
// we assume the size of res is sufficient to handle  
void im2col(Lint** res, Lint** a, uint channels, uint height, uint width, uint ksize, uint stride,  uint batch_size, int *map, NodeNetwork* nodeNet){

    for (uint i = 0; i < batch_size; i++) {
            
        uint pad = 0;

        uint c,h,w;
        uint height_col = (height + 2*pad - ksize) / stride + 1;
        uint width_col = (width + 2*pad - ksize) / stride + 1;
        uint channels_col = channels * ksize * ksize; 

        uint w_offset, h_offset, c_im, im_row, im_col, col_index;

        // used for batch optimization
        uint in_batch_size = channels*height*width;
        uint out_batch_size = channels_col*height_col*width_col;

        for (c = 0; c < channels_col; ++c) {
             w_offset = c % ksize;
             h_offset = (c / ksize) % ksize;
             c_im = c / ksize / ksize;
            for (h = 0; h < height_col; ++h) {
                for (w = 0; w < width_col; ++w) {

                    im_row = h_offset + h * stride;
                    im_col = w_offset + w * stride;
                    col_index = (c * height_col + h) * width_col + w;

                    // may also be unnecessary
                    if (im_row < 0 || im_col < 0 || (im_row >= height) || (im_col >= width)) {
                        res[0][col_index] = 0;
                        res[1][col_index] = 0;
                    } else {
                        res[0][col_index +i*out_batch_size] = a[0][i*in_batch_size + im_col + width*(im_row + height*c_im)];
                        res[1][col_index +i*out_batch_size] = a[1][i*in_batch_size + im_col + width*(im_row + height*c_im)];
                    }
                }
            }
        }
    }
}


// void Rss_MaxPool(Lint*** res, Lint*** a, uint m, uint n, uint winx, uint winy, uint ring_size, uint size, int *map, NodeNetwork* nodeNet){
void MaxPool(Lint** res, Lint** a, uint c, uint m, uint n, uint batch_size, uint ring_size, int *map, NodeNetwork* nodeNet){
    // takes in matrix of dimension m x n and pools according to the window 
    // output matrix dimensions are calculated based off of inputs
    // we exclude the z direction in function since it is one in all cases
    uint i, j, k, l, index; // used for loops
    // uint bytes = ((ring_size) + 7) >> 3;

    // uint size = (c*m*n)/2;
    uint size = (c*m*n)/2;
    // printf("size : %u\n", size);

    // Lint *res2 = new Lint [100000];
    // memset(res2,0,sizeof(Lint)*100000);



    Lint **x0 = new Lint*[2];
    Lint **x1 = new Lint*[2];
    Lint **temp = new Lint*[2];
    for (i = 0; i < 2; i++) {
        x0[i] = new Lint[size*batch_size];
        memset(x0[i],0,sizeof(Lint)*(size*batch_size));
        x1[i] = new Lint[size*batch_size];
        memset(x1[i],0,sizeof(Lint)*(size*batch_size));
        temp[i] = new Lint[size*batch_size];
        memset(temp[i],0,sizeof(Lint)*(size*batch_size));
    }

    for (j = 0; j < batch_size; j++) {
        
        for (i = 0; i < size; i++) {
            x0[0][j*size + i] = a[0][j*size*2 + 2*i];
            x0[1][j*size + i] = a[1][j*size*2 + 2*i];
            x1[0][j*size + i] = a[0][j*size*2 + 2*i + 1];
            x1[1][j*size + i] = a[1][j*size*2 + 2*i + 1];
        }
    }


    Rss_LT(temp, x0, x1, size*batch_size, ring_size, map, nodeNet);// c is the problem


    for (i = 0; i < 2; i++) {
        memset(x0[i],0,sizeof(Lint)*(size*batch_size));
        memset(x1[i],0,sizeof(Lint)*(size*batch_size));
        // memset(res[i],0,sizeof(Lint)*(size));
    }
    size = size / 2;
    // printf("size : %u\n", size);

    // Rss_Open(res2, temp, 16*12*12, map, ring_size, nodeNet);
    // for(int i =0; i< 30; i++) {
    //     printf("res[%i]  : %llu\n", i, res2[i]);
    // }
    for (l = 0; l < batch_size; l++) {
        k = 0;
        for (i = 0; i < c*m/2; i++) {
            for (j = 0; j < n/2; j++) {

                // printf("j + i*n : %u\n", j + i*n);
                // printf("j + m/2 + i*n : %u\n", j + m/2 +i*n);
                x0[0][l*size+k] = temp[0][2*l*size + j + i*n];
                x0[1][l*size+k] = temp[1][2*l*size + j + i*n]; 
                x1[0][l*size+k] = temp[0][2*l*size + j + m/2 + i*n];
                x1[1][l*size+k] = temp[1][2*l*size + j + m/2 + i*n];

                k++;

            }
        }
    }



    Rss_LT(res, x0, x1, size*batch_size, ring_size, map, nodeNet);

    // delete [] res2;
    // cleanup
    for(i = 0; i < 2; i++){
        delete [] x0[i];
        delete [] temp[i];
        delete [] x1[i];
    }
    delete [] x0;
    delete [] x1;
    delete [] temp;


}


// does not need to be changed for batch optimization
void ReLU(Lint** res, Lint** a,  uint size, uint ring_size, int *map, NodeNetwork* nodeNet){
    // takes in matrix of dimension m x n and calculates the ReLU for each element
    // output dimensions same as input
    uint i, j, k, index; // used for loops
    uint bytes = ((ring_size) + 7) >> 3;

    Lint **zero = new Lint*[2];
    for (i = 0; i < 2; i++) {
    	zero[i] = new Lint[size];
        memset(zero[i],0,sizeof(Lint)*(size));
    }
    Rss_LT(res, a, zero, size, ring_size, map, nodeNet);
    // cleanup 
    for(i = 0; i < 2; i++){
        delete [] zero[i];
    }
    delete [] zero;


}

void add_biases(Lint** res, Lint **a, Lint **b, uint m, uint n, uint batch_size, int *map, NodeNetwork* nodeNet){

    uint i, j, k;

    for (k = 0; k < batch_size; k++) {
        for (i = 0; i < m; i++) {
            for (j = 0; j < n; j++) {
               res[0][k*m*n + i*n + j] = a[0][k*m*n + i*n + j] + b[0][i];
               res[1][k*m*n + i*n + j] = a[1][k*m*n + i*n + j] + b[1][i];
            }
        }
    }


}
