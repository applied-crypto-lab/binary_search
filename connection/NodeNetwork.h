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

#ifndef NODENETWORK_H_
#define NODENETWORK_H_
#include "NodeConfiguration.h"
#include "cmath"
#include <cstdlib>
#include <cstring>
#include <map>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <vector>

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>     //for int8_t
#include <string.h>     //for memcmp
#include <tmmintrin.h>
#include <x86intrin.h>
#include <wmmintrin.h>  //for intrinsics for AES-NI
#include <inttypes.h>

#define GET_BIT(X, N) ( ( (X) >> (N) ) & Lint(1) ) //doesn't need to be Lint for AND
#define RST_BIT(X, N) ( (X) & ~(Lint(1) << (N) ) ) 
#define SET_BIT(X, N, B) (X & ~(Lint(1) << N)) | (B << N)

#define KE2(NK,OK,RND) NK = OK; \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4));  \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4));  \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4));  \
    NK = _mm_xor_si128(NK, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(OK, RND), 0xff));


class NodeNetwork {
public:
	NodeNetwork(NodeConfiguration *nodeConfig, std::string privatekey_filename, int num_threads, uint ring_size, uint num_parties);
	NodeNetwork();
	virtual ~NodeNetwork();

	//Send round data to a specific peer
	void sendDataToPeer(int, Lint*, int, int, uint, uint);
	void sendDataToPeer(int, uint, uint*);
	void sendDataToPeer(int, uint, unsigned char*);
	void sendDataToPeer(int, uint, Lint*, uint); 
	//void sendDataToPeer(int, int, long long*); 
	//void sendModeToPeers(int); 
	//Get round data from a specific peer
	void getDataFromPeer(int, Lint*, int, int, uint, uint);
	void getDataFromPeer(int, uint, uint*);
	void getDataFromPeer(int, uint, unsigned char*);
	void getDataFromPeer(int, uint, Lint*, uint);
	//void getDataFromPeer(int, int, long long*);

	void SendAndGetDataFromPeer(int, int, Lint*, Lint*, uint, uint);

	void multicastToPeers(Lint** , Lint** , uint, uint);

	void SendAndGetDataFromPeer_bit(int, int, uint8_t*, uint8_t*, uint);
	void sendDataToPeer_bit(int, uint8_t*, int, uint, uint);
	void getDataFromPeer_bit(int, uint8_t*, int, int, uint);
    void getRounds_bit(uint, uint *, uint *);

    void init_index_array();

    //Broadcast identical data to peers
	//	void broadcastToPeers(mpz_t*, int, mpz_t**);
	//	void broadcastToPeers(long long*, int, long long**); 
	//	void multicastToPeers(mpz_t**, mpz_t**, int);
//	void multicastToPeers(long long**, long long**, int); 

	//Get the ID of the compute Node
	int getID();
	int getNumOfThreads(); 
	int getNumParties(); 
	void getRounds(uint, uint*, uint*, uint); 
	//void handle_write(const boost::system::error_code& error);
	//Close all the connections to peers
	void closeAllConnections();

	//Encryption and Decryption
	void init_keys(int peer, int nRead);
	void gen_keyiv();
	void get_keyiv(char* key_iv);
	unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, uint *len);
	unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, uint *len);
	
	//Close
	//void mpzFromString(char*, mpz_t*, int*, int);
	double time_diff(struct timeval *, struct timeval *);


	//PRG
	void prgtest();
	__m128i * prg_keyschedule(uint8_t *);
	void prg_aes(uint8_t *, uint8_t *, __m128i *);
	void prg_setup();
	void prg_clean();
	void prg_getrandom_original(int keyID, int size, Lint* dest);
	void prg_getrandom(int keyID, uint size, uint length, uint8_t* dest);
	void prg_getrandom(uint size, uint length, uint8_t* dest);

	Lint *SHIFT;
	Lint *ODD;
	Lint *EVEN;
	uint RING;

    // Lint ***index_array;

private:

	static int mode; 
	static int numOfChangedNodes; 
	static int isManagerAwake; 
 
	int numOfThreads; 
	void connectToPeers();
	void requestConnection(int); 
	void acceptPeers(int); 
	std::map<int, int> peer2sock;  
	std::map<int, int> sock2peer; 
	int serverSock;

	int numParties;

	uint8_t **random_container;
	int container_size;
	int *P_container;
	__m128i ** prg_key;
	


};

#endif /* NODENETWORK_H_ */
