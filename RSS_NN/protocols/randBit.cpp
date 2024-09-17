#include "../Rss_Op.h"
extern "C"{
#include "../aes_ni.h"
}


void Rss_RandBit(Lint** b, uint size, uint ring_size, int *map, NodeNetwork* nodeNet){

    int pid = nodeNet->getID();
    uint i;

    uint bytes = ((ring_size+2) + 7) >> 3;


    Lint **u = new Lint *[2];
    Lint **a = new Lint *[2];
    Lint **d = new Lint *[2];
    
    for(i = 0; i < 2; i++){
        u[i] = new Lint [size];
        a[i] = new Lint [size];
        d[i] = new Lint [size];
    }

    Lint *e = new Lint [size];
    Lint *c = new Lint [size];


    uint8_t *buffer = new uint8_t [bytes*size];

    nodeNet->prg_getrandom(0, bytes, size, buffer);
    // memset(buffer, 0, bytes*size); //USED FOR TESTING
    for (i = 0 ; i<size; i++) {
        //nodeNet->prg_getrandom(1, bytes, c[0]+i);
          memcpy(u[0]+i, buffer + i*bytes, bytes);
    }
    nodeNet->prg_getrandom(1, bytes, size, buffer);
    // memset(buffer, 0, bytes*size); //USED FOR TESTING
    for (i = 0 ; i<size; i++) {
        //nodeNet->prg_getrandom(1, bytes, c[0]+i);
        memcpy(u[1]+i, buffer + i*bytes, bytes);
    }

        // used to make a odd, we only add 1 to one share of a
        // All shares will be doubled
    Lint a1 = 0;
    Lint a2 = 0;
    switch(pid){
        case 1:
            a1 = 1;
            // a1 = 0; //USED FOR TESTING
            a2 = 0;
            break;
        case 2:
            a1 = 0;
            a2 = 0;
            break;
        case 3:
            a1 = 0;
            a2 = 1;
            // a2 = 0; //USED FOR TESTING
            break;
    }

    for(i = 0 ; i<size; i++){
        // ensuring [a] is odd
        a[0][i] = Lint(Lint(2)*u[0][i] + a1); 
        a[1][i] = Lint(Lint(2)*u[1][i] + a2); 
    }


    // squaring a
    Rss_MultPub(e, a, a, size, map, ring_size+2, nodeNet); //ringsize+2
    rss_sqrt_inv(c, e, size, ring_size+2);

    for(i = 0 ; i<size; i++){
        // shares of d will be even
        d[0][i] = c[i]*a[0][i] + a1;
        d[1][i] = c[i]*a[1][i] + a2;
    }   

    for(i = 0 ; i<size; i++){
        // equivalent to dividing by 2
        b[0][i] = d[0][i] >> Lint(1); 
        b[1][i] = d[1][i] >> Lint(1); 
    }   

    // // effectively combines the two loops into one, eliminates d variable
    // for (i = 0; i < size; i++) {
    //     b[0][i] = (c[i]*a[0][i] + a1) >> Lint(1);
    //     b[1][i] = (c[i]*a[1][i] + a2) >> Lint(1);

    // }

    // freeing up
    delete [] c;
    delete [] e;
    for(i = 0; i < 2; i++){
        delete [] d[i];
        delete [] a[i];
        delete [] u[i];
    }
    delete [] d;
    delete [] a;
    delete [] u;    
}


void rss_sqrt_inv(Lint *c, Lint *e, uint size, uint ring_size) {

  Lint c1, c2, temp, d_;
  uint i, j;
  
  for (i = 0 ; i < size; i++){
    c1 = Lint(1);
    c2 = Lint(1);
    d_ = Lint(4); // 100 - the first mask

    for (j = 2; j < ring_size - 1; j++) {
        temp = e[i] - (c1)*(c1);
        if (temp != Lint(0)) {
            //get the jth+1 bit of temp, place it in jth position, and add to c1
            c1 += (temp & (d_ << Lint(1))) >> Lint(1);
        } 

        temp = Lint(1) - c1*c2;
        // get the jth bit of temp and add it to c2
        c2 += temp & d_;
        d_ = d_ << Lint(1);
    }
    // last round for the inv portion
    temp = Lint(1) - c1*c2;
    c[i] = c2 + (temp & d_);
    
    }   
}
