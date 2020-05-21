#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <emmintrin.h>


// compile with: gcc -Wall -O2 sseVag.c -o sse -DN=10000 -DR=10000
// without function: secs = 310364.961624
// SSE2 (Streaming SIMD Extensions 2) SIMD = Single Instruction, Multiple Data

void get_walltime(double *wct) {
  struct timeval tp;
  gettimeofday(&tp,NULL);
  *wct = (double)(tp.tv_sec+tp.tv_usec/1000000.0);
}

void inline free_memory (float **img1, int n){
  int i;
  for (i=0;i<n;i++) {
    free (img1[i]);
  }
  free (img1);
}

// From Internet
inline __m128 dot_product(__m128 v0, __m128 v1)
{
    v0 = _mm_mul_ps(v0, v1);

    v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(2, 3, 0, 1));
    v0 = _mm_add_ps(v0, v1);
    v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(0, 1, 2, 3));
    v0 = _mm_add_ps(v0, v1);

    return v0;
}

float ** create_image_array(){
  int i;
  float **img;
  img = (float **)malloc(N*sizeof(float *));
  if (img==NULL) exit(1); 
  for(i=0;i<N;i++){
    img[i] = (float *)malloc(R*sizeof(float));
    if(img[i] == NULL) exit(1);
  }
  return img;
}



int main() {
  float **image1, **image2, *product, *kimg2, *kaimg1;
  float K[] = {0.5, 0.5, 0.5, 0.5, 5.0, 0.5, 0.5, 0.5, 0.5};
  int i,j,c;
  double ts,te;
  __m128  *vfimage1, *vfK, *temp, *vfproduct;

  
  // allocate test arrays
  image1 = create_image_array();

  image2 = create_image_array();



  
  i = posix_memalign((void **)&kimg2,16,8*sizeof(float));
  if (i!=0) exit(1);
  i = posix_memalign((void **)&kaimg1,16,8*sizeof(float));
  if (i!=0) exit(1);
  i = posix_memalign((void **)&product,16,8*sizeof(float));
  if (i!=0) exit(1);
  // fill array with constants
  for(i=0;i<8;i++)
    kimg2[i] = K[i];


  
  //initialize all arrays - cache warm-up
  for (i=0;i<N;i++) {
    for (j=0;j<R;j++){
      image1[i][j] = 3.0*i*j;
      image2[i][j] = i*j;
    }
  }
  
  // get current time
  get_walltime(&ts);
  
  // make calculations 
  for (i=1;i<N-4;i++) {
    for (j=1;j<R-4;j++) {
      image2[i][j] = 0.;
      vfK = (__m128 *)kimg2;
      vfimage1 = (__m128 *)kaimg1;
      vfproduct = (__m128 *)product;

      *vfimage1 = _mm_load_ps1(&(image1[i-1][j-1])); 
      temp = vfimage1;
      temp++;
      *temp = _mm_load_ps1(&(image1[i][j-1]));//t++;


      for (c=0;c<8;c+=4) {
        *vfproduct = dot_product(*vfK, *vfimage1); 
        *vfproduct = _mm_add_ps(*vfproduct,*vfproduct);
        *vfproduct = _mm_shuffle_ps(*vfproduct,*vfproduct,_MM_SHUFFLE(3,1,0,1));
        *vfproduct = _mm_add_ps(*vfproduct,*vfproduct);
        *vfproduct = _mm_shuffle_ps(*vfproduct,*vfproduct,_MM_SHUFFLE(3,1,0,1));        
        image2[i][j] += product[0]; // this contains the result of dot product
        vfK++; vfimage1++; vfproduct++;
      }


    }
  }
 
  // get current time
  get_walltime(&te);
  

  printf("microsecs = %.2f\n", (te-ts)*1e6); // print total time
  
  // free memory
  free (product);
  free_memory (image1,N);
  free_memory (image2,N);
  
  
  return 0;
}