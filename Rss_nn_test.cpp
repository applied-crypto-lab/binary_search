/* File generated from [./Mul_test/mult.c] by PICCO
 */

#include <limits.h>
#include <float.h>

extern  "C" int   ort_initialize(int*, char***);
extern "C" void  ort_finalize(int);
#include "connection/Headers.h"
// #include "connection/NodeConfiguration.h"
// #include "connection/NodeNetwork.h"
#include "RSS_NN/Program.h"
#include "RSS_NN/Neural.h"

NodeConfiguration* nodeConfig;
NodeNetwork* nNet;

int  __original_main(int _argc_ignored, char **_argv_ignored){
	//int size = pow(2, atoi(_argv_ignored[4])); //intput power of 2
	int size = atoi(_argv_ignored[4]);   // input size directly
	int batch_size = atoi(_argv_ignored[6]);   // input size directly
	// nNet->setNumParties(3);
	test(nNet, nodeConfig, size, batch_size);
	// neural_main(nNet, nodeConfig, size, batch_size);
	return 0;
}

/* smc-compiler generated main() */
int main(int argc, char **argv){
	if(argc < 7){
	fprintf(stderr,"Incorrect input parameters\n");
	fprintf(stderr,"Usage: <id> <runtime-config> <privatekey-filename> <size of data(2^x)> <ring size(x bits)> <batch_size>\n");
	exit(1);
	}
	nodeConfig = new NodeConfiguration(atoi(argv[1]), argv[2], 128);
	std::cout << "Creating the NodeNetwork\n";
	nNet = new NodeNetwork(nodeConfig, argv[3], 1, atoi(argv[5]), 3);

	struct timeval start;
	struct timeval end;
	unsigned long timer;
	int _xval = 0;
	gettimeofday(&start,NULL); //start timer here
  	_xval = (int) __original_main(argc, argv);
	gettimeofday(&end,NULL);//stop timer here

	delete nodeConfig;
	delete nNet;
	timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("Total runtime = %ld us\n",timer);
	return 0;
	// return (_xval);
}

