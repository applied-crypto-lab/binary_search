#include "../Rss_Op.h"
extern "C"{
#include "../aes_ni.h"
}

void Rss_Dot(Lint* c, Lint** a, Lint** b, uint n, uint ring_size, int *map, NodeNetwork *nodeNet) {
  //  [n] size a;   [n] size b                                    
  //  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;   
  //  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;   
  //  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;   

  // uint bytes = (nodeNet->RING[ring_size]+7)>>3;                
  uint bytes = (ring_size+7)>>3;
  int i, j, k; // used for loops                                  

  Lint *v_a = new Lint [1];
  memset(v_a,0,sizeof(Lint)*1);

  Lint *send_buf = new Lint [1];
  memset(send_buf,0,sizeof(Lint));
  Lint *recv_buf = new Lint [1];
  memset(recv_buf,0,sizeof(Lint));

  uint8_t *buffer = new uint8_t [bytes];
  nodeNet->prg_getrandom(1, bytes, 1, buffer);

  for(k = 0; k < n; k++){
      v_a[0] += a[0][k] * b[0][k] + a[0][k] * b[1][k] + a[1][k] * b[0][k];
  }
  memcpy(c, buffer, bytes);
  //nodeNet->prg_getrandom(1, bytes, c[0][i]+j);                  
  v_a[0] = v_a[0] - c[0];

  //send v_a                                                      
  nodeNet->SendAndGetDataFromPeer(map[0], map[1], v_a, recv_buf, 1, ring_size);
  memcpy(c+1, recv_buf, sizeof(Lint));

  nodeNet->prg_getrandom(0, bytes, 1, buffer);
  c[1] = c[1] + c[0];
  //nodeNet->prg_getrandom(0, bytes, c[0][i]+j);                  
  memcpy(c, buffer, bytes);
  c[0] = c[0] + v_a[0];

  //free                                                          
  delete [] send_buf;
  delete [] recv_buf;
  delete [] buffer;
  delete [] v_a;
}

void Rss_DotArray(Lint** c, Lint** a, Lint** d, uint n, uint chunkSize, uint ring_size,  int *map, NodeNetwork *nodeNet, unsigned long* timer) {
  //  [n] size a;   [n] size b                                    
  //  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;   
  //  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;   
  //  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;   
  //  using d, select a chunk of size chunkSize from a into c     
  // uint bytes = (nodeNet->RING[ring_size]+7)>>3;                
  struct timeval start;
  struct timeval end;

  uint bytes = (ring_size+7)>>3;
  int i, j, k; // used for loops                                  


  Lint *v_a = new Lint [chunkSize];
  memset(v_a,0,sizeof(Lint)*chunkSize);

  Lint *recv_buf = new Lint [chunkSize];
  memset(recv_buf,0,sizeof(Lint)*chunkSize);
  int chunkNo = n/chunkSize;
  uint8_t *buffer = new uint8_t [bytes*chunkSize];
  nodeNet->prg_getrandom(1, bytes, chunkSize, buffer);

  int index = 0;
  gettimeofday(&start,NULL);//stop timer here 

  for(int i = 0; i < chunkSize; i++){
    for(int j = 0; j < chunkNo; j++){
      v_a[i] += a[0][i + j * chunkSize] * d[0][j] + a[0][i + j * chunkSize] * d[1][j] + a[1][i + j * chunkSize] * d[0][j];
    }
    memcpy(c[0]+i, buffer + i*bytes, bytes);
    //nodeNet->prg_getrandom(1, bytes, c[0][i]+j);
    v_a[i] = v_a[i] - c[0][i];
  }
  gettimeofday(&end,NULL);//stop timer here 
  *timer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;


  //send v_a         
  nodeNet->SendAndGetDataFromPeer(map[0], map[1], v_a, recv_buf, chunkSize, ring_size);
  gettimeofday(&start,NULL);//stop timer here 
  memcpy(c[1], recv_buf, sizeof(Lint)*chunkSize);


  nodeNet->prg_getrandom(0, bytes, chunkSize, buffer);
  for(int i = 0; i < chunkSize; ++i){
    c[1][i] = c[1][i] + c[0][i];
    //nodeNet->prg_getrandom(0, bytes, c[0][i]+j);
    memcpy(c[0]+i, buffer+i*bytes, bytes);
    c[0][i] = c[0][i] + v_a[i];
  }
  //free 
  delete [] recv_buf;
  delete [] buffer;
  delete [] v_a;
  gettimeofday(&end,NULL);//stop timer here  
  *timer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
}

void Rss_MatMult(Lint*** c, Lint*** a, Lint*** b, uint m, uint n, uint s, uint ring_size,  int *map, NodeNetwork *nodeNet) {
    //  [m][n] size matrix a;   [n][s] size matrix b
    //  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
    //  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
    //  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;

    // uint bytes = (nodeNet->RING[ring_size]+7)>>3;
    uint bytes = (ring_size+7)>>3;
    int i, j, k; // used for loops

    Lint **v_a = new Lint *[m];
    for(i = 0; i < m; i++){
        v_a[i] = new Lint [s];
        memset(v_a[i],0,sizeof(Lint)*s);
    }

    Lint *send_buf = new Lint [m*s];
    memset(send_buf,0,sizeof(Lint)*m*s);
    Lint *recv_buf = new Lint [m*s];
    memset(recv_buf,0,sizeof(Lint)*m*s);

    uint8_t *buffer = new uint8_t [bytes*m*s];
    nodeNet->prg_getrandom(1, bytes, m*s, buffer);


    for(i = 0; i < m; i++){
        for(j = 0; j < s; j++){
            for(k = 0; k < n; k++){
              v_a[i][j] += a[0][i][k] * b[0][k][j] + a[0][i][k] * b[1][k][j] + a[1][i][k] * b[0][k][j];  
          }
          memcpy(c[0][i]+j, buffer + (i*s+j)*bytes, bytes);
            //nodeNet->prg_getrandom(1, bytes, c[0][i]+j);
          v_a[i][j] = v_a[i][j] - c[0][i][j];

      }
    }

    //send v_a
    for(i = 0; i < m; i++){
        memcpy(send_buf+i*s, v_a[i], sizeof(Lint)*s);
    }
    nodeNet->SendAndGetDataFromPeer(map[0], map[1], send_buf, recv_buf, m*s, ring_size);
    for(i = 0; i < m; i++){
       memcpy(c[1][i], recv_buf+i*s, sizeof(Lint)*s);
    }

    nodeNet->prg_getrandom(0, bytes, m*s, buffer);
    for(i = 0; i < m; i++){
        for(j = 0; j < s; j++){
            c[1][i][j] = c[1][i][j] + c[0][i][j];

                //nodeNet->prg_getrandom(0, bytes, c[0][i]+j);
            memcpy(c[0][i]+j, buffer + (i*s+j)*bytes, bytes);
            c[0][i][j] = c[0][i][j] + v_a[i][j];
        }
    }
        //free
    delete [] send_buf;
    delete [] recv_buf;
    delete [] buffer;
    for(i = 0; i< m; i++){
        delete [] v_a[i];
    }
    delete [] v_a;
}





void Rss_MatMultArray(Lint** c, Lint** a, Lint** b, uint m, uint n, uint s, uint ring_size, int *map, NodeNetwork *nodeNet){
    // modified implementation that uses a 1d array representation of a matrix
    // a [m*n] = i*n+j (accessing the (i,j)th element)
    // [m][n] size matrix a;   [n][s] size matrix b
    // For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
    // For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
    // For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;
    uint bytes = (ring_size+7)>>3;
    uint i, j, k; // used for loops
    // printf("bytes: %u\n", bytes);

    Lint *v = new Lint [m*s];
    memset(v,0,sizeof(Lint)*(m*s));


    Lint *send_buf = new Lint [m*s];
    memset(send_buf,0,sizeof(Lint)*m*s);
    Lint *recv_buf = new Lint [m*s];
    memset(recv_buf,0,sizeof(Lint)*m*s);

    uint8_t *buffer = new uint8_t [bytes*m*s];
    nodeNet->prg_getrandom(1, bytes, m*s, buffer);
    // memset(buffer, 0, bytes*m*s); //USED FOR TESTING

    for(i = 0; i < m; i++){
        for(j = 0; j < s; j++){
            for(k = 0; k < n; k++){
              v[i*s+ j] += a[0][i*n + k] * b[0][k*s + j] + a[0][i*n + k] * b[1][k*s + j] + a[1][i*n + k] * b[0][k*s + j];  
            }
            memcpy(c[0] + (i*s+j), buffer + (i*s+j)*bytes, bytes);
            v[i*s+j] = v[i*s+j] - c[0][i*s + j];
        }
    }

    //send v_a
    for(i = 0; i < m; i++){
        // memcpy(send_buf+i*s, v+i, sizeof(Lint)*s);
        memcpy(send_buf+i*s, v+i*s, sizeof(Lint)*s);
    }
    nodeNet->SendAndGetDataFromPeer(map[0], map[1], send_buf, recv_buf, m*s, ring_size);

    for(i = 0; i < m; i++){
       memcpy(c[1] + i*s, recv_buf+i*s, sizeof(Lint)*s); // PROBLEM HERE
    }

    nodeNet->prg_getrandom(0, bytes, m*s, buffer);
    // memset(buffer, 0, bytes*m*s); //USED FOR TESTING

    for(i = 0; i < m; i++){
        for(j = 0; j < s; j++){
            c[1][i*s+j] = c[1][i*s+j] + c[0][i*s+j];

            memcpy(c[0]+(i*s+j), buffer + (i*s+j)*bytes, bytes);
            c[0][i*s + j] = c[0][i*s + j] + v[i*s+j];
        }
    }

    delete [] send_buf;
    delete [] recv_buf;
    delete [] buffer;
    delete [] v;
    // for(i = 0; i< m; i++){
    //     delete [] v_a[i];
    // }
    // delete [] v_a;
}



void Rss_MatMultArray_batch(Lint** c, Lint** a, Lint** b, uint m, uint n, uint s, uint ring_size, uint batch_size, uint weight_flag, int *map, NodeNetwork *nodeNet){
    // modified implementation that uses a 1d array representation of a matrix
    // a [m*n] = i*n+j (accessing the (i,j)th element)
    // [m][n] size matrix a;   [n][s] size matrix b
    // ---> [m][s] size matrix c;
    // For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
    // For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
    // For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;

    // if weight_flag == 0 --> the a matrix is the same throughout the computation

    uint bytes = (ring_size+7)>>3;
    uint i, j, k, l; // used for loops
    // printf("bytes: %u\n", bytes);

    Lint *v = new Lint [batch_size*m*s];
    memset(v,0,sizeof(Lint)*(batch_size*m*s));


    Lint *send_buf = new Lint [batch_size*m*s];
    memset(send_buf,0,sizeof(Lint)*batch_size*m*s);
    Lint *recv_buf = new Lint [batch_size*m*s];
    memset(recv_buf,0,sizeof(Lint)*batch_size*m*s);

    uint8_t *buffer = new uint8_t [bytes*batch_size*m*s];
    nodeNet->prg_getrandom(1, bytes, batch_size*m*s, buffer);
    // memset(buffer, 0, bytes*batch_size*m*s); //USED FOR TESTING

    for (l = 0; l < batch_size; l++) {
        
        for(i = 0; i < m; i++){
            for(j = 0; j < s; j++){
                for(k = 0; k < n; k++){
                  v[l*m*s + i*s+ j] 
                  += a[0][weight_flag*(l*m*n) + i*n + k] 
                    * b[0][l*n*s + k*s + j] 
                      + a[0][weight_flag*(l*s*n) + i*n + k] 
                        * b[1][l*s*n + k*s + j] 
                          + a[1][weight_flag*(l*m*n) + i*n + k] 
                            * b[0][l*s*n + k*s + j];  
                }
                memcpy(c[0] + (l*m*s +i*s+j), buffer + (l*m*s +i*s+j)*bytes, bytes);
                v[l*m*s + i*s+j] = v[l*m*s + i*s+j] - c[0][l*m*s +i*s + j];
            }
        }
    }

    //send v_a
    for (l = 0; l < batch_size; l++) {
        
        for(i = 0; i < m; i++){
            // memcpy(send_buf+i*s, v+i, sizeof(Lint)*s);
            memcpy(send_buf + i*s + l*m*s, v + i*s + l*m*s, sizeof(Lint)*s);
        }
    }
    nodeNet->SendAndGetDataFromPeer(map[0], map[1], send_buf, recv_buf, batch_size*m*s, ring_size);

    for (l = 0; l < batch_size; l++) {
        for(i = 0; i < m; i++){
           memcpy(c[1] + i*s + l*m*s, recv_buf+i*s + l*m*s, sizeof(Lint)*s); // PROBLEM HERE
        }

    }

    nodeNet->prg_getrandom(0, bytes, batch_size*m*s, buffer);
    // memset(buffer, 0, bytes*batch_size*m*s); //USED FOR TESTING

    for (l = 0; l < batch_size; l++) {
        for(i = 0; i < m; i++){
            for(j = 0; j < s; j++){
                c[1][l*m*s + i*s+j] = c[1][l*m*s + i*s+j] + c[0][l*m*s + i*s+j];

                memcpy(c[0]+(l*m*s + i*s+j), buffer + (l*m*s + i*s+j)*bytes, bytes);
                c[0][l*m*s + i*s + j] = c[0][l*m*s + i*s + j] + v[l*m*s + i*s+j];
            }
        }
    }

    delete [] send_buf;
    delete [] recv_buf;
    delete [] buffer;
    delete [] v;
    // for(i = 0; i< m; i++){
    //     delete [] v_a[i];
    // }
    // delete [] v_a;
}
