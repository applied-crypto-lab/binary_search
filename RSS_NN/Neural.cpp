#include "Neural.h"

void neural_main(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint batch_size){

	int i, j, k;
	int pid = nodeConfig->getID();
	Lint fractional = 1000;

	uint correct = 0;
	// may not be needed
	// int ring_size = nodeNet->RING; 
	int ring_size;

	uint final_ring_size = 49;

	// uint batch_size = 2;



 	Lint *res = new Lint [100000];
	memset(res,0,sizeof(Lint)*100000);
	
	Lint *res_index = new Lint [100];
	memset(res_index,0,sizeof(Lint)*100);

	struct timeval start;
	struct timeval end;
	unsigned long timer = 0;

	printf("size : %u\n", size);


	int map[2];
	switch(pid){
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

	// double *W1_in, *W2_in, *W3_in, *W4_in, *b1_in, *b2_in, *b3_in, *b4_in, *X_in, *Y_in; 

	// convert to long long int? (for consistency)
	long long  *W1_in = new long long  [16*25];
	long long  *W2_in = new long long  [16*400];
	long long  *W3_in = new long long  [100*256];
	long long  *W4_in = new long long  [10*100];
	long long  *b1_in = new long long  [1*16];
	long long  *b2_in = new long long  [1*16];
	long long  *b3_in = new long long  [1*100];
	long long  *b4_in = new long long  [1*10];
	long long  *Y_in = new long long   [1*100000];

	long long  *X_in = new long long   [784*10000];
	// long long  **X_in = new long long *[100000];
	// for (i = 0; i < 100000; i++) {
	// 	X_in[i] = new long long [784];
	// }


	Lint **W1 = new Lint *[2];
	Lint **W2 = new Lint *[2];
	Lint **W3 = new Lint *[2];
	Lint **W4 = new Lint *[2];
	Lint **b1 = new Lint *[2];
	Lint **b2 = new Lint *[2];
	Lint **b3 = new Lint *[2];
	Lint **b4 = new Lint *[2];
	Lint **X = new Lint *[2];
	Lint **Y = new Lint *[2];

	// vars for intermediate calculations
	Lint **X0 = new Lint *[2]; // convolution
	Lint **X1 = new Lint *[2]; // matmult
	Lint **X2 = new Lint *[2]; // maxpool
	
	Lint **X3 = new Lint *[2]; // convolution
	
	Lint **X3p = new Lint *[2]; // convolution

	Lint **X4 = new Lint *[2]; // matmult
	Lint **X5 = new Lint *[2]; // maxpool
	Lint **X6 = new Lint *[2]; // matmult
	Lint **X7 = new Lint *[2]; // matmult


	for (i = 0; i < 2; i++) {
		W1[i] = new Lint [16*25];
        memset(W1[i],0,sizeof(Lint)*(16*25));
		W2[i] = new Lint [16*400];
        memset(W2[i],0,sizeof(Lint)*(16*400));
		W3[i] = new Lint [100*256];
        memset(W3[i],0,sizeof(Lint)*(100*256));
		W4[i] = new Lint [10*100];
        memset(W4[i],0,sizeof(Lint)*(10*100));
		b1[i] = new Lint [1*16];
        memset(b1[i],0,sizeof(Lint)*(1*16));
		b2[i] = new Lint [1*16];
        memset(b2[i],0,sizeof(Lint)*(1*16));
		b3[i] = new Lint [1*100];
        memset(b3[i],0,sizeof(Lint)*(1*100));
		b4[i] = new Lint [1*10];
        memset(b4[i],0,sizeof(Lint)*(1*10));
        
		X[i]  = new Lint [784*10000*batch_size];
        memset(X[i],0,sizeof(Lint)*(784*10000*batch_size));
		Y[i]  = new Lint [1*100000*batch_size];
        memset(Y[i],0,sizeof(Lint)*(1*100000*batch_size));
		

		X0[i] = new Lint [25*576*batch_size];
        memset(X0[i],0,sizeof(Lint)*(25*576*batch_size));
		X1[i] = new Lint [16*24*24*batch_size];
        memset(X1[i],0,sizeof(Lint)*(16*24*24*batch_size));
		X2[i] = new Lint [16*12*12*batch_size];
        memset(X2[i],0,sizeof(Lint)*(16*12*12*batch_size));
		X3[i] = new Lint [400*64*batch_size];
        memset(X3[i],0,sizeof(Lint)*(400*64*batch_size));
		
		X3p[i] = new Lint [400*64*batch_size];
        memset(X3p[i],0,sizeof(Lint)*(400*64*batch_size));


		X4[i] = new Lint [16*8*8*batch_size];
        memset(X4[i],0,sizeof(Lint)*(16*8*8*batch_size));
		X5[i] = new Lint [16*4*4*batch_size];
        memset(X5[i],0,sizeof(Lint)*(16*16*batch_size));
		X6[i] = new Lint [100*1*batch_size];
        memset(X6[i],0,sizeof(Lint)*(100*1*batch_size));
		X7[i] = new Lint [10*1*batch_size];
        memset(X7[i],0,sizeof(Lint)*(10*1*batch_size));

	}


    // std::string fpath = "../model/MNIST_CNN_small/W1.csv";
    std::string fpath[10] ={"../model/MNIST_CNN_small/W1.csv", 
    						"../model/MNIST_CNN_small/W2.csv", 
    						"../model/MNIST_CNN_small/W3.csv", 
    						"../model/MNIST_CNN_small/W4.csv", 
    						"../model/MNIST_CNN_small/b1.csv", 
    						"../model/MNIST_CNN_small/b2.csv", 
    						"../model/MNIST_CNN_small/b3.csv", 
    						"../model/MNIST_CNN_small/b4.csv", 
    						// "../model/MNIST_CNN_small/X.csv", 
    						"../model/MNIST_CNN_small/X_backup.csv", 
    						"../model/MNIST_CNN_small/Y.csv"};

	i = 0;
	// reading in all the data, multiplying by the fractional, 
	// and storing it in an integer (rounding down)
	// DO NOT CHANGE ORDER!!!!!!!!!!!!!!!!!!!!!!
    readFile(W1_in, 16, 25, fractional, fpath[i]); i++;
    readFile(W2_in, 16, 400, fractional, fpath[i]); i++;
    readFile(W3_in, 100, 256, fractional, fpath[i]); i++;
    readFile(W4_in, 10, 100, fractional, fpath[i]); i++;
    readB(b1_in, 1, 16, fractional, fpath[i], 1); i++;
    readB(b2_in, 1, 16, fractional, fpath[i], 2); i++;
    readB(b3_in, 1, 100, fractional, fpath[i], 3); i++;
    readB(b4_in, 1, 10, fractional, fpath[i], 4); i++;
    // printf("hi\n");

    // readFile(X_in, 784, 100, fractional, fpath[i]); i++;
    // readFile(X_in, 784, 1, fractional, fpath[i]); i++;
    readX(X_in, size, 784, fpath[i]); i++;

    readFile(Y_in, 1, 10000, 1, fpath[i]); i++;


	// ring_size = 48;

	ring_size = 20;
    splitData(W1, W1_in, 16, 25, ring_size,  nodeNet); 
    splitData(b1, b1_in, 1, 16, ring_size,  nodeNet); 

	ring_size = 30;
    splitData(W2, W2_in, 16, 400, ring_size,  nodeNet); 
    splitData(b2, b2_in, 1, 16, ring_size,  nodeNet); 

	ring_size = 40;
    splitData(W3, W3_in, 100, 256, ring_size,  nodeNet); 
    splitData(b3, b3_in, 1, 100, ring_size,  nodeNet); 

	// ring_size = 49;
    splitData(W4, W4_in, 10, 100, final_ring_size,  nodeNet); 
    splitData(b4, b4_in, 1, 10, final_ring_size,  nodeNet); 
    splitData(Y,   Y_in, 1, 10000, final_ring_size,  nodeNet); 

	for (k = 0; k < (size/batch_size); k++){


		ring_size = 20;
		// can change the 1 to whatever our batch size will be
		// trivial operation
	    splitX(X, X_in, 784, batch_size, k, ring_size,  nodeNet); 


	    // BEGIN EXECUTION
		gettimeofday(&start,NULL); // start timer here



	    // nodeNet->RING = ring_size;
	    im2col(X0, X, 1, 28, 28, 5, 1, batch_size, map, nodeNet);


	    // all take place in one operation in server.cpp

		// W2 stays constant

		Rss_MatMultArray_batch(X1, W1, X0, 16, 25, 576, ring_size, batch_size, 0, map, nodeNet); 
		// Rss_MatMultArray(X1, W1, X0, 16, 25, 576, ring_size, map, nodeNet); 
		add_biases(X1, X1, b1, 16, 576, batch_size, map, nodeNet); 

		ReLU(X1, X1, 16*576*batch_size, ring_size, map, nodeNet);	





		MaxPool(X2, X1, 16, 24, 24, batch_size, ring_size, map, nodeNet);

	
		// Rss_Open(res, X2, 16*144*batch_size, map, ring_size, nodeNet);

		// for (i = 0; i < 16*144*batch_size; i++) {
		// 	// printf("res[%i] : %llu \n", i, res[i] ); //& nodeNet->SHIFT[48]);
		// 	printf("res[%i] : %llu \n", i, res[i] ); //& nodeNet->SHIFT[48]);
		// 	// print_binary(res[i], final_ring_size);
		// }

		



		Rss_Convert(X2, X2, 16*144*batch_size, ring_size, ring_size + 10, map, nodeNet);


	    im2col(X3, X2, 16, 12, 12, 5, 1, batch_size, map, nodeNet); 
		

		ring_size = 30;
		//W3 --> 30, X3 --> 20	
		// W2 stays constant
		Rss_MatMultArray_batch(X4, W2, X3, 16, 400, 64, ring_size, batch_size, 0, map, nodeNet);




		add_biases(X4, X4, b2, 16, 64, batch_size, map, nodeNet); 
		ReLU(X4, X4, 16*64*batch_size, ring_size, map, nodeNet);	




		MaxPool(X5, X4, 16, 8, 8, batch_size, ring_size, map, nodeNet);


		Rss_Convert(X5, X5, 16*4*4*batch_size, ring_size, ring_size + 10, map, nodeNet);

		ring_size = 40;





		Rss_MatMultArray_batch(X6, W3, X5, 100, 256, 1, ring_size, batch_size,  0,map, nodeNet);
		add_biases(X6, X6, b3, 100, 1, batch_size, map, nodeNet);
		ReLU(X6, X6, 100*1*batch_size, ring_size, map, nodeNet);	






		Rss_Convert(X6, X6, 100*1*batch_size, ring_size, final_ring_size, map, nodeNet);

		// ring_size = 49;




		Rss_MatMultArray_batch(X7, W4, X6, 10, 100, 1, final_ring_size, batch_size, 0, map, nodeNet);
		// Rss_MatMultArray(X7, W4, X6, 10, 100, 1, ring_size, map, nodeNet);
		add_biases(X7, X7, b4, 10, 1, batch_size, map, nodeNet);

		gettimeofday(&end,NULL); // stop timer here
	

		Rss_Open(res, X7, 10*batch_size, map, final_ring_size, nodeNet);

		// for (i = 0; i < 10*batch_size; i++) {
		// 	// printf("res[%i] : %llu \n", i, res[i] ); //& nodeNet->SHIFT[48]);
		// 	printf("res[%i] : %llu \n", i, res[i] ); //& nodeNet->SHIFT[48]);
		// 	// print_binary(res[i], final_ring_size);
		// }






		Rss_Open(res, X7, 10*batch_size, map, final_ring_size, nodeNet);


		// printf("\n");
		// for (i = 0; i < 10*batch_size; i++) {
		// 	// printf("res[%i] : %llu \n", i, res[i] ); //& nodeNet->SHIFT[48]);
		// 	printf("res[%i] : %llu \t", i, res[i] ); //& nodeNet->SHIFT[48]);
		// 	print_binary(res[i], final_ring_size);
		// }

		arg_max(X0, X1, X7, final_ring_size, 10, batch_size, map, nodeNet);
		// maximum( X0, X7, final_ring_size, 10, batch_size, map, nodeNet);

		Rss_Open(res, X1, batch_size, map, final_ring_size, nodeNet);

		// for (i = 0; i < batch_size; i++) {
		// 	// printf("max[%i] : %llu \n", i, res[i] +1 ); //& nodeNet->SHIFT[48]);
		// 	// print_binary(res[i], final_ring_size);
		// }

	 	// for (i = 0; i < 10; i++) {
	 	// 	if () {
	 			
	 	// 	}
 		// }
 	


		for (i = 0; i < batch_size; i++) {
		 	// printf("Prediction: %u\n", res[i]+1);
		 	// printf("Actual: %u\n", Y_in[k*batch_size + i]);
			
		 	if ((res[i]+1) == Y_in[k*batch_size + i]) {
		 		correct+=1;
		 	} 
		}

		timer += 1000000 * (end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec;



	 }

	printf("total runtime for NN exeuction = %ld us\n",timer);
	printf("average prediction time = %ld us\n",(timer/batch_size)/size);
	printf("Accuracy : %f %\n", (float) correct/size * 100 );

	// cleanup
	delete [] res;
	delete [] res_index;
	// 1d arrays

	// 2d arrays
	for(i = 0; i < 2; i++){
		delete [] W1[i]; 
		delete [] W2[i]; 
		delete [] W3[i]; 
		delete [] W4[i]; 
		delete [] b1[i]; 
		delete [] b2[i]; 
		delete [] b3[i]; 
		delete [] b4[i]; 
		delete [] X[i];
		delete [] Y[i];

		delete [] X0[i];
		delete [] X1[i];
		delete [] X2[i];

		delete [] X3[i];
		delete [] X3p[i];

		delete [] X4[i];
		delete [] X5[i];
		delete [] X6[i];
		delete [] X7[i];


	}
	delete [] W1; 
	delete [] W2; 
	delete [] W3; 
	delete [] W4; 
	delete [] b1; 
	delete [] b2; 
	delete [] b3; 
	delete [] b4; 
	delete [] X;
	delete [] Y;


	delete [] X0;
	delete [] X1;
	delete [] X2;
	delete [] X3;
	delete [] X3p;

	delete [] X4;
	delete [] X5;
	delete [] X6;
	delete [] X7;

	delete [] W1_in;
	delete [] W2_in;
	delete [] W3_in;
	delete [] W4_in;
	delete [] b1_in;
	delete [] b2_in;
	delete [] b3_in;
	delete [] b4_in;
	delete [] X_in;
	delete [] Y_in;



	// for(i = 0; i < 10000; i++){
	// 	delete [] X_in[i];

	// }
	// delete [] X_in;


}

