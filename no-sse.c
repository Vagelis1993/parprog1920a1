#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void get_walltime(double *wct) {
	struct timeval tp;
	gettimeofday(&tp,NULL);
	*wct = (double)(tp.tv_sec+tp.tv_usec/1000000.0);
}

float inline get_value(float **a, int i, int j, float *k){
  return a[i-1][j-1]*k[0] + a[i-1][j]*k[1] + a[i-1][j+1]*k[2] + a[i][j-1]*k[3] + a[i][j]*k[4] + a[i][j+1]*k[5] + a[i+1][j-1]*k[6] +
    a[i+1][j]*k[7] + a[i+1][j+1]*k[8];
}

void inline free_memory (float **img1, int n){
	int i;
	for (i=0;i<n;i++) {
		free (img1[i]);
	}
	free (img1);
}

// gcc -Wall -O2 no-sseB.c -o no-sseB -DN=10000 -DR=10000
int main() {
float **image1, **image2;
int i, j;
double ts, te; // time start and time end

	float K[] = {0.5, 0.5, 0.5, 0.5, 5.0, 0.5, 0.5, 0.5, 0.5};
	image1 =(float **)malloc(N*sizeof(float *));
	if (image1==NULL) exit(1);
	image2 = (float **)malloc(N*sizeof(float *));
	if (image2==NULL) {free(image1); exit(1); };

	for (i=0;i<N;i++){
		image1[i] = (float *)malloc(R*sizeof(float));
		image2[i] = (float *)malloc(R*sizeof(float));
		if (image1[i] == NULL || image2[i] == NULL){
			free_memory (image1,i);
			free_memory (image2,i);
			exit(1);

		}
	}

	for (i=0;i<N;i++) {
		for(j=0;j<R;j++){
			image1[i][j] = 3.0*i*j;
			image2[i][j] = i*j;
		}
	}

get_walltime(&ts);

for(i=1;i<N-1;i++) {
	for(j=1;j<R-1;j++) {
		image2[i][j] = get_value(image1, i, j, K); 

	}
}
get_walltime(&te);

printf("secs = %f\n", (te-ts)*1e6); //mikroseconds

free_memory (image1,N);
free_memory (image2,N);

return 0;

}