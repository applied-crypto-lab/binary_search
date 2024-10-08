/*   
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame

   PICCO is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   PICCO is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PICCO. If not, see <http://www.gnu.org/licenses/>.
*/

#include "NodeNetwork.h"
#include <vector>
#include <cstdlib>
#include "NodeConfiguration.h"
#include <string>
#include "time.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h> 
#include <sys/time.h> 
#include <errno.h> 
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include "openssl/bio.h"

//#include <libexplain/bind.h>
NodeConfiguration *config;
EVP_CIPHER_CTX *en, *de;
int MAX_BUFFER_SIZE = 229376; // in bytes 
//int MAX_BUFFER_SIZE = 4194304; 

std::map<int, EVP_CIPHER_CTX*> peer2enlist;
std::map<int, EVP_CIPHER_CTX*> peer2delist;
std::string privatekeyfile;
unsigned char* KeyIV;
unsigned char* peerKeyIV;

/************ STATIC VARIABLES INITIALIZATION ***************/
int NodeNetwork::mode = 0; // -1 -- non-thread, 0 -- thread
int NodeNetwork::numOfChangedNodes = 0; //number of nodes that has changed modes so far
/************************************************************/


NodeNetwork::NodeNetwork(NodeConfiguration *nodeConfig, std::string privatekey_filename, int num_threads, uint ring_size, uint num_parties) {
	privatekeyfile = privatekey_filename;
	config = nodeConfig;
	connectToPeers();
	numOfThreads = num_threads; // it should be read from parsing 
	int peers = config->getPeerCount();
    	int numb = 8 * sizeof(char);
	
	prg_setup();
	numParties = num_parties;
	RING = ring_size; //setting last element to ring_size+2
	SHIFT = new Lint [ring_size+22];
	ODD = new Lint [ring_size+2];
	EVEN = new Lint [ring_size+2];

	for (Lint i = 0; i <= ring_size+21; i++) {
		SHIFT[i] = (Lint(1) << Lint(i)) - Lint(1); // mod 2^i

		// this is needed to handle "undefined behavior" of << when we want
		// to shift by more than the size of the type (in bits)
		if (i == sizeof(Lint)*8 )
		{
			SHIFT[i] =  -1;
		}
		
	}

	Lint temp = 0;
	for (Lint i = 0; i <= 8*sizeof(Lint); i++) { 
		temp = temp | Lint((i % 2 == 0));
		temp = temp << 1; 
	}
	for (Lint i = 0; i < ring_size+1; i++) { 
        EVEN[i] = (temp >> 1) & SHIFT[i];
        ODD[i] = (temp) & SHIFT[i];
    }
	

	// init_index_array();

}

NodeNetwork::NodeNetwork() {}

NodeNetwork::~NodeNetwork() 
{
	prg_clean();
	free(KeyIV);	
	free(peerKeyIV);
	EVP_CIPHER_CTX_free(en);
	EVP_CIPHER_CTX_free(de);
}


// void NodeNetwork::init_index_array(){
//     Lint r_size_2, rounds, i, j, l, y, z, k;

// 	index_array = new Lint **[2];
//     for (i = 0; i < 2; i++) {
//         index_array[i] = new Lint*[RING];
//         for (j = 0; j < RING; j++) {
//             r_size_2 = pow(2, ceil(log2(j))); // rounding up to next power of two
//             rounds = ceil(log2(r_size_2));
//             index_array[i][j] = new Lint[rounds];
// 			memset(index_array[i][j], 0, sizeof(Lint)*rounds);
//         }
//     } 
// 	// r_size <=> k
//     for (Lint k = 0; k < RING; k++) {
//         r_size_2 = pow(2, ceil(log2(k))); // rounding up to next power of two
//         rounds = ceil(log2(r_size_2));

//         for (i = 1; i <= rounds; i++) {
//             for (j = 1; j <= ceil(r_size_2 / pow(2, i)); j++) {
//                 y = uint(pow(2, i - 1) + j * pow(2, i)) % r_size_2;
//                 for (z = 1; z <= (pow(2, i - 1)); z++) {
//                     if ((((y % r_size_2)) <= k) && (((y + z) % (r_size_2 + 1)) <= k)) {

// 						index_array[0][k][i] = SET_BIT(index_array[0][k][i],(y % r_size_2) - 1 , Lint(1));
// 						index_array[1][k][i] = SET_BIT(index_array[1][k][i],((y + z) % (r_size_2 + 1)) - 1 , Lint(1));


//                     }
//                 }
//             }
//         }
// 	}
// }



void NodeNetwork::sendDataToPeer(int id, Lint* data, int start, int amount, uint size, uint flag){
	try{
        	uint read_amount = 0;
        	if(start+amount > size)
            		read_amount = size-start;
        	else
            		read_amount = amount;
		uint unit_size = (flag + 7)>>3;
		// uint unit_size = (RING[flag] + 7)>>3;
		uint buffer_size = unit_size * read_amount;
		char* buffer = (char*)malloc(sizeof(char) * buffer_size);
		char* pointer = buffer;  
		memset(buffer, 0, buffer_size);
		for(int i = start; i < start+read_amount; i++){
			//mpz_export(pointer, NULL, -1, 1, -1, 0, data[i]); 
			//printf("dtat before shfit is %lx\n", data[i]);
			data[i] = data[i] & SHIFT[flag];
			//printf("data after shfit is %lx\n", data[i]);
			memcpy(pointer, &data[i], unit_size);
			pointer += unit_size; 
		}
		
		
		EVP_CIPHER_CTX *en_temp = peer2enlist.find(id)->second;
		unsigned char* encrypted = aes_encrypt(en_temp, (unsigned char*)buffer, &buffer_size);
		sendDataToPeer(id, 1, &buffer_size); 
		sendDataToPeer(id, buffer_size, encrypted);
		free(buffer);
		free(encrypted);
	}catch(std::exception& e){
		std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
	}
}

void NodeNetwork::sendDataToPeer(int id, uint size, Lint* data, uint flag)
{
    uint count = 0, rounds = 0; 
    getRounds(size, &count, &rounds, flag); 
    for(int k = 0; k <= rounds; k++)
        sendDataToPeer(id, data, k*count, count, size, flag);
}


//*************Added for 3P table lookup



//void NodeNetwork::multicastToPeers(mpz_t** data, mpz_t** buffers, int size, int threadID){ 

void NodeNetwork::SendAndGetDataFromPeer(int sendtoID, int RecvFromID, Lint* SendData, Lint* RecvData, uint size, uint flag){
    //compute the maximum size of data that can be communicated 
    uint count = 0, rounds = 0; 
    getRounds(size, &count, &rounds, flag);
    // printf("rounds = %u\n", rounds);
     
    for(int k = 0; k <= rounds; k++)
    {
	sendDataToPeer(sendtoID, SendData, k*count, count, size, flag);
        getDataFromPeer(RecvFromID, RecvData, k*count, count, size, flag);
    }
}

void NodeNetwork::getDataFromPeer(int id, uint size, Lint* buffer, uint flag)
{
    uint count = 0, rounds = 0; 
    getRounds(size, &count, &rounds, flag); 
    memset(buffer,0,sizeof(Lint)*size);
    for(int k = 0; k <= rounds; k++)
        getDataFromPeer(id, buffer, k*count, count, size, flag);
}


void NodeNetwork::sendDataToPeer(int id, uint size, unsigned char* data){
	try{
		int on = 1; 
		unsigned char* p = data; 
		int bytes_read = sizeof(unsigned char) * size; 
		int sockfd = peer2sock.find(id)->second; 
  		fd_set fds;
		while(bytes_read > 0){
			int bytes_written = send(sockfd, p, sizeof(unsigned char) * bytes_read, MSG_DONTWAIT);
			if(bytes_written < 0){
				FD_ZERO(&fds); 
				FD_SET(sockfd, &fds);
				int n = select(sockfd+1, NULL, &fds, NULL, NULL);
				if(n > 0)
					continue; 
			} 
			else{ 
				bytes_read -= bytes_written; 
				p += bytes_written; 
			}
		}
	} catch(std::exception& e){
		std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
	}
}

void NodeNetwork::sendDataToPeer(int id, uint size, uint* data){
	try{
		uint* p = data; 
		int bytes_read = sizeof(uint) * size;
		int sockfd = peer2sock.find(id)->second;
		fd_set fds; 
		while(bytes_read > 0){
			int bytes_written = send(sockfd, p, bytes_read, MSG_DONTWAIT);
			if(bytes_written < 0){
				FD_ZERO(&fds); 
				FD_SET(sockfd, &fds); 
				int n = select(sockfd+1, NULL, &fds, NULL, NULL); 
				if(n > 0)
					continue; 
			} 
			else{
				bytes_read -= bytes_written; 
				p += (bytes_written/sizeof(uint)); 
			}
		} 
	}catch(std::exception& e){
		std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
	}
}

void NodeNetwork::getDataFromPeer(int id, uint size, uint* buffer){
	try{
		int length = 0, rbytes = 0;
		uint* tmp_buffer = (uint*)malloc(sizeof(uint) * size);
		memset(tmp_buffer, 0, sizeof(int) * size);
		fd_set fds;
		std::map<int, int>::iterator it;
		int sockfd = peer2sock.find(id)->second;
		while(length < sizeof(uint) * size){
				rbytes = recv(sockfd, tmp_buffer, sizeof(uint) * (size-length/sizeof(uint)), MSG_DONTWAIT);
				if(rbytes < 0){
					FD_ZERO(&fds); 
					FD_SET(sockfd, &fds); 
					int n = select(sockfd+1, &fds, NULL, NULL, NULL);
					if(n > 0)
						continue;  
				}
				else{
					memcpy(&buffer[length/sizeof(uint)], tmp_buffer, rbytes); 
					length += rbytes;
				}
		}
		
		free(tmp_buffer); 
	}catch(std::exception& e){
		std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
	}
}

void NodeNetwork::getDataFromPeer(int id, uint size, unsigned char* buffer){
	try{
		int length = 0, rbytes = 0;
		unsigned char*  tmp_buffer = (unsigned char*)malloc(sizeof(unsigned char) * size);
		fd_set fds;
		int sockfd = peer2sock.find(id)->second; 
		while(length < sizeof(unsigned char) * size){
			rbytes = recv(sockfd, tmp_buffer, sizeof(unsigned char) * (size-length/sizeof(unsigned char)), MSG_DONTWAIT);
			if(rbytes < 0){
				FD_ZERO(&fds); 
				FD_SET(sockfd, &fds); 
				int n = select(sockfd+1, &fds, NULL, NULL, NULL);
				if(n > 0)
					continue;  
			}
			else{
				memcpy(&buffer[length/sizeof(unsigned char)], tmp_buffer, rbytes); 
				length += rbytes;
			}
		}
		free(tmp_buffer); 
    }catch(std::exception& e){
        std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
    }
}



void NodeNetwork::getDataFromPeer(int id, Lint* data, int start, int amount, uint size, uint flag){
	try{
            	int write_amount = 0;
            	if(start+amount > size)
                	write_amount = size-start;
            	else
                	write_amount = amount;
	    	// uint unit_size = (RING[flag] + 7)>>3;
	    	uint unit_size = (flag + 7)>>3;
	    	uint length = 0;
	    	getDataFromPeer(id, 1, &length);
	    	char* buffer = (char*)malloc(sizeof(char) * length);
	    	getDataFromPeer(id, length, (unsigned char*)buffer);    	
		EVP_CIPHER_CTX *de_temp = peer2delist.find(id)->second;
		char *decrypted = (char *) aes_decrypt(de_temp, (unsigned char*)buffer, &length);
		memset(&data[start],0,sizeof(Lint)*write_amount);
		for(int i = start; i < start+write_amount; i++)
	   	{
			memcpy(&data[i],decrypted,unit_size);
			decrypted += unit_size; 
	    	}

		//free(decrypted);
	    	free(buffer);
	    }catch(std::exception& e){
			std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
	    }
}




void NodeNetwork::multicastToPeers(Lint** data, Lint** buffers, uint size, uint flag){
    int id = getID();
    int peers = config->getPeerCount();
    
    int sendIdx = 0, getIdx = 0; 
    //compute the maximum size of data that can be communicated 
    uint count = 0, rounds = 0; 
    getRounds(size, &count, &rounds, flag); 
    //memset(buffer,0,sizeof(Lint)*size);


	for(int k = 0; k <= rounds; k++){
		
		for(int j = 1; j <= peers+1; j++){
			if(id == j)
                        continue;
			sendDataToPeer(j, data[j-1], k*count, count, size, flag);
                }
                for(int j = 1; j <= peers+1; j++)
                {
			if(id == j)
                        continue;
			getDataFromPeer(j, buffers[j-1], k*count, count, size, flag);
                }
	}
}

void NodeNetwork::connectToPeers(){
	int peers = config->getPeerCount(); 
	for (int i = 1; i <= peers+1; i++)
                if(config->getID() == i)
                {
                        if(i != (peers+1))
                                requestConnection(peers+1-i);
                        if(i != 1){
			printf("asd/n");
		        acceptPeers(i-1);
			}
		}


}


/*
void NodeNetwork::sendModeToPeers(int id){
     int peers = config->getPeerCount(); 
     int msg = -2;
     for(int j = 1; j <= peers+1; j++){
        if(id == j)
          continue;
      	  sendDataToPeer(j, 1, &msg);
   }
   //sleep(1);
}
*/

void NodeNetwork::requestConnection(int numOfPeers){
		peerKeyIV = (unsigned char*)malloc(32);	//freed in destructor
		int* sockfd = (int*)malloc(sizeof(int) * numOfPeers); 
		int* portno = (int*)malloc(sizeof(int) * numOfPeers); 
		struct sockaddr_in *serv_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in) * numOfPeers); 
		struct hostent **server = (struct hostent**)malloc(sizeof(struct hostent*) * numOfPeers); 
	 	int on = 1; 		
		
		for(int i = 0; i < numOfPeers; i++){
			int ID = config->getID()+i+1; 
			portno[i] = config->getPeerPort(ID); 
			sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd[i] < 0)
				fprintf(stderr, "ERROR, opening socket\n");
			// the function below might not work in certain
			// configurations, e.g., running all nodes from the
			// same VM. it is not used for single-threaded programs
			// and thus be commented out or replaced with an
			// equivalent function otherwise.
 			//fcntl(sockfd[i], F_SETFL, O_NONBLOCK);
			int rc = setsockopt(sockfd[i], SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)); 
			rc = setsockopt(sockfd[i], IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on));
			server[i] = gethostbyname((config->getPeerIP(ID)).c_str());
			if(server[i] == NULL)
				fprintf(stderr, "ERROR, no such hosts \n");
			bzero((char*) &serv_addr[i], sizeof(serv_addr[i]));
			serv_addr[i].sin_family = AF_INET; 
			bcopy((char*)server[i]->h_addr, (char*)&serv_addr[i].sin_addr.s_addr, server[i]->h_length); 
			serv_addr[i].sin_port = htons(portno[i]); 
			
			int res, valopt; 
  			fd_set myset; 
  			struct timeval tv; 
  			socklen_t lon;
			res = connect(sockfd[i], (struct sockaddr*) &serv_addr[i], sizeof(serv_addr[i])); 
			if (res < 0) { 
     				if (errno == EINPROGRESS) {
        				tv.tv_sec = 15; 
        				tv.tv_usec = 0; 
        				FD_ZERO(&myset); 
        				FD_SET(sockfd[i], &myset); 
        				if (select(sockfd[i]+1, NULL, &myset, NULL, &tv) > 0) { 
           					lon = sizeof(int); 
           					getsockopt(sockfd[i], SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon); 
           					if (valopt) { 
							fprintf(stderr, "Error in connection() %d - %s\n", valopt, strerror(valopt)); 
              						exit(0); 
           					}
        				} 
        				else { 
           					fprintf(stderr, "Timeout or error() %d - %s\n", valopt, strerror(valopt)); 
          				 	exit(0); 
        			     	} 
     				} 
     				else { 
        				fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
        				exit(0); 
     				} 
  			}	
			printf("Connected to node %d\n", ID); 
			peer2sock.insert(std::pair<int, int>(ID, sockfd[i]));
			sock2peer.insert(std::pair<int, int>(sockfd[i], ID));

			FILE *prikeyfp = fopen(privatekeyfile.c_str(), "r");
			if( prikeyfp == NULL ) printf("File Open %s error\n", privatekeyfile.c_str());
			RSA *priRkey = PEM_read_RSAPrivateKey(prikeyfp, NULL, NULL, NULL);
			if( priRkey == NULL) printf("Read Private Key for RSA Error\n"); 
			char *buffer = (char*)malloc(RSA_size(priRkey));
    		int n = read(sockfd[i], buffer, RSA_size(priRkey));
    		if (n < 0) printf("ERROR reading from socket \n");
			char *decrypt = (char*)malloc(n);
			memset(decrypt, 0x00, n); 
			int dec_len = RSA_private_decrypt(n, (unsigned char*)buffer, (unsigned char*)decrypt, priRkey, RSA_PKCS1_OAEP_PADDING);
			if(dec_len < 1) printf("RSA private decrypt error\n");
			memcpy(peerKeyIV, decrypt, 32);
			init_keys(ID, 1);
			free(buffer);
			free(decrypt);
			fclose(prikeyfp);
		}

		//free(sockfd);
		free(portno);
		free(serv_addr);
		for(int i = 0; i < numOfPeers; i++){
			//free(server[i]);
		}
		//free(server);
}
	
void NodeNetwork::acceptPeers(int numOfPeers){
		KeyIV = (unsigned char*)malloc(32);
		int sockfd, maxsd, portno, on = 1; 
		int *newsockfd = (int*)malloc(sizeof(int) * numOfPeers); 
		socklen_t *clilen = (socklen_t*)malloc(sizeof(socklen_t) * numOfPeers); 
		struct sockaddr_in serv_addr;
		struct sockaddr_in *cli_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in) * numOfPeers);
			
		fd_set master_set, working_set; 
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		// see comment for fcntl above
		//fcntl(sockfd, F_SETFL, O_NONBLOCK);  
		if(sockfd < 0)
			fprintf(stderr, "ERROR, opening socket\n"); 
		int rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)); 
		rc = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on));  
		if(rc < 0)
			printf("setsockopt() or ioctl() failed\n"); 	
		bzero((char*) &serv_addr, sizeof(serv_addr)); 
		portno = config->getPort(); 
		serv_addr.sin_family = AF_INET; 
		serv_addr.sin_addr.s_addr = INADDR_ANY; 
		serv_addr.sin_port = htons(portno); 
		
		//printf("%d,  \n", htons(portno));

		if((bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) < 0){
			printf("ERROR, on binding \n"); 
			//printf(stderr, "%s\n",explain_bind(sockfd, (struct sockaddr*) &serv_addr,sizeof(serv_addr)));
		}
		listen(sockfd, 7); 
		// start to accept connections
		FD_ZERO(&master_set); 
		maxsd = sockfd; 
		FD_SET(sockfd, &master_set); 
		for(int i = 0; i < numOfPeers; i++){
			memcpy(&working_set, &master_set, sizeof(master_set)); 
			rc = select(maxsd+1, &working_set, NULL, NULL, NULL);  
			if(rc <= 0)
				printf("select failed or time out \n"); 
			if(FD_ISSET(sockfd, &working_set)){
				clilen[i] = sizeof(cli_addr[i]); 
				newsockfd[i] = accept(sockfd, (struct sockaddr*) &cli_addr[i], &clilen[i]); 
				if(newsockfd[i] < 0)
					fprintf(stderr, "ERROR, on accept\n");
				// see comment for fcntl above
 				//fcntl(newsockfd[i], F_SETFL, O_NONBLOCK);
				peer2sock.insert(std::pair<int, int>(config->getID() - (i+1), newsockfd[i]));
				sock2peer.insert(std::pair<int, int>(newsockfd[i], config->getID() - (i+1)));

				unsigned char key_iv[32]; 
				RAND_status();
				if( !RAND_bytes(key_iv, 32) )
					printf("Key, iv generation error\n");
				memcpy(KeyIV, key_iv, 32);
				int peer = config->getID() - (i+1);
				FILE *pubkeyfp = fopen((config->getPeerPubKey(peer)).c_str(), "r");
				if( pubkeyfp == NULL ) printf("File Open %s error \n", (config->getPeerPubKey(peer)).c_str());
				RSA *publicRkey = PEM_read_RSA_PUBKEY(pubkeyfp, NULL, NULL, NULL); 
				if( publicRkey == NULL) printf("Read Public Key for RSA Error\n"); 
				char *encrypt = (char*)malloc(RSA_size(publicRkey));
				memset(encrypt, 0x00, RSA_size(publicRkey));
				int enc_len = RSA_public_encrypt(32, KeyIV, (unsigned char*)encrypt, publicRkey, RSA_PKCS1_OAEP_PADDING);
				if(enc_len < 1) printf("RSA public encrypt error\n");
				int n = write(newsockfd[i], encrypt, enc_len);
     			if (n < 0) printf("ERROR writing to socket \n");
				init_keys(peer, 0);
				free(encrypt);
				fclose(pubkeyfp);
			}
		}
}


void NodeNetwork::init_keys(int peer, int nRead){
	unsigned char key[16],iv[16];
	memset(key, 0x00, 16);
	memset(iv, 0x00, 16);
	if(0 == nRead) //useKey KeyIV
	{		
		memcpy(key, KeyIV, 16);
		memcpy(iv, KeyIV+16, 16);
	}
	else //getKey from peers
	{
		memcpy(key, peerKeyIV, 16);
		memcpy(iv, peerKeyIV+16, 16);
	}
    en = EVP_CIPHER_CTX_new();

    // EVP_CIPHER_CTX_init(en);
	EVP_EncryptInit_ex(en,EVP_aes_128_cbc(), NULL, key, iv);
    de = EVP_CIPHER_CTX_new();
	// EVP_CIPHER_CTX_init(de);
	EVP_DecryptInit_ex(de,EVP_aes_128_cbc(), NULL, key, iv);
	peer2enlist.insert(std::pair<int, EVP_CIPHER_CTX*>(peer, en));
	peer2delist.insert(std::pair<int, EVP_CIPHER_CTX*>(peer, de));
}


void NodeNetwork::getRounds(uint size, uint *count, uint *rounds, uint flag)
{

    int peers = config->getPeerCount();
    uint unit_size = (flag + 7)>>3;
    // uint unit_size = (RING[flag] + 7)>>3;
    *count = MAX_BUFFER_SIZE/(peers+1)/unit_size;
    if(size % (*count) != 0)
    	*rounds = size/(*count);	
    else
	*rounds = size/(*count)-1; 
}

double NodeNetwork::time_diff(struct timeval *t1, struct timeval *t2){
	double elapsed;
	if(t1->tv_usec > t2->tv_usec){
		t2->tv_usec += 1000000;
		t2->tv_sec--;
	}

	elapsed = (t2->tv_sec-t1->tv_sec) + (t2->tv_usec - t1->tv_usec)/1000000.0;

	return(elapsed);
}

int NodeNetwork::getID(){
	return config->getID();
}

int NodeNetwork::getNumParties(){
	return numParties; 
}

int NodeNetwork::getNumOfThreads(){
	return numOfThreads; 
}
unsigned char *NodeNetwork::aes_encrypt(EVP_CIPHER_CTX* e, unsigned char *plaintext, uint *len){
	int c_len = *len + AES_BLOCK_SIZE;
	int f_len = 0;
	unsigned char *ciphertext = (unsigned char*) malloc(c_len);
	
	EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

	EVP_EncryptUpdate(e,ciphertext, &c_len, plaintext, *len);

	EVP_EncryptFinal_ex(e, ciphertext+c_len, &f_len);

	*len = c_len + f_len;

	return ciphertext;
}

unsigned char *NodeNetwork::aes_decrypt(EVP_CIPHER_CTX* e, unsigned char *ciphertext, uint *len){
	int p_len = *len;
	int f_len = 0;
	unsigned char *plaintext = (unsigned char*)malloc(p_len + AES_BLOCK_SIZE);

	EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
	EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
	EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);
	*len = p_len  + f_len;
	return plaintext;
}

void NodeNetwork::closeAllConnections(){
}

void NodeNetwork::prgtest(){
	printf("prgtest\n");
}

/*
//getringsize:
//	inital ringsize flag = 1
//	ringsize - 1    flag = 0
//	ringsize + 2    flag = 2
int NodeNetwork::getringsize(int flag){
	return RING[flag];
}

//getshift:
//	inital ringsize shift flag = 1
//	ringsize - 1 shift    flag = 0
//	ringsize + 2 shfit    flag = 2
Lint NodeNetwork::getShfit(int flag){
	return SHIFT[flag];
}
*/

__m128i * NodeNetwork::prg_keyschedule(uint8_t * src) {
	__m128i * r = (__m128i*)malloc(11*sizeof(__m128i));

    r[0] = _mm_load_si128((__m128i *) src);

	KE2(r[1], r[0], 0x01)
	KE2(r[2], r[1], 0x02)
	KE2(r[3], r[2], 0x04)
	KE2(r[4], r[3], 0x08)
	KE2(r[5], r[4], 0x10)
	KE2(r[6], r[5], 0x20)
	KE2(r[7], r[6], 0x40)
	KE2(r[8], r[7], 0x80)
	KE2(r[9], r[8], 0x1b)
	KE2(r[10], r[9], 0x36)

	return r;
}


void NodeNetwork::prg_aes(uint8_t * dest, uint8_t * src, __m128i * ri) {
	__m128i rr, mr;
	__m128i * r = ri;

    rr = _mm_loadu_si128((__m128i *) src);
    mr = rr;

    mr = _mm_xor_si128(mr, r[0]);

    mr = _mm_aesenc_si128(mr, r[1]);
    mr = _mm_aesenc_si128(mr, r[2]);
    mr = _mm_aesenc_si128(mr, r[3]);
    mr = _mm_aesenc_si128(mr, r[4]);
    mr = _mm_aesenc_si128(mr, r[5]);
    mr = _mm_aesenc_si128(mr, r[6]);
    mr = _mm_aesenc_si128(mr, r[7]);
    mr = _mm_aesenc_si128(mr, r[8]);
    mr = _mm_aesenc_si128(mr, r[9]);
    mr = _mm_aesenclast_si128(mr, r[10]);
    mr = _mm_xor_si128(mr, rr);
    _mm_storeu_si128((__m128i*) dest, mr);

}


void NodeNetwork::prg_setup(){
	random_container = new uint8_t *[3];
	for(int i = 0; i< 3; i++){
		random_container[i] = new uint8_t [16];
		memset(random_container[i],0,sizeof(uint8_t)*16);
	}

	prg_key = new __m128i *[3];

	uint8_t tempKey_A[16];
	uint8_t tempKey_B[16];
	uint8_t tempKey_C[16];


	uint8_t RandomData[64];
	FILE *fp = fopen("/dev/urandom", "r");
	// reading 48 bits to generate 2 keys
	// last 16 are to seed private key 
	// increaseing to 48 so each party has a private key
	// that will be used for offline prg_aes_ni (non-interactive)
	if (fread(RandomData, 1, 64, fp) != 64) {
		fprintf(stderr,"Could not read random bytes.");
		exit(1);
	}
	fclose(fp);

	memcpy(random_container[0], RandomData, 16);
	memcpy(tempKey_A, RandomData+16, 16);
	memcpy(tempKey_C, RandomData+32, 16);
	memcpy(random_container[2], RandomData+48, 16);

	int pid = getID();
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

	sendDataToPeer(map[0], 32, RandomData);
	getDataFromPeer(map[1], 32, RandomData);



	memcpy(random_container[1], RandomData, 16);
	memcpy(tempKey_B, RandomData+16, 16);


	prg_key[0] = prg_keyschedule(tempKey_A);
	prg_key[1] = prg_keyschedule(tempKey_B);
	prg_key[2] = prg_keyschedule(tempKey_C);

	uint8_t res[16] = {};
	prg_aes(res, random_container[0], prg_key[0]);
	memcpy(random_container[0], res, 16);

	prg_aes(res, random_container[1], prg_key[1]);
	memcpy(random_container[1], res, 16);
	
	prg_aes(res, random_container[2], prg_key[2]);
	memcpy(random_container[2], res, 16);
	

	P_container = new int [3];
	P_container[0] = 0;
	P_container[1] = 0;
	P_container[2] = 0;
	container_size = 16;
	printf("prg setup\n");
}

//This func no only cleans prg stuff, it will also take care of other things
void NodeNetwork::prg_clean(){
	for(int i = 0; i<2;i++){
		delete [] random_container[i];	
		free(prg_key[i]);

		// for (int j = 0; j < RING; j++) {
        //     delete[] index_array[i][j];
        // }

        // delete[] index_array[i];
    }

    // delete[] index_array;
    delete [] random_container;
	delete [] prg_key;
	delete [] P_container;

	// delete [] RING;
	delete [] SHIFT;
	delete [] ODD;
	delete [] EVEN;
	printf("prg cleanup\n");
}


// void NodeNetwork::prg_getrandom_original(int keyID, int size, Lint* dest){		
// //this size means how many random bytes we need 
// 	uint8_t *buffer = new uint8_t [size];
// 	//printf("curent P is %d \n",P_container[keyID]);
// 	if(size <= container_size - P_container[keyID]){
// 		memcpy(dest, random_container[keyID]+P_container[keyID], size);
// 		P_container[keyID] = P_container[keyID] + size;
// 	}
// 	else{
// 		//printf("run aes\n");
// 		memcpy(buffer, random_container[keyID] + P_container[keyID], container_size - P_container[keyID]);
// 		uint8_t res[16] = {};
// 		prg_aes(res, random_container[keyID], prg_key[keyID]);
// 		memcpy(random_container[keyID], res, 16);
// 		memcpy(buffer + container_size - P_container[keyID], random_container[keyID], size - container_size + P_container[keyID]);
// 		memcpy(dest,buffer,size);

// 		P_container[keyID] = size - container_size + P_container[keyID];
// 	}

// 	delete [] buffer;
// }

void NodeNetwork::prg_getrandom(int keyID, uint size, uint length, uint8_t* dest){	
// we assume container_size is 16, so all *container_size are replaced as <<4
//this size means how many random bytes we need 
	//uint8_t *buffer = new uint8_t [size];
	//printf("curent P is %d \n",P_container[keyID]);
	uint rounds = ((size * length - container_size + P_container[keyID]) + 15) >> 4;
	if (rounds == 0){
		memcpy(dest, random_container[keyID] + P_container[keyID], size * length);
		P_container[keyID] = P_container[keyID] + size * length;
	}
	else{
		memcpy(dest, random_container[keyID] + P_container[keyID], container_size - P_container[keyID]);
		if(rounds >= 2){
			prg_aes(dest + (container_size - P_container[keyID]), random_container[keyID], prg_key[keyID]);
			for(int i = 1; i < rounds - 1; i++){
				prg_aes(dest + (container_size - P_container[keyID]) + (i<<4), dest + (container_size - P_container[keyID]) + ((i-1)<<4), prg_key[keyID]);
			}
			prg_aes(random_container[keyID], dest + (container_size - P_container[keyID]) + ((rounds-2)<<4), prg_key[keyID]);
			memcpy(dest + container_size - P_container[keyID] +  ((rounds-1)<<4), random_container[keyID], size*length - ((rounds-1)<<4) - container_size + P_container[keyID]);
			P_container[keyID] = size*length - ((rounds-1)<<4) - container_size + P_container[keyID];
		}
		else{
			prg_aes(random_container[keyID], random_container[keyID], prg_key[keyID]);
			memcpy(dest + container_size - P_container[keyID], random_container[keyID], size*length - container_size + P_container[keyID]);
			P_container[keyID] = size*length - container_size + P_container[keyID];
		}
	}

	//delete [] buffer;
}



// used for offline random generation
// defaults to using party's private key
void NodeNetwork::prg_getrandom(uint size, uint length, uint8_t* dest){	
// we assume container_size is 16, so all *container_size are replaced as <<4
//this size means how many random bytes we need 
// when this functions is called we use the party's private key stored in [2]
	uint keyID = 2;
	uint rounds = ((size * length - container_size + P_container[keyID]) + 15) >> 4;
	if (rounds == 0){
		memcpy(dest, random_container[keyID] + P_container[keyID], size * length);
		P_container[keyID] = P_container[keyID] + size * length;
	}
	else{
		memcpy(dest, random_container[keyID] + P_container[keyID], container_size - P_container[keyID]);
		if(rounds >= 2){
			prg_aes(dest + (container_size - P_container[keyID]), random_container[keyID], prg_key[keyID]);
			for(int i = 1; i < rounds - 1; i++){
				prg_aes(dest + (container_size - P_container[keyID]) + (i<<4), dest + (container_size - P_container[keyID]) + ((i-1)<<4), prg_key[keyID]);
			}
			prg_aes(random_container[keyID], dest + (container_size - P_container[keyID]) + ((rounds-2)<<4), prg_key[keyID]);
			memcpy(dest + container_size - P_container[keyID] +  ((rounds-1)<<4), random_container[keyID], size*length - ((rounds-1)<<4) - container_size + P_container[keyID]);
			P_container[keyID] = size*length - ((rounds-1)<<4) - container_size + P_container[keyID];
		}
		else{
			prg_aes(random_container[keyID], random_container[keyID], prg_key[keyID]);
			memcpy(dest + container_size - P_container[keyID], random_container[keyID], size*length - container_size + P_container[keyID]);
			P_container[keyID] = size*length - container_size + P_container[keyID];
		}
	}

	//delete [] buffer;
}



/*
void NodeNetwork::prg_getrandom(int keyID, int size, Lint* dest){		
//this size means how many random bytes we need 
	if(size <= container_size - P_container[keyID]){
		memcpy(dest, random_container[keyID]+P_container[keyID], size);
		P_container[keyID] = P_container[keyID] + size;
	}
	else{
		memcpy(dest, random_container[keyID] + P_container[keyID], container_size - P_container[keyID]);
		prg_aes(random_container[keyID], random_container[keyID], prg_key[keyID]);
		memcpy(dest + container_size - P_container[keyID], random_container[keyID], size - container_size + P_container[keyID]);
		P_container[keyID] = size - container_size + P_container[keyID];
	}

	//delete [] buffer;
}
*/

/*
void NodeNetwork::prg_Prandom(void* dest, int keyID, uint cell_size, uint random_size, uint cell_number)
//cell_size: size of each dest element; random_size: size of randomness for each cell; random_number: number of cells in dest
{
	
}
*/
//blow funcs are added for bit operation
void NodeNetwork::SendAndGetDataFromPeer_bit(int sendtoID, int RecvFromID, uint8_t* SendData, uint8_t* RecvData, uint size){ //sizes means number of bytes
    //compute the maximum size of data that can be communicated 
    uint count = 0, rounds = 0; 
    getRounds_bit(size, &count, &rounds); 
    // printf("rounds = %u\n", rounds);
    
    for(int k = 0; k <= rounds; k++)
    {
	sendDataToPeer_bit(sendtoID, SendData, k*count, count, size);
        getDataFromPeer_bit(RecvFromID, RecvData, k*count, count, size);
    }
}



void NodeNetwork::sendDataToPeer_bit(int id, uint8_t* data, int start, uint amount, uint size){
	try{
        	int read_amount = 0;
        	if(start+amount > size)
            		read_amount = size-start;
        	else
            		read_amount = amount;
		int unit_size = 1;
		uint buffer_size = unit_size * read_amount;
		char* buffer = (char*)malloc(sizeof(char) * buffer_size);
		char* pointer = buffer;  
		memset(buffer, 0, buffer_size);
		memcpy(pointer, &data[start], unit_size*read_amount);
		
		EVP_CIPHER_CTX *en_temp = peer2enlist.find(id)->second;
		unsigned char* encrypted = aes_encrypt(en_temp, (unsigned char*)buffer, &buffer_size);
		sendDataToPeer(id, 1, &buffer_size); 
		sendDataToPeer(id, buffer_size, encrypted);
		free(buffer);
		free(encrypted);
	}catch(std::exception& e){
		std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
	}
}

void NodeNetwork::getDataFromPeer_bit(int id, uint8_t* data, int start, int amount, uint size){
	try{
            	int write_amount = 0;
            	if(start+amount > size)
                	write_amount = size-start;
            	else
                	write_amount = amount;
	    	int unit_size = 1;
	    	uint length = 0;
	    	getDataFromPeer(id, 1, &length);
	    	char* buffer = (char*)malloc(sizeof(char) * length);
	    	getDataFromPeer(id, length, (unsigned char*)buffer);    	
		EVP_CIPHER_CTX *de_temp = peer2delist.find(id)->second;
		char *decrypted = (char *) aes_decrypt(de_temp, (unsigned char*)buffer, &length);
		memset(&data[start],0,sizeof(uint8_t)*write_amount);
		memcpy(&data[start],decrypted,unit_size*write_amount);
		//free(decrypted);
	    	free(buffer);
		//free(tmp);
	    }catch(std::exception& e){
			std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
	    }
}


void NodeNetwork::getRounds_bit(uint size, uint *count, uint *rounds)	//size means number of bytes
{

    int peers = config->getPeerCount();
    int unit_size = 1; 
    *count = MAX_BUFFER_SIZE/(peers+1)/unit_size;
    if(size % (*count) != 0)
    	*rounds = size/(*count);	
    else
	*rounds = size/(*count)-1; 
}

