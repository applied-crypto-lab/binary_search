#include "../svm_ops.h"
extern "C"{
#include "../aes_ni.h"
}

// non-index preserving implementation, just returns the max value
void maximum(Lint** res, Lint **a, uint ring_size, uint size, uint batch_size, int *map, NodeNetwork* nodeNet){
    if(size > 1) {
        uint i, j;
        uint new_size = size >> 1; // rounding down
        uint new_size_ceil = (size+1)/2; // rounding up
        // printf("\nnew_size : %u\n", new_size);
        // printf("new_size_ceil : %u\n", new_size_ceil);
        // printf("size : %u\n", size);

        Lint **a0 = new Lint*[2]; 
        Lint **a1 = new Lint*[2]; 
        Lint **ap = new Lint*[2]; 
        for (i = 0; i < 2; i++) {
            a0[i] = new Lint [size*batch_size];
            memset(a0[i],0,sizeof(Lint)*size*batch_size);
            a1[i] = new Lint [size*batch_size];
            memset(a1[i],0,sizeof(Lint)*size*batch_size);
            ap[i] = new Lint [size*batch_size];
            memset(ap[i],0,sizeof(Lint)*size*batch_size);
        }

        Lint *res_check = new Lint[size*batch_size]; 
		memset(res_check,0,sizeof(Lint)*size*batch_size);


        for (j = 0; j < batch_size; j++) {
	        for (i = 0; i < new_size; i++) {

		        // copying the first half of a into a0
	            memcpy(a0[0] + i + j*new_size_ceil, a[0] + i + j*size, sizeof(Lint));
	            memcpy(a0[1] + i + j*new_size_ceil, a[1] + i + j*size, sizeof(Lint));


		        // copying the last half of a into a1
	            memcpy(a1[0] + i + j*new_size_ceil, a[0] + new_size + i + j*size, sizeof(Lint));
	            memcpy(a1[1] + i + j*new_size_ceil, a[1] + new_size + i + j*size, sizeof(Lint));
	        }
        } 

		// Rss_Open(res_check, a, size*batch_size, map, ring_size, nodeNet);
		// for (i = 0; i < size*batch_size; i++) {
		// 	printf("a[%i] : %llu\n", i, res_check[i]);
		// }
		// printf("\n");




		// Rss_Open(res_check, a0, new_size_ceil*batch_size, map, ring_size, nodeNet);
		// for (i = 0; i < new_size_ceil*batch_size; i++) {
		// 	printf("a0[%i] : %llu\n", i, res_check[i]);
		// }
		// printf("\n");

		// Rss_Open(res_check, a1, new_size_ceil*batch_size, map, ring_size, nodeNet);
		// for (i = 0; i < new_size_ceil*batch_size; i++) {
		// 	printf("a1[%i] : %llu\n", i, res_check[i]);
		// }

		// printf("\n");
		// Rss_Open(res_check, a1, new_size_ceil*batch_size, map, ring_size, nodeNet);
		// for (i = 0; i < new_size_ceil*batch_size; i++) {
		// 	printf("res_check[%i] : %llu\n", i, res_check[i]);
		// }

        // Rss_LT(res, a1, a, new_size, ring_size, map, nodeNet);

        Rss_LT(ap, a0, a1, new_size_ceil*batch_size, ring_size, map, nodeNet);
		// printf("\n");
		// printf("\n");

		// Rss_Open(res_check, ap, new_size_ceil*batch_size, map, ring_size, nodeNet);
		// for (i = 0; i < new_size_ceil*batch_size; i++) {
		// 	printf("res_check[%i] : %llu\n", i, res_check[i]);
		// }
		// Rss_Open(res_check, a, size*batch_size, map, ring_size, nodeNet);
		// for (i = 0; i < size*batch_size; i++) {
		// 	printf("a[%i] : %llu\n", i, res_check[i]);
		// }
		// printf("\n");


 		// checking for unused shares, putting them in a1
        if((size & 1)) {
        	// printf("HI\n");
        	for (i = 0; i < batch_size; i++) {
        		// printf("i = %u\n", i);
        		// printf("i*new_size_ceil + new_size = %u\n", i*new_size_ceil + new_size);
        		// printf("i*size + size-1 = %u\n", i*size + size-1);
	            ap[0][i*new_size_ceil + new_size] = a[0][i*size + size-1];
	            ap[1][i*new_size_ceil + new_size] = a[1][i*size + size-1];
        	}
            new_size += 1;

        }

  //       printf("\n");
		// Rss_Open(res_check, ap, new_size_ceil*batch_size, map, ring_size, nodeNet);
		// for (i = 0; i < new_size_ceil*batch_size; i++) {
		// 	printf("a'[%i] : %llu\n", i, res_check[i]);
		// }


        maximum(res, ap, ring_size, new_size, batch_size, map, nodeNet);



        for (i = 0; i < 2; i++) {
            delete [] a0[i];
            delete [] a1[i];
            delete [] ap[i];
        }
        delete [] a0;
        delete [] a1;
        delete [] ap;
        delete [] res_check;


    }  else {
    	// fill the maximums in at the ith position where i=[0,batch_size]
    	for (int i = 0; i < batch_size; i++) {
	        res[0][i] = a[0][i];
	        res[1][i] = a[1][i];
    	}

    }

}


// returns the index of the maximum a value (res_index), as well as the value itself (res)
void arg_max(Lint** res, Lint** res_index,  Lint **a, uint ring_size, uint size, uint batch_size,  int *map, NodeNetwork* nodeNet){

    if(size > 1) {

        uint i, j;
        uint new_size = size >> 1; // rounding down
        uint new_size_original = size >> 1; // rounding down
        uint new_size_ceil = (size+1)/2; // rounding up
	    int pid = nodeNet->getID();
	    // printf("size : %u\n", size);
	    // printf("new_size : %u\n", new_size);
     

	    uint total = size*batch_size;
	    uint total_new = new_size*batch_size;
	    uint total_new_ceil = new_size_ceil*batch_size;


        Lint **a0 = new Lint*[2];
        Lint **a1 = new Lint*[2]; 
        Lint **ap = new Lint*[2]; 
	    Lint **c = new Lint*[2];
	    Lint **diff = new Lint*[2];
		Lint **d0 = new Lint*[2];
	    Lint **index = new Lint*[2];

		Lint *res_check = new Lint[total]; 
		memset(res_check,0,sizeof(Lint)*total);


        for (i = 0; i < 2; i++) {
            a0[i] = new Lint [total];
            memset(a0[i],0,sizeof(Lint)*total);

            a1[i] = new Lint [total];
            memset(a1[i],0,sizeof(Lint)*total);
			
			ap[i] = new Lint [total];
            memset(ap[i],0,sizeof(Lint)*total);
	        
	        c[i] = new Lint [total];
            memset(c[i],0,sizeof(Lint)*total);
	        diff[i] = new Lint [total];
            memset(diff[i],0,sizeof(Lint)*total);
	        d0[i] = new Lint [total];
            memset(d0[i],0,sizeof(Lint)*total);
	        index[i] = new Lint [total];
            memset(index[i],0,sizeof(Lint)*total);
        }

        for (j = 0; j < batch_size; j++) {
	        for (i = 0; i < new_size; i++) {
	

		        // copying the first half of a into a0
	            memcpy(a0[0] + i + j*new_size_ceil, a[0] + i + j*size, sizeof(Lint));
	            memcpy(a0[1] + i + j*new_size_ceil, a[1] + i + j*size, sizeof(Lint));


		        // copying the last half of a into a1
	            memcpy(a1[0] + i + j*new_size_ceil, a[0] + new_size + i + j*size, sizeof(Lint));
	            memcpy(a1[1] + i + j*new_size_ceil, a[1] + new_size + i + j*size, sizeof(Lint));

	        }
        } 
        
        // checking for unused shares, putting them in a1
        // if((size & 1)) {
        // 	for (i = 0; i < batch_size; i++) {
	       //      a1[0][i*new_size_ceil + new_size] = a[0][i*size + size-1];
	       //      a1[1][i*new_size_ceil + new_size] = a[1][i*size + size-1];
        // 	}
        //     new_size += 1;
        // }

	    Lint b1 = 0;
	    Lint b2 = 0;
	    switch(pid){
	        case 1:
	            b1 = 1;
	            b2 = 0;
	            break;
	        case 2:
	            b1 = 0;
	            b2 = 0;
	            break;
	        case 3:
	            b1 = 0;
	            b2 = 1;
	            break;
	    }



		// Rss_Open(res_check, a0, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("a0[%i] : %llu\n", i, res_check[i]);
		// }
		// printf("\n");

		// Rss_Open(res_check, a1, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("a1[%i] : %llu\n", i, res_check[i]);
		// }



	    for (i = 0; i < total_new_ceil; i++) {
	        diff[0][i] = a0[0][i] - a1[0][i];
	        diff[1][i] = a0[1][i] - a1[1][i];
	    }


	    Rss_MSB(c, diff, total_new_ceil, ring_size, map, nodeNet);

		// Rss_Open(res_check, diff, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("res_check[%i] : %llu\n", i, res_check[i]);
		// }


	    for (i = 0; i < total_new_ceil; i++) {
	        diff[0][i] = a1[0][i] - a0[0][i];
	        diff[1][i] = a1[1][i] - a0[1][i];
	    }


	    Rss_Mult(d0, c, diff, total_new_ceil, ring_size, map, nodeNet);

	    for (i = 0; i < total_new_ceil; i++) {
	        ap[0][i] = d0[0][i] + a0[0][i];
	        ap[1][i] = d0[1][i] + a0[1][i]; 
	    }

		// Rss_Open(res_check, c, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("c[%i] : %llu\n", i, res_check[i]);
		// }
	 //    // new_size -=1;

	    for (j = 0; j < batch_size; j++) {
    		// printf("j : %u\n", j);
	    	for (i = 0; i < new_size; i++) {
	    		// printf("i : %u\n", i);
	    		// printf("j*new_size + i : %u\n", j*new_size_ceil + i);
		        index[0][j*new_size_ceil + i] = c[0][j*new_size_ceil + i] * (new_size) + i*b1;
		        index[1][j*new_size_ceil + i] = c[1][j*new_size_ceil + i] * (new_size) + i*b2;
	    	}
	    	// printf("hi\n");
	    }
	    
		// Rss_Open(res_check, index, total_new_ceil, map, ring_size, nodeNet);

		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("res_check[%i] : %llu\n", i, res_check[i]);
		// }


  //       printf("\n");
		// Rss_Open(res_check, index, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("index'[%i] : %llu\n", i, res_check[i]);
		// }
        if((size & 1)) {
        	for (i = 0; i < batch_size; i++) {
	            ap[0][i*new_size_ceil + new_size] = a[0][i*size + size-1];
	            ap[1][i*new_size_ceil + new_size] = a[1][i*size + size-1];

		        index[0][i*new_size_ceil + new_size] =  b1*(size-1);
		        index[1][i*new_size_ceil + new_size] =  b2*(size-1);
        	}
            new_size += 1;
        } 


  //       printf("\n");
		// Rss_Open(res_check, index, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("index'[%i] : %llu\n", i, res_check[i]);
		// }





        arg_max_helper(res, res_index, ap, index, ring_size, new_size, batch_size, map, nodeNet);

	    for(i = 0; i < 2; i++){
            delete [] a0[i];
            delete [] a1[i];
            delete [] ap[i];
	        delete [] c[i];
	        delete [] diff[i];
	        delete [] d0[i];
	        delete [] index[i];
	    }

        delete [] a0;
        delete [] a1;
        delete [] ap;
	    delete [] c;
	    delete [] diff;
	    delete [] d0;
	    delete [] index;
	    delete [] res_check;

    }  
    else {

		for (int i = 0; i < batch_size; i++) {
	        res[0][i] = a[0][i];
	        res[1][i] = a[1][i];
            res_index[0][i] = 0;
	        res_index[1][i] = 0;
    	}
    }

}



void arg_max_helper(Lint** res, Lint** res_index, Lint **a, Lint **a_index, uint ring_size, uint size,  uint batch_size, int *map, NodeNetwork* nodeNet){

    if(size > 1) {

        Lint i, j;
        uint new_size = size >> 1; // rounding down
        uint new_size_ceil = (size+1)/2; // rounding up
	    int pid = nodeNet->getID();


	    uint total = size*batch_size;
	    uint total_new = new_size*batch_size;
	    uint total_new_ceil = new_size_ceil*batch_size;

        // printf("\nnew_size : %u\n", new_size);
        // printf("new_size_ceil : %u\n", new_size_ceil);
        // printf("size : %u\n", size);

        Lint **a0 = new Lint*[2];
        Lint **a1 = new Lint*[2];
        Lint **ap = new Lint*[2];
        Lint **a0_index = new Lint*[2];
        Lint **a1_index = new Lint*[2];
	    Lint **c = new Lint*[2];
	    Lint **diff = new Lint*[2];
		Lint **d0 = new Lint*[2];
	    Lint **index = new Lint*[2];

        for (i = 0; i < 2; i++) {
            a0[i] = new Lint [total];
            memset(a0[i],0,sizeof(Lint)*total);
            a0_index[i] = new Lint [total];
            memset(a0_index[i],0,sizeof(Lint)*total);

            a1[i] = new Lint [total];
            memset(a1[i],0,sizeof(Lint)*total);
            a1_index[i] = new Lint [total];
            memset(a1_index[i],0,sizeof(Lint)*total);


            ap[i] = new Lint [total];
            memset(ap[i],0,sizeof(Lint)*total);


	        c[i] = new Lint [total];
            memset(c[i],0,sizeof(Lint)*total);
	        diff[i] = new Lint [total];
            memset(diff[i],0,sizeof(Lint)*total);

	        d0[i] = new Lint [total];
            memset(d0[i],0,sizeof(Lint)*total);
	        index[i] = new Lint [total];
            memset(index[i],0,sizeof(Lint)*total);
        }


		Lint *res_check = new Lint[total]; 
		memset(res_check,0,sizeof(Lint)*total);

        // copying the last half of a into a0
        // for (i = 0; i < new_size; i++) {
        //     memcpy(a0[0] + i, a[0] + new_size + i, sizeof(Lint));
        //     memcpy(a0[1] + i, a[1] + new_size + i, sizeof(Lint));
        //     memcpy(a0_index[0] + i, a_index[0] + new_size + i, sizeof(Lint));
        //     memcpy(a0_index[1] + i, a_index[1] + new_size + i, sizeof(Lint));
        // }


        for (j = 0; j < batch_size; j++) {
	        for (i = 0; i < new_size; i++) {

		        // copying the first half of a into a0
	            memcpy(a0[0] + i + j*new_size_ceil, a[0] + i + j*size, sizeof(Lint));
	            memcpy(a0[1] + i + j*new_size_ceil, a[1] + i + j*size, sizeof(Lint));
	            memcpy(a0_index[0] + i + j*new_size_ceil, a_index[0] + i + j*size, sizeof(Lint));
	            memcpy(a0_index[1] + i + j*new_size_ceil, a_index[1] + i + j*size, sizeof(Lint));



		        // copying the last half of a into a1
	            memcpy(a1[0] + i + j*new_size_ceil, a[0] + new_size + i + j*size, sizeof(Lint));
	            memcpy(a1[1] + i + j*new_size_ceil, a[1] + new_size + i + j*size, sizeof(Lint));
	            memcpy(a1_index[0] + i + j*new_size_ceil, a_index[0] + new_size + i + j*size, sizeof(Lint));
	            memcpy(a1_index[1] + i + j*new_size_ceil, a_index[1] + new_size + i + j*size, sizeof(Lint));

	        }
        } 



		// Rss_Open(res_check, a0, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("a0[%i] : %llu\n", i, res_check[i]);
		// }
		// printf("\n");

		// Rss_Open(res_check, a1, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("a1[%i] : %llu\n", i, res_check[i]);
		// }


	    for (i = 0; i < total_new_ceil; i++) {
	        diff[0][i] = a0[0][i] - a1[0][i];
	        diff[1][i] = a0[1][i] - a1[1][i];
	    }

	    Rss_MSB(c, diff, total_new_ceil, ring_size, map, nodeNet);


		// Rss_Open(res_check, c, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("c[%i] : %llu\n", i, res_check[i]);
		// }

	    for (i = 0; i < total_new_ceil; i++) {
	        diff[0][i] = a1[0][i] - a0[0][i];
	        diff[1][i] = a1[1][i] - a0[1][i];
	    }

	    Rss_Mult(d0, c, diff, total_new_ceil, ring_size, map, nodeNet);

	    for (i = 0; i < total_new_ceil; i++) {
	        ap[0][i] = d0[0][i] + a0[0][i];
	        ap[1][i] = d0[1][i] + a0[1][i]; 

	    	
	        diff[0][i] = a1_index[0][i] - a0_index[0][i];
	        diff[1][i] = a1_index[1][i] - a0_index[1][i];
	    }


	    Rss_Mult(d0, c, diff, total_new_ceil, ring_size, map, nodeNet);

	    for (i = 0; i < total_new_ceil; i++) {
	        index[0][i] = d0[0][i] + a0_index[0][i];
	        index[1][i] = d0[1][i] + a0_index[1][i]; 
	    }

  //       printf("\n");
		// Rss_Open(res_check, ap, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("ap[%i] : %llu\n", i, res_check[i]);
		// }

        if((size & 1)) {

        	for (i = 0; i < batch_size; i++) {
	            ap[0][i*new_size_ceil + new_size] = a[0][i*size + size-1];
	            ap[1][i*new_size_ceil + new_size] = a[1][i*size + size-1];

		        index[0][i*new_size_ceil + new_size] = a_index[0][i*size + size-1];
		        index[1][i*new_size_ceil + new_size] = a_index[1][i*size + size-1];
        	}
            new_size += 1;
        } 
  //       printf("\n");
		// Rss_Open(res_check, ap, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("ap'[%i] : %llu\n", i, res_check[i]);
		// }


  //       printf("\n");
		// Rss_Open(res_check, index, total_new_ceil, map, ring_size, nodeNet);
		// for (i = 0; i < total_new_ceil; i++) {
		// 	printf("index'[%i] : %llu\n", i, res_check[i]);
		// }


        arg_max_helper(res, res_index, ap, index, ring_size, new_size, batch_size, map, nodeNet);

	    for(i = 0; i < 2; i++){
            delete [] a0[i];
            delete [] a0_index[i];
            delete [] a1[i];
            delete [] a1_index[i];
            delete [] ap[i];

	        delete [] c[i];
	        delete [] diff[i];
	        delete [] d0[i];
	        delete [] index[i];
	    }

        delete [] a0;
        delete [] a1;
        delete [] ap;
        delete [] a0_index;
        delete [] a1_index;
	    delete [] c;
	    delete [] diff;
	    delete [] d0;
	    delete [] index;

	    delete [] res_check;


    }  

    else {

		for (int i = 0; i < batch_size; i++) {
	        res[0][i] = a[0][i];
	        res[1][i] = a[1][i];
            res_index[0][i] = a_index[0][i]; 
	        res_index[1][i] = a_index[1][i];
    	}
    }

}




