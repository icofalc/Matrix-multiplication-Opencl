__kernel void matrice_mul(__global int *A, __global int *B,__global int *C,int n){
  
   
   int tx = get_global_id(0); 
   int ty = get_global_id(1);
 

   int wA=n; 
   int wB=n;    

   int value = 0;
   for (int k = 0; k < wA; ++k)
   {
      int elementA = A[ty * wA + k];
      int elementB = B[k * wB + tx];
      value += elementA * elementB;
   }
 
 
   C[ty * wA + tx] = value;
}

