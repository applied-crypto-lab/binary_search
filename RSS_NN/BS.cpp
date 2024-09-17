#include "BS.h"

void Modifiedbs5(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint blockSize, uint ring_size, int* map, Lint** array, Lint* tar, Lint** res, unsigned long* timer, unsigned long* Commtimer)
{
    struct timeval start;
    struct timeval end;
    int pid = nodeConfig->getID();
    
    int BSSize = size/blockSize;
    Lint one[2]; //one: 0, 0, 1
    switch(pid){
    case 1:
      one[0] = 0;
      one[1] = 0;
      break;
    case 2:
      one[0] = 0;
      one[1] = 1;
      break;
    case 3:
      one[0] = 1;
      one[1] = 0;
      break;
    }
    Lint **tarTmp= new Lint *[2];
    tarTmp[0] = new Lint [1];
    tarTmp[1] = new Lint [1];
    tarTmp[0][0] = tar[0];
    tarTmp[1][0] = tar[1];
    
    Lint **c;
    c = new Lint *[2];
    
    c[0] = new Lint [1];
    memset(c[0],0,sizeof(Lint)*1);
    c[1] = new Lint [1];
    memset(c[1],0,sizeof(Lint)*1);

    Lint **dotRes;
    dotRes = new Lint *[2];

    dotRes[0] = new Lint [1];
    memset(dotRes[0],0,sizeof(Lint)*1);
    dotRes[1] = new Lint [1];
    memset(dotRes[1],0,sizeof(Lint)*1);    

    int layer = 0;
    int rest = BSSize;
    while(rest != 1){
      layer++;
      rest /=2;
    }
    Lint ***L = new Lint **[2];
    L[0] = new Lint *[layer];
    L[1] = new Lint *[layer];
    for(int i = 0; i < layer; i++){
      L[0][i] = new Lint [BSSize/2];
      L[1][i] = new Lint [BSSize/2];
    }
    Lint **Q = new Lint *[2];
    Q[0] = new Lint [BSSize];
    Q[1] = new Lint [BSSize];
    
    Lint **Mult_tmp1 = new Lint *[2];
    Mult_tmp1[0] = new Lint [BSSize];
    Mult_tmp1[1] = new Lint [BSSize];
    
    Lint **Mult_tmp2 = new Lint *[2];
    Mult_tmp2[0] = new Lint [BSSize];
    Mult_tmp2[1] = new Lint [BSSize];
    
    Lint **arrayTmp;
    arrayTmp = new Lint *[2];
    arrayTmp[0] = new Lint [BSSize];
    arrayTmp[1] = new Lint [BSSize];
    for(int i = 0; i < BSSize; ++i){
      arrayTmp[0][i] = array[0][(i+1)*blockSize - 1];
      arrayTmp[1][i] = array[1][(i+1)*blockSize - 1];
    }
    
    int layer_size = 1;
    int layer_offset = 2;
    
    for(int i = 0; i < layer; i++){
      for(int j = 0; j < layer_size; j++){
	L[0][i][j] = arrayTmp[0][((2*j+1)*BSSize)/layer_offset - 1];
	L[1][i][j] = arrayTmp[1][((2*j+1)*BSSize)/layer_offset - 1];
      }
      layer_size *= 2;
      layer_offset *=2;
    }
    Q[0][0] = one[0];
    Q[1][0] = one[1];
    layer_size = 1;
    Lint* dot_tmp[2];
    Lint* Localres = new Lint [2];
    Localres[0] = 0;
    Localres[1] = 0;

    for(int i = 0; i < layer; i++){
      //printf("layersize %d", layer_size);
      dot_tmp[0] = L[0][i];
      dot_tmp[1] = L[1][i];
      Rss_Dot(Localres, Q, dot_tmp, layer_size, ring_size, map, nodeNet);
      dotRes[0][0] = Localres[0];
      dotRes[1][0] = Localres[1];
      /*Debug
      Lint* dtmp = new Lint[layer_size];
      Rss_Open(dtmp, dotRes, 1, map, ring_size, nodeNet);
      for(int i = 0; i < 1; ++i){
	printf("c[%d] is %lX \n", i, dtmp[i]);
      }
      delete [] dtmp;
      */
      
      Rss_JustLT(c, dotRes, tarTmp, 1, ring_size, map, nodeNet);
      c[0][0] = one[0] - c[0][0];
      c[1][0] = one[1] - c[1][0];
      
      for(int j = 0; j < 2*layer_size; j++){
	Mult_tmp1[0][j] = Q[0][j/2];
	Mult_tmp1[1][j] = Q[1][j/2];
	if(j%2 == 0){
	  Mult_tmp2[0][j] = c[0][0];
	  Mult_tmp2[1][j] = c[1][0];
	}else{
	  Mult_tmp2[0][j] = one[0] - c[0][0];
	  Mult_tmp2[1][j] = one[1] - c[1][0];
	}
      }
      Rss_Mult(Q, Mult_tmp1, Mult_tmp2, 2*layer_size, ring_size, map, nodeNet);
      layer_size *= 2;
    }
    gettimeofday(&start,NULL);//stop timer here
    Rss_DotArray(res, array, Q, size, blockSize, ring_size, map, nodeNet, Commtimer);
    gettimeofday(&end,NULL);//stop timer here
    *timer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    
    
    for(int i = 0; i < layer; i++){
      delete [] L[0][i];
      delete [] L[1][i];
    }
    delete [] L[0];
    delete [] L[1];
    delete [] L;
    delete [] Mult_tmp1[0];
    delete [] Mult_tmp1[1];
    delete [] Mult_tmp1;
    delete [] Mult_tmp2[0];
    delete [] Mult_tmp2[1];
    delete [] Mult_tmp2;
    delete [] Q[0];
    delete [] Q[1];
    delete [] Q;
    delete [] arrayTmp[0];
    delete [] arrayTmp[1];
    delete [] arrayTmp;
    delete [] Localres;
    delete [] tarTmp[0];
    delete [] tarTmp[1];
    delete [] tarTmp;
    delete [] c[0];
    delete [] c[1];
    delete [] c;
    delete [] dotRes[0];
    delete [] dotRes[1];
    delete [] dotRes;
}


void Modifiedbs1(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint blockSize, uint ring_size, int* map, Lint** array, Lint* tar, Lint** res, unsigned long* timer, unsigned long* Commtimer){
    int i, j, k;
    struct timeval start;
    struct timeval end;
    Lint **tarTmp= new Lint *[2];
    tarTmp[0] = new Lint [size/blockSize];
    tarTmp[1] = new Lint [size/blockSize];
    for(int i = 0; i < size/blockSize; i++){
      tarTmp[0][i] = tar[0];
      tarTmp[1][i] = tar[1];
    }
    
    int pid = nodeConfig->getID();
    Lint one[2]; //one: 0, 0, 1                                                                                
    switch(pid){
    case 1:
      one[0] = 0;
      one[1] = 0;
      break;
    case 2:
      one[0] = 0;
      one[1] = 1;
      break;
    case 3:
      one[0] = 1;
      one[1] = 0;
      break;
    }
    
    Lint **c;
    Lint **d;
    Lint **tmp;
    c = new Lint *[2];
    d = new Lint *[2];
    tmp = new Lint *[2];
    
    c[0] = new Lint [size/blockSize];
    memset(c[0],0,sizeof(Lint)*size/blockSize);
    c[1] = new Lint [size/blockSize];
    memset(c[1],0,sizeof(Lint)*size/blockSize);
    d[0] = new Lint [size/blockSize];
    memset(d[0],0,sizeof(Lint)*size/blockSize);
    d[1] = new Lint [size/blockSize];
    memset(d[1],0,sizeof(Lint)*size/blockSize);
    
    Lint **arrayTmp;
    arrayTmp = new Lint *[2];
    arrayTmp[0] = new Lint [size/blockSize];
    arrayTmp[1] = new Lint [size/blockSize];
    for(int i = 0; i < size/blockSize; ++i){
      arrayTmp[0][i] = array[0][(i+1)*blockSize - 1];
      arrayTmp[1][i] = array[1][(i+1)*blockSize - 1];
    }

    Rss_JustLT(c, arrayTmp, tarTmp, size/blockSize, ring_size, map, nodeNet);

    for(int i = 0; i < size/blockSize; ++i){
      c[0][i] = one[0] - c[0][i];
      c[1][i] = one[1] - c[1][i];
    }

    for(int i = 1; i < size/blockSize-1; i++){
      tarTmp[0][i] = one[0] - c[0][i-1];
      tarTmp[1][i] = one[1] - c[1][i-1];
    }
    Rss_Mult(d, c, tarTmp, size/blockSize-1, ring_size, map, nodeNet);

    d[0][0] = c[0][0];
    d[1][0] = c[1][0];
    
    d[0][size/blockSize-1] = one[0] - c[0][size/blockSize-2];
    d[1][size/blockSize-1] = one[1] - c[1][size/blockSize-2];   
    
    gettimeofday(&start,NULL);//stop timer here
    Rss_DotArray(res, array, d, size, blockSize, ring_size, map, nodeNet, Commtimer);
    gettimeofday(&end,NULL);//stop timer here                                                                  
    *timer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    /*
    //Debug
      Lint* dtmp = new Lint[blockSize];
      Rss_Open(dtmp, res, blockSize, map, ring_size, nodeNet);
      for(int i = 0; i < blockSize; ++i){
      printf("c[%d] is %lX \n", i, dtmp[i]);
      }
    */


    delete [] tarTmp[0];
    delete [] tarTmp[1];
    delete [] tarTmp;
    delete [] arrayTmp[0];
    delete [] arrayTmp[1];
    delete [] arrayTmp;
    
    delete [] c[0];
    delete [] c[1];
    delete [] c;
    delete [] d[0];
    delete [] d[1];
    delete [] d;
}



void bs15(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint* tar, Lint* res){
    struct timeval start;
    struct timeval end;
    unsigned long timer;
    struct timeval start1;
    struct timeval end1;
    unsigned long timer1 = 0;
    
    int pid = nodeConfig->getID();
    Lint one[2]; //one: 0, 0, 1
    switch(pid){
    case 1:
      one[0] = 0;
      one[1] = 0;
      break;
    case 2:
      one[0] = 0;
      one[1] = 1;
      break;
    case 3:
      one[0] = 1;
      one[1] = 0;
      break;
    }
    Lint **tarTmp= new Lint *[2];
    tarTmp[0] = new Lint [1];
    tarTmp[1] = new Lint [1];
    tarTmp[0][0] = tar[0];
    tarTmp[1][0] = tar[1];
    
    Lint **c;
    Lint **p;
    Lint **p_tmp;
    c = new Lint *[2];
    p = new Lint *[2];
    p_tmp = new Lint *[2];
    
    c[0] = new Lint [1];
    memset(c[0],0,sizeof(Lint)*1);
    c[1] = new Lint [1];
    memset(c[1],0,sizeof(Lint)*1);
    p[0] = new Lint [1];
    memset(p[0],array[0][size-1],sizeof(Lint)*1);
    p[1] = new Lint [1];
    memset(p[1],array[1][size-1],sizeof(Lint)*1);
    p_tmp[0] = new Lint [1];
    memset(p_tmp[0],0,sizeof(Lint)*1);
    p_tmp[1] = new Lint [1];
    memset(p_tmp[1],0,sizeof(Lint)*1);
    
    Lint* dot_tmp[2];
    
    int layer = 0;
    int rest = size/2;
    while(rest != 1){
      layer++;
      rest /=2;
    }

    Lint ***L = new Lint **[2];
    L[0] = new Lint *[layer];
    L[1] = new Lint *[layer];
    for(int i = 0; i < layer; i++){
      L[0][i] = new Lint [size/2];
      L[1][i] = new Lint [size/2];
      //  memset(L[0][i],0,sizeof(Lint)*size/2);
      //  memset(L[1][i],0,sizeof(Lint)*size/2);
    }

    Lint **Q = new Lint *[2];
    Q[0] = new Lint [size/2+1];
    Q[1] = new Lint [size/2+1];
    //memset(Q[0],0,sizeof(Lint)*(size/2+1));
    //memset(Q[1],0,sizeof(Lint)*(size/2+1));
    Lint **Mult_tmp1 = new Lint *[2];
    Mult_tmp1[0] = new Lint [size/2+1];
    Mult_tmp1[1] = new Lint [size/2+1];
    //memset(Mult_tmp1[0],0,sizeof(Lint)*(size/2 + 1));
    //memset(Mult_tmp1[1],0,sizeof(Lint)*(size/2 + 1));
    Lint **Mult_tmp2 = new Lint *[2];
    Mult_tmp2[0] = new Lint [size/2+1];
    Mult_tmp2[1] = new Lint [size/2+1];
    //memset(Mult_tmp2[0],0,sizeof(Lint)*(size/2 + 1));
    //memset(Mult_tmp2[1],0,sizeof(Lint)*(size/2 + 1));
    int layer_size = 1;
    int layer_offset = 2;
    layer_size = 1;
    layer_offset = 2;
    for(int i = 0; i < layer; i++){
      layer_size *= 2;
      layer_offset *=2;
      for(int j = 0; j < layer_size; j++){
	L[0][i][j] = array[0][((2*j+1)*size)/layer_offset - 1];
	L[1][i][j] = array[1][((2*j+1)*size)/layer_offset - 1];
      }
    }
    int BS1_s = 12;
    int BS1_size = pow(2, BS1_s);
    int BS_i = (BS1_size / 2) - 1;
    int BS_offset = BS1_size/2;
    int k = 2;
    
    Lint **BS_d;
    BS_d = new Lint *[2];
    BS_d[0] = new Lint [BS1_size];
    memset(BS_d[0],0,sizeof(Lint)*BS1_size);
    BS_d[1] = new Lint [BS1_size];
    memset(BS_d[1],0,sizeof(Lint)*BS1_size);
        
    Lint **BS_Array = new Lint *[2];
    BS_Array[0] = new Lint [BS1_size];
    BS_Array[1] = new Lint [BS1_size];
    memset(BS_Array[0],0,sizeof(Lint)*BS1_size);
    memset(BS_Array[1],0,sizeof(Lint)*BS1_size);
    BS_Array[0][BS_i] = array[0][size/2-1];
    BS_Array[1][BS_i] = array[1][size/2-1];
    BS_i = (BS_i + 1) / 2 - 1;
    
    for (int i = 0 ;i < BS1_s-1; i++){
      for (int j = 0 ;j < k; j++){
	BS_Array[0][BS_i + j * BS_offset] = L[0][i][j];
	BS_Array[1][BS_i + j * BS_offset] = L[1][i][j];
      }
      BS_i = (BS_i + 1) / 2 - 1;
      BS_offset = BS_offset / 2;
      k = k * 2;
    }
    //_s->smc_set(INC_L[INC_logK - 1][INC_size_last - 1], BS_L[BS_K - 1], 32,32, "int", -1);                   
    BS_Array[0][BS1_size-1] = array[0][size-1];
    BS_Array[1][BS1_size-1] = array[1][size-1];
    //gettimeofday(&start,NULL);//stop timer here
    //for(int rep = 0; rep < 1000; rep++){
    //gettimeofday(&start1,NULL);//stop timer here
    for(int i = 0; i < BS1_size; ++i){
      Mult_tmp2[0][i] = tar[0];
      Mult_tmp2[1][i] = tar[1];
    }
    Rss_JustLT(Mult_tmp1, BS_Array, Mult_tmp2, BS1_size, ring_size, map, nodeNet);
    for(int i = 0; i < BS1_size; ++i){
      Mult_tmp1[0][i] = one[0] - Mult_tmp1[0][i];
      Mult_tmp1[1][i] = one[1] - Mult_tmp1[1][i];
    }
    
    for(int i = 1; i < BS1_size-1; i++){
      Mult_tmp2[0][i] = one[0] - Mult_tmp1[0][i-1];
      Mult_tmp2[1][i] = one[1] - Mult_tmp1[1][i-1];
    }
    
    Rss_Mult(BS_d, Mult_tmp1, Mult_tmp2, BS1_size-1, ring_size, map, nodeNet);
    
    
    BS_d[0][0] = Mult_tmp1[0][0];
    BS_d[1][0] = Mult_tmp1[1][0];
    
    BS_d[0][BS1_size-1] = one[0] - Mult_tmp1[0][BS1_size-2];
    BS_d[1][BS1_size-1] = one[1] - Mult_tmp1[1][BS1_size-2];
    
    Rss_Dot(res, BS_Array, BS_d, BS1_size, ring_size, map, nodeNet);
    
    
    p[0][0] = res[0];
    p[1][0] = res[1];
    Q[0][0] = Mult_tmp1[0][0];
    Q[1][0] = Mult_tmp1[1][0];
    layer_size = BS1_size;
    for(int i = 1; i < layer_size; i++){
      Q[0][i] = BS_d[0][i]+ BS_d[0][i]-Mult_tmp1[0][i] + Mult_tmp1[0][i-1];
      Q[1][i] = BS_d[1][i]+ BS_d[1][i]-Mult_tmp1[1][i] + Mult_tmp1[1][i-1];
    }
    
    dot_tmp[0] = L[0][BS1_s-1];
    dot_tmp[1] = L[1][BS1_s-1];
    Rss_Dot(res, Q, dot_tmp, layer_size, ring_size, map, nodeNet);
    
    p_tmp[0][0] = res[0];
    p_tmp[1][0] = res[1];
    
    Rss_JustLT(c, p_tmp, tarTmp, 1, ring_size, map, nodeNet);
    c[0][0] = one[0] - c[0][0];
    c[1][0] = one[1] - c[1][0];
    
    
    /*Debug
      Lint* dtmp = new Lint[size];
      Rss_Open(dtmp, c, 1, map, ring_size, nodeNet);
      for(int i = 0; i < 1; ++i){
      printf("c[%d] is %lX \n", i, dtmp[i]);
      }
    */
    
    
    p_tmp[0][0] = res[0] - p[0][0];
    p_tmp[1][0] = res[1] - p[1][0];
    //gettimeofday(&end1,NULL);//stop timer here
    //timer1 += 1000000 * (end1.tv_sec-start1.tv_sec)+ end1.tv_usec-start1.tv_usec;
    for(int i = BS1_s; i < layer; i++){
      layer_size *= 2;
      //printf("layersize %d\n", layer_size);
      for(int j = 0; j < layer_size; j++){
	Mult_tmp1[0][j] = Q[0][j/2];
	Mult_tmp1[1][j] = Q[1][j/2];
	if(j%2 == 0){
	  Mult_tmp2[0][j] = c[0][0];
	  Mult_tmp2[1][j] = c[1][0];
	}else{
	  Mult_tmp2[0][j] = one[0] - c[0][0];
	  Mult_tmp2[1][j] = one[1] - c[1][0];
	}
      }
      Mult_tmp1[0][layer_size] = c[0][0];
      Mult_tmp1[1][layer_size] = c[1][0];
      Mult_tmp2[0][layer_size] = p_tmp[0][0];
      Mult_tmp2[1][layer_size] = p_tmp[1][0];
      Rss_Mult(Q, Mult_tmp1, Mult_tmp2, layer_size+1, ring_size, map, nodeNet);
      
      p[0][0] += Q[0][layer_size];
      p[1][0] += Q[1][layer_size];
      
      dot_tmp[0] = L[0][i];
      dot_tmp[1] = L[1][i];
      Rss_Dot(res, Q, dot_tmp, layer_size, ring_size, map, nodeNet);
      
      p_tmp[0][0] = res[0];
      p_tmp[1][0] = res[1];
      Rss_JustLT(c, p_tmp, tarTmp, 1, ring_size, map, nodeNet);
      c[0][0] = one[0] - c[0][0];
      c[1][0] = one[1] - c[1][0];
      
      p_tmp[0][0] = res[0] - p[0][0];
      p_tmp[1][0] = res[1] - p[1][0];
    }
    Rss_Mult(Q, c, p_tmp, 1, ring_size, map, nodeNet);
    p[0][0] += Q[0][0];
    p[1][0] += Q[1][0];
    
    res[0] = p[0][0];
    res[1] = p[1][0];
    //}
    //gettimeofday(&end,NULL);//stop timer here
    //timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    //printf("runtime for bs15 with data size %d = %ld us\n", size, timer);
    //printf("runtime for bs1 with data size %d = %ld us\n", BS1_size, timer1);
    for(int i = 0; i < layer; i++){
      delete [] L[0][i];
      delete [] L[1][i];
    }

    delete [] BS_Array[0];
    delete [] BS_Array[1];
    delete [] BS_Array;
    delete [] L[0];
    delete [] L[1];
    delete [] L;
    delete [] Mult_tmp1[0];
    delete [] Mult_tmp1[1];
    delete [] Mult_tmp1;
    delete [] Mult_tmp2[0];
    delete [] Mult_tmp2[1];
    delete [] Mult_tmp2;
    delete [] Q[0];
    delete [] Q[1];
    delete [] Q;
    delete [] p[0];
    delete [] p[1];
    delete [] p;
    delete [] p_tmp[0];
    delete [] p_tmp[1];
    delete [] p_tmp;
    delete [] c[0];
    delete [] c[1];
    delete [] c;
    
    delete [] BS_d[0];
    delete [] BS_d[1];
    delete [] BS_d;
    delete [] tarTmp[0];
    delete [] tarTmp[1];
    delete [] tarTmp;
}


void bs5(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint* tar, Lint* res){
    //    struct timeval start;
  //    struct timeval end;
  //    unsigned long timer;
      int pid = nodeConfig->getID();
      Lint one[2]; //one: 0, 0, 1
      switch(pid){
      case 1:
	one[0] = 0;
	one[1] = 0;
	break;
      case 2:
	one[0] = 0;
	one[1] = 1;
	break;
      case 3:
	one[0] = 1;
	one[1] = 0;
	break;
      }
      Lint **tarTmp= new Lint *[2];
      tarTmp[0] = new Lint [1];
      tarTmp[1] = new Lint [1];
      
      tarTmp[0][0] = tar[0];
      tarTmp[1][0] = tar[1];
      
      
      Lint **c;
      c = new Lint *[2];
      c[0] = new Lint [1];
      c[1] = new Lint [1];
      
      Lint **p;
      p = new Lint *[2];
      p[0] = new Lint [1];
      //memset(p[0],array[0][size-1],sizeof(Lint)*1);
      p[1] = new Lint [1];
      //memset(p[1],array[1][size-1],sizeof(Lint)*1);
      p[0][0] = array[0][size-1];
      p[1][0] = array[1][size-1];
      
      Lint **p_tmp;
      p_tmp = new Lint *[2];
      p_tmp[0] = new Lint [1];
      p_tmp[1] = new Lint [1];
      
      int layer = 0;
      int rest = size/2;
      while(rest != 1){
	layer++;
	rest /=2;
      }
      
      Lint ***L = new Lint **[2];
      L[0] = new Lint *[layer];
      L[1] = new Lint *[layer];
      for(int i = 0; i < layer; i++){
	L[0][i] = new Lint [size/2];
	L[1][i] = new Lint [size/2];
      }
      
      Lint **Q = new Lint *[2];
      Q[0] = new Lint [size/2+1];
      Q[1] = new Lint [size/2+1];
      
      Lint **Mult_tmp1 = new Lint *[2];
      Mult_tmp1[0] = new Lint [size/2+1];
      Mult_tmp1[1] = new Lint [size/2+1];
      
      Lint **Mult_tmp2 = new Lint *[2];
      Mult_tmp2[0] = new Lint [size/2+1];
      Mult_tmp2[1] = new Lint [size/2+1];
      
      int layer_size = 1;
      int layer_offset = 2;
      layer_size = 1;
      layer_offset = 2;
      
      for(int i = 0; i < layer; i++){
	layer_size *= 2;
	layer_offset *=2;
	for(int j = 0; j < layer_size; j++){
	  L[0][i][j] = array[0][((2*j+1)*size)/layer_offset - 1];
	  L[1][i][j] = array[1][((2*j+1)*size)/layer_offset - 1];
	}
      }

      //First compr
      p_tmp[0][0] = array[0][size/2-1];
      p_tmp[1][0] = array[1][size/2-1];
      
      Rss_JustLT(c, p_tmp, tarTmp, 1, ring_size, map, nodeNet);
      c[0][0] = one[0] - c[0][0];
      c[1][0] = one[1] - c[1][0];

      p_tmp[0][0] = array[0][size/2-1] - array[0][size-1];
      p_tmp[1][0] = array[1][size/2-1] - array[1][size-1];
      
      Q[0][0] = one[0];
      Q[1][0] = one[1];
      layer_size = 1;
      Lint* dot_tmp[2];
      for(int i = 0; i < layer; i++){
	layer_size *= 2;
	//printf("layersize %d", layer_size);
	for(int j = 0; j < layer_size; j++){
	  Mult_tmp1[0][j] = Q[0][j/2];
	  Mult_tmp1[1][j] = Q[1][j/2];
	  if(j%2 == 0){
	    Mult_tmp2[0][j] = c[0][0];
	    Mult_tmp2[1][j] = c[1][0];
	  }else{
	    Mult_tmp2[0][j] = one[0] - c[0][0];
	    Mult_tmp2[1][j] = one[1] - c[1][0];
	  }
	}
	Mult_tmp1[0][layer_size] = c[0][0];
	Mult_tmp1[1][layer_size] = c[1][0];
	
	Mult_tmp2[0][layer_size] = p_tmp[0][0];
	Mult_tmp2[1][layer_size] = p_tmp[1][0];
	
	Rss_Mult(Q, Mult_tmp1, Mult_tmp2, layer_size+1, ring_size, map, nodeNet);
	
	p[0][0] += Q[0][layer_size];
	p[1][0] += Q[1][layer_size];
        
	dot_tmp[0] = L[0][i];
	dot_tmp[1] = L[1][i];

	Rss_Dot(res, Q, dot_tmp, layer_size, ring_size, map, nodeNet);

	
	p_tmp[0][0] = res[0];
	p_tmp[1][0] = res[1];

	Rss_JustLT(c, p_tmp, tarTmp, 1, ring_size, map, nodeNet);
	c[0][0] = one[0] - c[0][0];
	c[1][0] = one[1] - c[1][0];

	p_tmp[0][0] = res[0] - p[0][0];
	p_tmp[1][0] = res[1] - p[1][0];
      }
      Rss_Mult(Q, c, p_tmp, 1, ring_size, map, nodeNet);
      p[0][0] += Q[0][0];
      p[1][0] += Q[1][0];
      
      res[0] = p[0][0];
      res[1] = p[1][0];
      for(int i = 0; i < layer; i++){
	delete [] L[0][i];
	delete [] L[1][i];
      }
      delete [] L[0];
      delete [] L[1];
      delete [] L;
      delete [] Mult_tmp1[0];
      delete [] Mult_tmp1[1];
      delete [] Mult_tmp1;
      delete [] Mult_tmp2[0];
      delete [] Mult_tmp2[1];
      delete [] Mult_tmp2;
      delete [] Q[0];
      delete [] Q[1];
      delete [] Q;
      
      
      delete [] tarTmp[0];
      delete [] tarTmp[1];
      delete [] tarTmp;
      delete [] p[0];
      delete [] p[1];
      delete [] p;
      delete [] p_tmp[0];
      delete [] p_tmp[1];
      delete [] p_tmp;
      delete [] c[0];
      delete [] c[1];
      delete [] c;
}



void bs1(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint* tar, Lint* res){
    struct timeval start;
    struct timeval end;
    unsigned long timer;
    Lint **tarTmp= new Lint *[2];
    tarTmp[0] = new Lint [size];
    //memset(tarTmp[0], tar[0], size);
    tarTmp[1] = new Lint [size];
    //memset(tarTmp[1], tar[1], size);
    for(int i = 0; i < size; i++){
      tarTmp[0][i] = tar[0];
      tarTmp[1][i] = tar[1];
    }

    int pid = nodeConfig->getID();
    Lint one[2]; //one: 0, 0, 1
    switch(pid){
    case 1:
      one[0] = 0;
      one[1] = 0;
      break;
    case 2:
      one[0] = 0;
      one[1] = 1;
      break;
    case 3:
      one[0] = 1;
      one[1] = 0;
      break;
    }	
    
    Lint **c;
    Lint **d;	
    c = new Lint *[2];
    d = new Lint *[2];
    

    c[0] = new Lint [size];
    memset(c[0],0,sizeof(Lint)*size);
    c[1] = new Lint [size];
    memset(c[1],0,sizeof(Lint)*size);
    d[0] = new Lint [size];
    memset(d[0],0,sizeof(Lint)*size);
    d[1] = new Lint [size];
    memset(d[1],0,sizeof(Lint)*size);

    //gettimeofday(&start,NULL); //start timer here  
    
    //for(int rep = 0; rep < 1000; rep++){
    Rss_JustLT(c, array, tarTmp, size, ring_size, map, nodeNet);    
    for(int i = 0; i < size; ++i){
      c[0][i] = one[0] - c[0][i];
      c[1][i] = one[1] - c[1][i];
    }

    for(int i = 1; i < size-1; i++){
      tarTmp[0][i] = one[0] - c[0][i-1];
      tarTmp[1][i] = one[1] - c[1][i-1];
    }

    Rss_Mult(d, c, tarTmp, size-1, ring_size, map, nodeNet);

    /*Debug
    Lint* dtmp = new Lint[size];
    Rss_Open(dtmp, d, size, map, ring_size, nodeNet);
    for(int i = 0; i < size; ++i){
      printf("c[%d] is %lX \n", i, dtmp[i]);
    }
    */
    d[0][0] = c[0][0];
    d[1][0] = c[1][0];
    
    d[0][size-1] = one[0] - c[0][size-2];
    d[1][size-1] = one[1] - c[1][size-2];
    
    Rss_Dot(res, array, d, size, ring_size, map, nodeNet);

    delete [] tarTmp[0];
    delete [] tarTmp[1];
    delete [] tarTmp;
    
    delete [] c[0];
    delete [] c[1];
    delete [] c;
    delete [] d[0];
    delete [] d[1];
    delete [] d;
}

void Mbs1write(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint blockSize, uint ring_size, int* map, Lint** array, Lint* tar, Lint** res, Lint** bitarray, unsigned long* timer, unsigned long* Commtimer, bool outputblock){
    //outputBitarray: Ture, output the selected block; False, only output bitarray;
    int i, j, k;
    struct timeval start;
    struct timeval end;
    Lint **tarTmp= new Lint *[2];
    tarTmp[0] = new Lint [size/blockSize];
    tarTmp[1] = new Lint [size/blockSize];
    for(int i = 0; i < size/blockSize; i++){
      tarTmp[0][i] = tar[0];
      tarTmp[1][i] = tar[1];
    }
    
    int pid = nodeConfig->getID();
    Lint one[2]; //one: 0, 0, 1                                                                                
    switch(pid){
    case 1:
      one[0] = 0;
      one[1] = 0;
      break;
    case 2:
      one[0] = 0;
      one[1] = 1;
      break;
    case 3:
      one[0] = 1;
      one[1] = 0;
      break;
    }
    
    Lint **c;
    Lint **tmp;
    c = new Lint *[2];
    tmp = new Lint *[2];
    
    c[0] = new Lint [size/blockSize];
    memset(c[0],0,sizeof(Lint)*size/blockSize);
    c[1] = new Lint [size/blockSize];
    memset(c[1],0,sizeof(Lint)*size/blockSize);
    
    Lint **arrayTmp;
    arrayTmp = new Lint *[2];
    arrayTmp[0] = new Lint [size/blockSize];
    arrayTmp[1] = new Lint [size/blockSize];
    for(int i = 0; i < size/blockSize; ++i){
      arrayTmp[0][i] = array[0][(i+1)*blockSize - 1];
      arrayTmp[1][i] = array[1][(i+1)*blockSize - 1];
    }

    Rss_JustLT(c, arrayTmp, tarTmp, size/blockSize, ring_size, map, nodeNet);

    for(int i = 0; i < size/blockSize; ++i){
      c[0][i] = one[0] - c[0][i];
      c[1][i] = one[1] - c[1][i];
    }

    for(int i = 1; i < size/blockSize-1; i++){
      tarTmp[0][i] = one[0] - c[0][i-1];
      tarTmp[1][i] = one[1] - c[1][i-1];
    }
    Rss_Mult(bitarray, c, tarTmp, size/blockSize-1, ring_size, map, nodeNet);

    bitarray[0][0] = c[0][0];
    bitarray[1][0] = c[1][0];
    
    bitarray[0][size/blockSize-1] = one[0] - c[0][size/blockSize-2];
    bitarray[1][size/blockSize-1] = one[1] - c[1][size/blockSize-2];   
    
    if(outputblock){
      gettimeofday(&start,NULL);//stop timer here
      Rss_DotArray(res, array, bitarray, size, blockSize, ring_size, map, nodeNet, Commtimer);
      gettimeofday(&end,NULL);//stop timer here                                                                  
      *timer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    }
    /*
    //Debug
      Lint* dtmp = new Lint[blockSize];
      Rss_Open(dtmp, res, blockSize, map, ring_size, nodeNet);
      for(int i = 0; i < blockSize; ++i){
      printf("c[%d] is %ld \n", i, dtmp[i]);
      }
    */

    delete [] tarTmp[0];
    delete [] tarTmp[1];
    delete [] tarTmp;
    delete [] arrayTmp[0];
    delete [] arrayTmp[1];
    delete [] arrayTmp;
    
    delete [] c[0];
    delete [] c[1];
    delete [] c;
}


void bs5write(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size,  uint ring_size, int* map, Lint** array, Lint* tar, Lint* value, unsigned long* timer)
{
    struct timeval start;
    struct timeval end;
    int pid = nodeConfig->getID();
    
    int BSSize = size;
    Lint one[2]; //one: 0, 0, 1
    switch(pid){
    case 1:
      one[0] = 0;
      one[1] = 0;
      break;
    case 2:
      one[0] = 0;
      one[1] = 1;
      break;
    case 3:
      one[0] = 1;
      one[1] = 0;
      break;
    }
    Lint **tarTmp= new Lint *[2];
    tarTmp[0] = new Lint [1];
    tarTmp[1] = new Lint [1];
    tarTmp[0][0] = tar[0];
    tarTmp[1][0] = tar[1];
    
    Lint **c;
    c = new Lint *[2];
    
    c[0] = new Lint [1];
    memset(c[0],0,sizeof(Lint)*1);
    c[1] = new Lint [1];
    memset(c[1],0,sizeof(Lint)*1);

    Lint **dotRes;
    dotRes = new Lint *[2];

    dotRes[0] = new Lint [1];
    memset(dotRes[0],0,sizeof(Lint)*1);
    dotRes[1] = new Lint [1];
    memset(dotRes[1],0,sizeof(Lint)*1);    

    int layer = 0;
    int rest = BSSize;
    while(rest != 1){
      layer++;
      rest /=2;
    }
    Lint ***L = new Lint **[2];
    L[0] = new Lint *[layer];
    L[1] = new Lint *[layer];
    for(int i = 0; i < layer; i++){
      L[0][i] = new Lint [BSSize/2];
      L[1][i] = new Lint [BSSize/2];
    }
    Lint **Q = new Lint *[2];
    Q[0] = new Lint [BSSize];
    Q[1] = new Lint [BSSize];
    
    Lint **Mult_tmp1 = new Lint *[2];
    Mult_tmp1[0] = new Lint [BSSize];
    Mult_tmp1[1] = new Lint [BSSize];
    
    Lint **Mult_tmp2 = new Lint *[2];
    Mult_tmp2[0] = new Lint [BSSize];
    Mult_tmp2[1] = new Lint [BSSize];
    
    Lint **arrayTmp;
    arrayTmp = new Lint *[2];
    arrayTmp[0] = new Lint [BSSize];
    arrayTmp[1] = new Lint [BSSize];
    for(int i = 0; i < BSSize; ++i){
      arrayTmp[0][i] = array[0][i];
      arrayTmp[1][i] = array[1][i];
    }
    
    int layer_size = 1;
    long layer_offset = 2;
    
    for(int i = 0; i < layer; i++){
      for(int j = 0; j < layer_size; j++){
	long index = (long)(2*j+1);
	index = index * BSSize/layer_offset - 1;
	//printf("%d , %d, %ld \n", i, j,index);
	L[0][i][j] = arrayTmp[0][index];
	L[1][i][j] = arrayTmp[1][index];
      }
      layer_size *= 2;
      layer_offset *=2;
    }
    Q[0][0] = one[0];
    Q[1][0] = one[1];
    layer_size = 1;
    Lint* dot_tmp[2];
    Lint* Localres = new Lint [2];
    Localres[0] = 0;
    Localres[1] = 0;

    for(int i = 0; i < layer; i++){
      //printf("layersize %d", layer_size);
      dot_tmp[0] = L[0][i];
      dot_tmp[1] = L[1][i];
      Rss_Dot(Localres, Q, dot_tmp, layer_size, ring_size, map, nodeNet);
      dotRes[0][0] = Localres[0];
      dotRes[1][0] = Localres[1];
      /*Debug
      Lint* dtmp = new Lint[layer_size];
      Rss_Open(dtmp, dotRes, 1, map, ring_size, nodeNet);
      for(int i = 0; i < 1; ++i){
	printf("c[%d] is %lX \n", i, dtmp[i]);
      }
      delete [] dtmp;
      */
      
      Rss_JustLT(c, dotRes, tarTmp, 1, ring_size, map, nodeNet);
      c[0][0] = one[0] - c[0][0];
      c[1][0] = one[1] - c[1][0];
      
      for(int j = 0; j < 2*layer_size; j++){
	Mult_tmp1[0][j] = Q[0][j/2];
	Mult_tmp1[1][j] = Q[1][j/2];
	if(j%2 == 0){
	  Mult_tmp2[0][j] = c[0][0];
	  Mult_tmp2[1][j] = c[1][0];
	}else{
	  Mult_tmp2[0][j] = one[0] - c[0][0];
	  Mult_tmp2[1][j] = one[1] - c[1][0];
	}
      }
      Rss_Mult(Q, Mult_tmp1, Mult_tmp2, 2*layer_size, ring_size, map, nodeNet);
      layer_size *= 2;
    }

    gettimeofday(&start,NULL);//stop timer here
    //write

    //Rss_DotArray(res, array, Q, size, blockSize, ring_size, map, nodeNet, Commtimer);
    for(int i = 0; i < size; ++i){
      Mult_tmp1[0][i] = array[0][i] - value[0];
      Mult_tmp1[1][i] = array[1][i] - value[1];
    }
    
    Rss_Mult(Mult_tmp2, Mult_tmp1, Q, size, ring_size, map, nodeNet);

    /*
	//Debug, show result
	Lint* ddtmp = new Lint[size];
	Rss_Open(ddtmp, tarTmp, size, map, ring_size, nodeNet);
	for(int i = 0; i < size; ++i){
	  printf("after mult tmp[%d] is %ld \n", i, tarTmp[i]);
	} 
	delete [] ddtmp;
	//Debug end
    */	

    for(int i = 0; i < size; ++i){
      array[0][i] = array[0][i] - Mult_tmp2[0][i];
      array[1][i] = array[1][i] - Mult_tmp2[1][i];
    } 

    gettimeofday(&end,NULL);//stop timer here
    *timer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
   
    for(int i = 0; i < layer; i++){
      delete [] L[0][i];
      delete [] L[1][i];
    }
    delete [] L[0];
    delete [] L[1];
    delete [] L;
    delete [] Mult_tmp1[0];
    delete [] Mult_tmp1[1];
    delete [] Mult_tmp1;
    delete [] Mult_tmp2[0];
    delete [] Mult_tmp2[1];
    delete [] Mult_tmp2;
    delete [] Q[0];
    delete [] Q[1];
    delete [] Q;
    delete [] arrayTmp[0];
    delete [] arrayTmp[1];
    delete [] arrayTmp;
    delete [] Localres;
    delete [] tarTmp[0];
    delete [] tarTmp[1];
    delete [] tarTmp;
    delete [] c[0];
    delete [] c[1];
    delete [] c;
    delete [] dotRes[0];
    delete [] dotRes[1];
    delete [] dotRes;
}

void hybridbswrite(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size1, uint size2, uint orig_size, uint ring_size, int* map, Lint** bitarray1, Lint** bitarray2, Lint** array, Lint* value, unsigned long* timer, unsigned long* localtimer){
    struct timeval start;
    struct timeval end;

    //combine bitarray
    Lint **mult_tmp1= new Lint *[2];
    mult_tmp1[0] = new Lint [orig_size];
    mult_tmp1[1] = new Lint [orig_size];
    
    Lint **mult_tmp2= new Lint *[2];
    mult_tmp2[0] = new Lint [orig_size];
    mult_tmp2[1] = new Lint [orig_size];
        
    Lint **mult_tmp3= new Lint *[2];
    mult_tmp3[0] = new Lint [orig_size];
    mult_tmp3[1] = new Lint [orig_size];

    gettimeofday(&start,NULL);//stop timer here

    for(int i = 0; i < size1; i++){
      for(int j = 0; j < size2; j++){
	mult_tmp1[0][i*size2+j] = bitarray1[0][i];
	mult_tmp1[1][i*size2+j] = bitarray1[1][i];
      }
      memcpy(&mult_tmp2[0][i*size2], bitarray2[0], sizeof(Lint)*size2);
      memcpy(&mult_tmp2[1][i*size2], bitarray2[1], sizeof(Lint)*size2);
    }
    
    gettimeofday(&end,NULL);//stop timer here  
    *localtimer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;

    Rss_Mult(mult_tmp3, mult_tmp1, mult_tmp2, orig_size, ring_size, map, nodeNet);

    gettimeofday(&start,NULL);//stop timer here
    bswrite(nodeNet, nodeConfig, orig_size, ring_size, map, array, mult_tmp3, value, localtimer);
    gettimeofday(&end,NULL);//stop timer here
    *timer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;

    /*
	//Debug, show result
	Lint* ddtmp = new Lint[orig_size];
	Rss_Open(ddtmp, array, orig_size, map, ring_size, nodeNet);
	for(int i = 0; i < orig_size; ++i){
	  printf("bitarray[%d] is %ld \n", i, ddtmp[i]);
	} 
	delete [] ddtmp;
	//Debug end
	*/

    delete [] mult_tmp1[0];
    delete [] mult_tmp1[1];
    delete [] mult_tmp1;

    delete [] mult_tmp2[0];
    delete [] mult_tmp2[1];
    delete [] mult_tmp2;

    delete [] mult_tmp3[0];
    delete [] mult_tmp3[1];
    delete [] mult_tmp3;


}

void bs1write(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint* tar, Lint* value, unsigned long* timer, unsigned long* localtimer){
    struct timeval start;
    struct timeval start1;
    struct timeval end;
    //unsigned long timer;
    Lint **tarTmp= new Lint *[2];
    tarTmp[0] = new Lint [size];
    //memset(tarTmp[0], tar[0], size);
    tarTmp[1] = new Lint [size];
    //memset(tarTmp[1], tar[1], size);
    for(int i = 0; i < size; i++){
      tarTmp[0][i] = tar[0];
      tarTmp[1][i] = tar[1];
    }

    int pid = nodeConfig->getID();
    Lint one[2]; //one: 0, 0, 1
    switch(pid){
    case 1:
      one[0] = 0;
      one[1] = 0;
      break;
    case 2:
      one[0] = 0;
      one[1] = 1;
      break;
    case 3:
      one[0] = 1;
      one[1] = 0;
      break;
    }
    
    Lint **c;
    Lint **d;
    c = new Lint *[2];
    d = new Lint *[2];
    
    
    c[0] = new Lint [size];
    memset(c[0],0,sizeof(Lint)*size);
    c[1] = new Lint [size];
    memset(c[1],0,sizeof(Lint)*size);
    d[0] = new Lint [size];
    memset(d[0],0,sizeof(Lint)*size);
    d[1] = new Lint [size];
    memset(d[1],0,sizeof(Lint)*size);
    
    //gettimeofday(&start,NULL); //start timer here
    
    //for(int rep = 0; rep < 1000; rep++){
    Rss_JustLT(c, array, tarTmp, size, ring_size, map, nodeNet);
    for(int i = 0; i < size; ++i){
      c[0][i] = one[0] - c[0][i];
      c[1][i] = one[1] - c[1][i];
    }
    
    for(int i = 1; i < size-1; i++){
      tarTmp[0][i] = one[0] - c[0][i-1];
      tarTmp[1][i] = one[1] - c[1][i-1];
    }
    
    Rss_Mult(d, c, tarTmp, size-1, ring_size, map, nodeNet);
    
    d[0][0] = c[0][0];
    d[1][0] = c[1][0];
    
    d[0][size-1] = one[0] - c[0][size-2];
    d[1][size-1] = one[1] - c[1][size-2];
    
    gettimeofday(&start,NULL);//stop timer here

    //write
    for(int i = 0; i < size; ++i){
      c[0][i] = array[0][i] - value[0];
      c[1][i] = array[1][i] - value[1];
    }

    gettimeofday(&end,NULL);//stop timer here
    *localtimer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec; 
    
    Rss_Mult(tarTmp, c, d, size, ring_size, map, nodeNet);

    /*
	//Debug, show result
	Lint* ddtmp = new Lint[size];
	Rss_Open(ddtmp, tarTmp, size, map, ring_size, nodeNet);
	for(int i = 0; i < size; ++i){
	  printf("after mult tmp[%d] is %ld \n", i, tarTmp[i]);
	} 
	delete [] ddtmp;
	//Debug end
    */	

    gettimeofday(&start1,NULL);//stop timer here

    for(int i = 0; i < size; ++i){
      array[0][i] = array[0][i] - tarTmp[0][i];
      array[1][i] = array[1][i] - tarTmp[1][i];
    } 
    gettimeofday(&end,NULL);//stop timer here
    *timer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec; 
    *localtimer += 1000000 * (end.tv_sec-start1.tv_sec)+ end.tv_usec-start1.tv_usec; 


    delete [] tarTmp[0];
    delete [] tarTmp[1];
    delete [] tarTmp;
    
    delete [] c[0];
    delete [] c[1];
    delete [] c;
    delete [] d[0];
    delete [] d[1];
    delete [] d;
}

void bswrite(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint ring_size, int* map, Lint** array, Lint** bitarray, Lint* value, unsigned long* localtimer){
    struct timeval start;
    struct timeval start1;
    struct timeval end;

    Lint **tmp= new Lint *[2];
    tmp[0] = new Lint [size];
    tmp[1] = new Lint [size];
    
    Lint **tmp1= new Lint *[2];
    tmp1[0] = new Lint [size];
    tmp1[1] = new Lint [size];

    gettimeofday(&start,NULL);//stop timer here 
    for(int i = 0; i < size; ++i){
      tmp[0][i] = array[0][i] - value[0];
      tmp[1][i] = array[1][i] - value[1];
    }
    gettimeofday(&end,NULL);//stop timer here
    *localtimer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
     

    Rss_Mult(tmp1, tmp, bitarray, size, ring_size, map, nodeNet);

    gettimeofday(&start,NULL);//stop timer here
    for(int i = 0; i < size; ++i){
      array[0][i] = array[0][i] - tmp1[0][i];
      array[1][i] = array[1][i] - tmp1[1][i];
    }
    gettimeofday(&end,NULL);//stop timer here 
    *localtimer += 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;

    
    delete [] tmp[0];
    delete [] tmp[1];
    delete [] tmp;
    delete [] tmp1[0];
    delete [] tmp1[1];
    delete [] tmp1;

}


void BS(NodeNetwork* nodeNet, NodeConfiguration* nodeConfig, uint size, uint batch_size, uint alpha, uint alpha2){
  	int bits;
	int i, j, k;
	int pid = nodeConfig->getID();
	int flag = 0;
	int total;
	int ring_size = nodeNet->RING; 
	// uint batch_size = 2;

	printf("ring_size = %i\n", ring_size);
	printf("batch_size = %i\n", batch_size);
	int bytes = (ring_size+2+8-1)/8;

	struct timeval start;
	struct timeval end;
	unsigned long timer;

        struct timeval start1;
        struct timeval end1;
        unsigned long timer1 = 0;

	printf("hello, I am %d\n", pid);
	
	// splitData(output, input, 10, 10, ring_size, size);
	// splitData(output, input, 10, 10, ring_size, size);


	int map[2];
	Lint one[2]; //one: 0, 0, 1
	switch(pid){
		case 1:
			map[0] = 3;
			map[1] = 2;

			one[0] = 0;
			one[1] = 0;
			break;
		case 2:
			map[0] = 1;
			map[1] = 3;

			one[0] = 0;
			one[1] = 1;
			break;
		case 3:
			map[0] = 2;
			map[1] = 1;

			one[0] = 1;
			one[1] = 0;
			break;
	}	

	//setup prg key (will be used by all parties, only for data makeup)
	__m128i * key_prg;
	uint8_t key_raw[]    = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
	key_prg = offline_prg_keyschedule(key_raw);
	//setup prg seed(k1, k2, k3)
	uint8_t k1[] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
	uint8_t k2[] = {0xa2, 0x34, 0x6f, 0x67, 0x10, 0x1b, 0x13, 0xa3, 0x56, 0x45, 0x90, 0xb2, 0x13, 0xe3, 0x23, 0x24};


	//make up data
	Lint **a;
	Lint **b;
	Lint **c;	// c = a op b
	Lint **d;
	a = new Lint *[2];
	b = new Lint *[2];
	c = new Lint *[2];
	d = new Lint *[2];

	c[0] = new Lint [size];
	memset(c[0],0,sizeof(Lint)*size);
	c[1] = new Lint [size];
	memset(c[1],0,sizeof(Lint)*size);
	d[0] = new Lint [size];
	memset(d[0],0,sizeof(Lint)*size);
	d[1] = new Lint [size];
	memset(d[1],0,sizeof(Lint)*size);

    // for (i = 0; i < 2; i++) {
    //     c[i] = new Lint [size];
    //     memset(c[i],0,sizeof(Lint)*size);
    //     d[i] = new Lint [size];
    //     memset(d[i],0,sizeof(Lint)*size);
    // }



	Lint **Data1;
	Lint **Data2;

	Data1 = new Lint *[3];
	for(int i = 0; i< 3; i++){
		Data1[i] = new Lint [size];
		memset(Data1[i],0,sizeof(Lint)*size);
	}

	Data2 = new Lint *[3];
	for(int i = 0; i < 3; i++){
		Data2[i] = new Lint [size];
		memset(Data2[i],0,sizeof(Lint)*size);
	}

	for(int i = 0; i < size; i++){
		prg_aes_ni(Data1[0]+i, k1, key_prg);
		prg_aes_ni(Data1[1]+i, k1, key_prg);
		prg_aes_ni(Data2[0]+i, k2, key_prg);
		prg_aes_ni(Data2[1]+i, k2, key_prg);
		Data1[2][i] = i - Data1[0][i] - Data1[1][i];
		Data2[2][i] = i - Data2[0][i] - Data2[1][i];
		// printf("Data[2][%i] : %i\n", i, Data2[2][i]);

	}
	// printf("Data1[0][%i] : %llu\n", 0, Data1[0][0] );
	// printf("Data1[1][%i] : %llu\n", 0, Data1[1][0] );
	free(key_prg);

 	Lint *res = new Lint [size];
	memset(res,0,sizeof(Lint)*size);
 	Lint *res2 = new Lint [size];
	memset(res2,0,sizeof(Lint)*size);
 	Lint *res3 = new Lint [size];
	memset(res3,0,sizeof(Lint)*size);

 	Lint *res_check = new Lint [size];
	memset(res_check,0,sizeof(Lint)*size);
	//assign data
	switch(pid){
		case 1:
			a[0] = Data1[1];
			a[1] = Data1[2];
			b[0] = Data2[1];
			b[1] = Data2[2];
			break;
		case 2:
			a[0] = Data1[2];
			a[1] = Data1[0];
			b[0] = Data2[2];
			b[1] = Data2[0];
			break;
		case 3:
			a[0] = Data1[0];
			a[1] = Data1[1];
			b[0] = Data2[0];
			b[1] = Data2[1];
			break;
	}
	//Setup private target 
	Lint* target = new Lint [2]; //set to be 1+2+3=6
	switch(pid){
		case 1:
			target[0] = size/3 + 1;
			target[1] = size/3 + 1;
			break;
		case 2:
			target[0] = size/3 + 1;
			target[1] = size/3 + 1;
			break;
		case 3:
		        target[0] = size/3 + 1;
			target[1] = size/3 + 1;
			break;
	}	

	Lint* value = new Lint [2]; //set to be 2+2+3=7
	switch(pid){
		case 1:
			value[0] = size/3 + 2;
			value[1] = size/3 + 2;
			break;
		case 2:
			value[0] = size/3 + 2;
			value[1] = size/3 + 2;
			break;
		case 3:
		        value[0] = size/3 + 2;
			value[1] = size/3 + 2;
			break;
	}	


	/*
	//Do computation;
	Lint bsres[2];
	Lint numRes;
	Lint ** toBeOpen;
	toBeOpen = new Lint *[2];

	toBeOpen[0] = new Lint [1];
	//        memset(toBeOpen[0],0,sizeof(Lint));
        toBeOpen[1] = new Lint [1];
	//	memset(toBeOpen[1],0,sizeof(Lint));

	toBeOpen[0][0] = target[0];
	toBeOpen[1][0] = target[1];

	//        printf("%lX ",toBeOpen[0][0]);
	//        printf("%lX",toBeOpen[1][0]);

	//	Rss_Open(bsres, toBeOpen, 1, map, ring_size, nodeNet);
	//	printf("bs res is %lX \n", bsres[0]);

	gettimeofday(&start,NULL);//stop timer here

	for(int i = 0; i < 100; i++){
	  bs1(nodeNet, nodeConfig, size, ring_size, map, a, target, bsres);
	}
	gettimeofday(&end,NULL);//stop timer here                                  
	timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	printf("runtime for bs with data size %d = %ld us\n", size, timer);

	toBeOpen[0][0] = bsres[0];
	toBeOpen[1][0] = bsres[1];

	//	printf("%lX ",toBeOpen[0][0]);
	//	printf("%lX",toBeOpen[1][0]);
	Rss_Open(&numRes, toBeOpen, 1, map, ring_size, nodeNet);
	printf("bs res is %lX \n", numRes);
	*/

	/*
	//Do hybrid
	Lint numRes;
	Lint ** toBeOpen;
	toBeOpen = new Lint *[2];
	int rep = 10;

	toBeOpen[0] = new Lint [1];
	//        memset(toBeOpen[0],0,sizeof(Lint));
        toBeOpen[1] = new Lint [1];
	//	memset(toBeOpen[1],0,sizeof(Lint));


	unsigned long DPtimer = 0;
	unsigned long DPcommtimer = 0;

	int blockSize = size/pow(2,alpha); //beta

        Lint **Hres;                                                                 
        Hres = new Lint *[2];
	Lint bsres[2]; 

        Hres[0] = new Lint [blockSize];
        memset(Hres[0],0,sizeof(Lint)*blockSize);
        Hres[1] = new Lint [blockSize];
        memset(Hres[1],0,sizeof(Lint)*blockSize);

	
        gettimeofday(&start,NULL);//stop timer here
        for(int i = 0; i < rep; i++){
	  gettimeofday(&start1,NULL);//stop timer here
	  Modifiedbs5(nodeNet, nodeConfig, size, blockSize, ring_size, map, a, target, Hres, &DPtimer, &DPcommtimer);
	  gettimeofday(&end1,NULL);//stop timer here
	  timer1 += 1000000 * (end1.tv_sec-start1.tv_sec)+ end1.tv_usec-start1.tv_usec;
	  bs15(nodeNet, nodeConfig, blockSize, ring_size, map, Hres, target, bsres);
        }
        gettimeofday(&end,NULL);//stop timer here
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	//        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        printf("runtime for 1st phase with data size %d = %ld ms\n", size/blockSize, timer1/1000); 
        printf("DotProdcut(blockSize = %d) data size %d = %ld ms\n", blockSize, alpha, DPtimer/1000); 
        printf("DotProdcut Comm (blockSize = %d) data size %d = %ld ms\n \n", blockSize, alpha, DPcommtimer/1000); 


        printf("runtime for phase 2 bs with data size %d = %ld ms\n", blockSize, timer/1000 - timer1/1000); 
        printf("%d repeated runtime for bs with data size %d = %ld ms\n", rep, size, timer/1000); 

	toBeOpen[0][0] = bsres[0];
	toBeOpen[1][0] = bsres[1];

	Rss_Open(&numRes, toBeOpen, 1, map, ring_size, nodeNet);
	printf("bs res is %lX \n", numRes);

	
	delete [] Hres[0];
	delete [] Hres[1];
	delete [] Hres;
	*/

	/*
        //Do TribleHybrid
        Lint numRes;
        Lint ** toBeOpen;
        toBeOpen = new Lint *[2];
        int rep = 100;

        toBeOpen[0] = new Lint [1];
        toBeOpen[1] = new Lint [1];

        Lint bsres[2];
        unsigned long timer2 = 0;
        unsigned long DPtimer = 0;
        unsigned long DPcommtimer = 0;

        unsigned long DPtimer2 = 0;
        unsigned long DPcommtimer2 = 0;

        //for 1st bs
        int blockSize = size/pow(2,alpha); //beta
        //for 2nd bs
        int blockSize2 = blockSize/pow(2, alpha2);

        Lint **Hres;
        Hres = new Lint *[2];
        Hres[0] = new Lint [blockSize];
        memset(Hres[0],0,sizeof(Lint)*blockSize);
        Hres[1] = new Lint [blockSize];
        memset(Hres[1],0,sizeof(Lint)*blockSize);

        Lint **Hres2;
        Hres2 = new Lint *[2];
        Hres2[0] = new Lint [blockSize2];
        memset(Hres2[0],0,sizeof(Lint)*blockSize2);
        Hres2[1] = new Lint [blockSize2];
        memset(Hres2[1],0,sizeof(Lint)*blockSize2);


        gettimeofday(&start,NULL);//stop timer here
        for(int i = 0; i < rep; i++){
          gettimeofday(&start1,NULL);//stop timer here
          Modifiedbs1(nodeNet, nodeConfig, size, blockSize, ring_size, map, a, target, Hres, &DPtimer, &DPcommtimer);
          gettimeofday(&end1,NULL);//stop timer here
          timer1 += 1000000 * (end1.tv_sec-start1.tv_sec)+ end1.tv_usec-start1.tv_usec;

          gettimeofday(&start1,NULL);//stop timer here
          Modifiedbs1(nodeNet, nodeConfig, blockSize, blockSize2, ring_size, map, Hres, target, Hres2, &DPtimer2, &DPcommtimer2);
          gettimeofday(&end1,NULL);//stop timer here
          
          timer2 += 1000000 * (end1.tv_sec-start1.tv_sec)+ end1.tv_usec-start1.tv_usec;

          bs15(nodeNet, nodeConfig, blockSize2, ring_size, map, Hres2, target, bsres);
        }
        gettimeofday(&end,NULL);//stop timer here
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        //        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        printf("*************1st BS***************\n");
        printf("runtime for 1st phase with data size %d = %ld ms\n", size/blockSize, timer1/1000);
        printf("DotProdcut(blockSize = %d) data size %d = %ld ms\n", blockSize, alpha, DPtimer/1000);
        printf("DotProdcut Comm (blockSize = %d) data size %d = %ld ms\n \n", blockSize, alpha, DPcommtimer/1000);

        printf("*************2nd BS***************\n");
        printf("runtime for 1st phase with data size %d = %ld ms\n", blockSize/blockSize2, timer2/1000);
        printf("DotProdcut(blockSize = %d) data size %d = %ld ms\n", blockSize2, alpha2, DPtimer2/1000);
        printf("DotProdcut Comm (blockSize = %d) data size %d = %ld ms\n \n", blockSize2, alpha2, DPcommtimer2/1000);

        printf("*************3rd BS***************\n");
        printf("runtime for 3rd bs with data size %d = %ld ms\n", blockSize2, timer/1000 - timer1/1000 - timer2/1000);
        printf("*************TOTAL***************\n");
        printf("%d repeated runtime for bs with data size (%d, %d) %d = %ld ms\n", rep, alpha, alpha2, size, timer/1000);

        toBeOpen[0][0] = bsres[0];
        toBeOpen[1][0] = bsres[1];

        Rss_Open(&numRes, toBeOpen, 1, map, ring_size, nodeNet);
        printf("bs res is %lX \n", numRes);


        delete [] Hres[0];
        delete [] Hres[1];
        delete [] Hres;
        delete [] Hres2[0];
        delete [] Hres2[1];
        delete [] Hres2;
	*/
	

	/*
	//Run Dot-Product only
	Lint ** toBeOpen;
        toBeOpen = new Lint *[2];
        int rep = 100;
        toBeOpen[0] = new Lint [1];
        toBeOpen[1] = new Lint [1];


        unsigned long DPtimer = 0;
        unsigned long DPcommtimer = 0;

        int blockSize = size/pow(2,alpha); //beta

        Lint **Hres;
        Hres = new Lint *[2];
        Lint bsres[2];

        Hres[0] = new Lint [blockSize];
        memset(Hres[0],0,sizeof(Lint)*blockSize);
        Hres[1] = new Lint [blockSize];
        memset(Hres[1],0,sizeof(Lint)*blockSize);

        Lint **Darray;
        Darray = new Lint *[2];

        Darray[0] = new Lint [size/blockSize];
        memset(Darray[0],0,sizeof(Lint)*size/blockSize);
        Darray[1] = new Lint [size/blockSize];
        memset(Darray[1],0,sizeof(Lint)*size/blockSize);

        memcpy(Darray[0], a[0], sizeof(Lint)*size/blockSize);
        memcpy(Darray[1], a[1], sizeof(Lint)*size/blockSize);

        gettimeofday(&start,NULL);//stop timer here
        for(int i = 0; i < rep; i++){
          Rss_DotArray(Hres, a, Darray, size, blockSize, ring_size, map, nodeNet, &DPcommtimer);
        }
        gettimeofday(&end,NULL);//stop timer here
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        //        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        printf("runtime for DotP with data size %d with alpha %d = %ld ms\n", size, size/blockSize, timer/1000);
        printf("DotProdcut Comm (blockSize = %d) data size %d = %ld ms\n \n", blockSize, alpha, DPcommtimer/1000);

        delete [] Hres[0];
        delete [] Hres[1];
        delete [] Hres;
        delete [] Darray[0];
        delete [] Darray[1];
        delete [] Darray;
	*/

	
	//Do write
	int rep = 1000;	
	unsigned long DPtimer = 0;
        unsigned long DPcommtimer = 0;
	int blockSize = 1;
	unsigned long WriteTimer = 0;

	Lint **bitarray;
        bitarray = new Lint *[2];

        bitarray[0] = new Lint [size];
        //memset(bitarray[0],0,sizeof(Lint)*blockSize);
        bitarray[1] = new Lint [size];
        //memset(bitarray[1],0,sizeof(Lint)*blockSize);


        gettimeofday(&start,NULL);//stop timer here

        for(int i = 0; i < rep; i++){
	  //          gettimeofday(&start1,NULL);//stop timer here

	  //          Modifiedbs1(nodeNet, nodeConfig, size, blockSize, ring_size, map, a, target, bitarray, &DPtimer, &DPcommtimer);
          //gettimeofday(&end1,NULL);//stop timer here

          //timer1 += 1000000 * (end1.tv_sec-start1.tv_sec)+ end1.tv_usec-start1.tv_usec;
          //bs5write(nodeNet, nodeConfig, size, ring_size, map, a,target, value, &WriteTimer);
          bs1write(nodeNet, nodeConfig, size, ring_size, map, a,target, value, &WriteTimer, &DPcommtimer);

	}

        gettimeofday(&end,NULL);//stop timer
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;

        printf("runtime for bitarray generation phase with data size %d = %ld ms\n", size, timer/1000 - WriteTimer/1000);
        //printf("DotProdcut(blockSize = %d) data size %d = %ld ms\n", blockSize, alpha, DPtimer/1000);
        //printf("DotProdcut Comm (blockSize = %d) data size %d = %ld ms\n \n", blockSize, alpha, DPcommtimer/1000);

        printf("runtime for bs write with data size %d = %ld ms\n", size, WriteTimer/1000);
	printf("runtime for bs local work with data size %d = %ld ms\n", size, DPcommtimer/1000);
        printf("%d repeated runtime for bs wirte with data size %d = %ld ms\n", rep, size, timer/1000);


        delete [] bitarray[0];
        delete [] bitarray[1];
        delete [] bitarray;

	//*/

	/*
	//Do hybrid write
	Lint numRes;
	int rep = 100;
	unsigned long DPtimer = 0;
	unsigned long DPcommtimer = 0;
	unsigned long writetimer = 0;
	unsigned long timer2 = 0;


	int blockSize = size/pow(2,alpha); //beta

	Lint **bitarray1;
        bitarray1 = new Lint *[2];
        bitarray1[0] = new Lint [size/blockSize];
        bitarray1[1] = new Lint [size/blockSize];

	Lint **bitarray2;
        bitarray2 = new Lint *[2];
        bitarray2[0] = new Lint [blockSize];
        bitarray2[1] = new Lint [blockSize];
	

        Lint **Hres;                                                                 
        Hres = new Lint *[2];
	Lint bsres[2]; 

        Hres[0] = new Lint [blockSize];
        memset(Hres[0],0,sizeof(Lint)*blockSize);
        Hres[1] = new Lint [blockSize];
        memset(Hres[1],0,sizeof(Lint)*blockSize);

	
        gettimeofday(&start,NULL);//stop timer here
        for(int i = 0; i < rep; i++){
	  gettimeofday(&start1,NULL);//stop timer here
	  //Modifiedbs5(nodeNet, nodeConfig, size, blockSize, ring_size, map, a, target, Hres, &DPtimer, &DPcommtimer);
	  Mbs1write(nodeNet, nodeConfig, size, blockSize, ring_size, map, a, target, Hres, bitarray1, &DPtimer, &DPcommtimer, true);	  
	  gettimeofday(&end1,NULL);//stop timer here
	  timer1 += 1000000 * (end1.tv_sec-start1.tv_sec)+ end1.tv_usec-start1.tv_usec;

	  gettimeofday(&start1,NULL);//stop timer here
	  Mbs1write(nodeNet, nodeConfig, blockSize, 1, ring_size, map, Hres, target, Hres, bitarray2, &DPtimer, &DPcommtimer, false);
	  gettimeofday(&end1,NULL);//stop timer here
	  timer2 += 1000000 * (end1.tv_sec-start1.tv_sec)+ end1.tv_usec-start1.tv_usec;

	  //bs15(nodeNet, nodeConfig, blockSize, ring_size, map, Hres, target, bsres);
	  hybridbswrite(nodeNet, nodeConfig, size/blockSize, blockSize, size, ring_size, map, bitarray1, bitarray2, a, value, &writetimer, &DPcommtimer);
	}
        gettimeofday(&end,NULL);//stop timer here
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
	//        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        printf("runtime for 1st phase with data size %d = %ld ms\n", size/blockSize, timer1/1000); 
        printf("runtime for 2nd phase with data size %d = %ld ms\n", blockSize, timer2/1000); 

        printf("runtime for 3nd phase with data size %d = %ld ms\n", blockSize, (timer - timer1 - timer2 - writetimer)/1000); 
        printf("runtime for write phase with data = %ld ms\n", writetimer/1000); 
        printf("runtime for local work phase with data = %ld ms\n", DPcommtimer/1000); 

        printf("%d repeated runtime for bs write with data size %d = %ld ms\n", rep, size, timer/1000); 

	

        delete [] bitarray1[0];
        delete [] bitarray1[1];
        delete [] bitarray1;
        delete [] bitarray2[0];
        delete [] bitarray2[1];
        delete [] bitarray2;
	
	delete [] Hres[0];
	delete [] Hres[1];
	delete [] Hres;
	
	*/

	//Free Memory
	delete [] res;
	delete [] res2;
	delete [] res3;
	delete [] res_check;
	for(int i = 0; i<3;i++){

		delete [] Data1[i];
		delete [] Data2[i];
	
	}

	delete [] Data1;
	delete [] Data2;
	delete [] a;
	delete [] b;



	delete [] c[0];
	delete [] c[1];
	delete [] c;

	delete [] d[0];
	delete [] d[1];
	delete [] d; 
	
}
