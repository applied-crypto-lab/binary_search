#include "../Rss_Op.h"
extern "C"{
#include "../aes_ni.h"
}

void Rss_Mult_Bitwise(Lint** c, Lint** a, Lint** b, uint size, uint ring_size, int *map, NodeNetwork *nodeNet) 
//  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
//  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
//  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;
{
    // uint bytes = (nodeNet->RING[ring_size] + 7) >> 3;
    uint bytes = (ring_size + 7) >> 3;
    int i;

    Lint *v = new Lint [size];

    uint8_t *buffer = new uint8_t [bytes*size];   
    nodeNet->prg_getrandom(1, bytes, size, buffer);
    
    for (i = 0 ; i < size; i++) {
    //nodeNet->prg_getrandom(1, bytes, c[0]+i);
        memcpy(c[0]+i, buffer + i*bytes, bytes);
        v[i] =((a[0][i] & b[0][i]) ^ (a[0][i] & b[1][i]) ^ (a[1][i] & b[0][i])) ^ c[0][i];
    }
    //communication
    nodeNet->SendAndGetDataFromPeer(map[0], map[1], v, c[1], size, ring_size);
    nodeNet->prg_getrandom(0, bytes, size, buffer);

    for(i = 0; i < size; i++){
        c[1][i] = c[1][i] ^ c[0][i];
        //nodeNet->prg_getrandom(0, bytes, c[0]+i);
        memcpy(c[0]+i, buffer + i*bytes, bytes);
        c[0][i] = c[0][i] ^ v[i];
    }



    //free
    delete [] v;
    delete [] buffer;
}

void Rss_Mult(Lint** c, Lint** a, Lint** b, uint size, uint ring_size, int *map, NodeNetwork *nodeNet) 
//  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
//  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
//  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;
{
    // uint bytes = (nodeNet->RING[ring_size] + 7) >> 3;
    uint bytes = (ring_size + 7) >> 3;
    int i;

    Lint *v = new Lint [size];

    uint8_t *buffer = new uint8_t [bytes*size];   
    nodeNet->prg_getrandom(1, bytes, size, buffer);
    
    for (i = 0 ; i < size; i++) {
    //nodeNet->prg_getrandom(1, bytes, c[0]+i);
        memcpy(c[0]+i, buffer + i*bytes, bytes);
        v[i] = a[0][i] * b[0][i] + a[0][i] * b[1][i] + a[1][i] * b[0][i] - c[0][i];
    }
    //communication
    nodeNet->SendAndGetDataFromPeer(map[0], map[1], v, c[1], size, ring_size);
    nodeNet->prg_getrandom(0, bytes, size, buffer);


    for(i = 0; i < size; i++){
        c[1][i] = c[1][i] + c[0][i];
        //nodeNet->prg_getrandom(0, bytes, c[0]+i);
        memcpy(c[0]+i, buffer + i*bytes, bytes);
        c[0][i] = c[0][i] + v[i];
    }
    
    //free
    delete [] v;
    delete [] buffer;
}

void Rss_Mult_Byte(uint8_t** c, uint8_t** a, uint8_t** b, uint size, int *map, NodeNetwork *nodeNet) 
{
    // size == how many bytes we're multiplying
    // uint bytes = (size+8-1)>>3;  //number of bytes need to be send/recv
    // uint bytes = size;  //number of bytes need to be send/recv

    int i = 0;
    uint8_t *v = new uint8_t [size];
    nodeNet->prg_getrandom(1, 1, size, c[0]);  //<-- COMMENT OUT FOR TESTING
    // for(i = 0; i < 2; i++) memset(c[i], 0, sizeof(uint8_t)*size); //<-- COMMENT IN FOR TESTING
    
    for(i = 0 ; i < size; i++){  //do operations byte by byte
        v[i] =((a[0][i] & b[0][i]) ^ (a[0][i] & b[1][i]) ^ (a[1][i] & b[0][i])) ^ c[0][i];
    }

    //communication
    nodeNet->SendAndGetDataFromPeer_bit(map[0], map[1], v, c[1], size);
    for(i = 0; i < size; i++){
        c[1][i] = c[1][i] ^ c[0][i];
    }
    nodeNet->prg_getrandom(0, 1, size, c[0]); //<-- COMMENT OUT FOR TESTING
    for(i = 0; i < size; i++){
        c[0][i] = c[0][i] ^ v[i];
    }

    //free
    delete [] v;
}





void Rss_MultPub(Lint * c, Lint** a, Lint** b, uint size, int *map, uint ring_size, NodeNetwork* nodeNet) 
//  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
//  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
//  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;   
{
    int i, j, k; // used for loops

    // uint bytes = (nodeNet->RING[ring_size] +7) >> 3;
    uint bytes = (ring_size +7) >> 3;

    Lint **sendbuf = new Lint *[3];
    Lint **recvbuf = new Lint *[3];
    for(i = 0; i< 3; i++){
        sendbuf[i] = new Lint [size];
        memset(sendbuf[i],0,sizeof(Lint)*size);
        recvbuf[i] = new Lint [size];
        memset(recvbuf[i],0,sizeof(Lint)*size);
    }

    int pid = nodeNet->getID();
    Lint *v = new Lint [size];
    Lint *v_a = new Lint [size];

    Lint opa = 0;
    Lint opb = 0;
    switch(pid){
        case 1:
            opa = 1;
            opb = 1;
            break;
        case 2:
            opa = -1;
            opb = 1;
            break;
        case 3:
            opa = -1;
            opb = -1;
            break;
    }


    uint8_t *buffer = new uint8_t [bytes*size];
    nodeNet->prg_getrandom(0, bytes, size, buffer);
    for (i = 0 ; i<size; i++) {
        memcpy(v_a+i, buffer + i*bytes, bytes);
    }
    nodeNet->prg_getrandom(1, bytes, size, buffer);
    for (i = 0 ; i<size; i++) {
        memcpy(c+i, buffer + i*bytes, bytes);
    }

    for(i = 0; i<size; i++){
        v[i] = a[0][i] * b[0][i] + a[0][i] * b[1][i] + a[1][i] * b[0][i];
        c[i] = v[i] + opb*c[i] + opa*v_a[i];
    }

    //communication
    //move data into buf
    for(i = 1; i<= 3; i++){
        if(i == pid) continue;
        memcpy(sendbuf[i-1],c, sizeof(Lint)*size);
    }

    nodeNet->multicastToPeers(sendbuf, recvbuf, size, ring_size);

    memcpy(v_a, recvbuf[map[0]-1], sizeof(Lint)*size);
    memcpy(v, recvbuf[map[1]-1], sizeof(Lint)*size);

    for(i = 0; i<size; i++){
        //mask here
        c[i] = c[i] + v_a[i] + v[i];
        c[i] = c[i] & nodeNet->SHIFT[ring_size];
    }

        //free
    delete [] v;
    delete [] v_a;
    delete [] buffer;
    for(i = 0; i< 3; i++){
        delete [] sendbuf[i];
        delete [] recvbuf[i];
    }
    delete [] sendbuf;
    delete [] recvbuf;

}

