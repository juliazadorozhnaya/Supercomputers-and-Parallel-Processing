#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <omp.h>

void prt1a(char *t1, double *v, int n,char *t2);
void wtime(double *t) {
	static int sec = -1;
	struct timeval tv;
	gettimeofday(&tv, (void *)0);
	if (sec < 0) sec = tv.tv_sec;
	*t = (tv.tv_sec - sec) + 1.0e-6*tv.tv_usec;
}

int N;
double *A;
#define A(i,j) A[(i)*(N+1)+(j)]
double *X;

int main(int argc,char **argv) {
	double time0, time1;
	FILE *in;
	int i, j, k;
	int threads = omp_get_num_procs();
	in=fopen("data.in","r");
	if(in==NULL) {
		printf("Can not open 'data.in' "); exit(1);
	}
	i=fscanf(in,"%d", &N);
	if(i<1) {
		printf("Wrong 'data.in' (N ...)"); exit(2);
	}

	omp_set_num_threads(threads);
	/* create arrays */
	A=(double *)malloc(N*(N+1)*sizeof(double));
	X=(double *)malloc(N*sizeof(double));
	printf("GAUSS %dx%d\n----------------------------------\n",N,N);
	/* initialize array A*/

    #pragma omp parallel for shared(A, N) private(i, j) schedule(static)
    for(i=0; i <= N-1; i++)
		for(j=0; j <= N; j++)
			if (i==j || j==N)
				A(i,j) = 1.f;
			else 
				A(i,j)=0.f;
	wtime(&time0);


	/* elimination */
	double m;
	omp_set_nested(1);
    for (i=0; i<N-1; i++) {
    	#pragma omp parallel for shared(N, A, i) private(m, j,k) schedule(dynamic)	
		for (k=i+1; k <= N-1; k++)
			m = A(k,i) / A(i,i);
			for (j=i+1; j <= N; j++)
				A(k,j) = A(k,j)- (m * A(i,j));
	/* reverse substitution */
	}

    X[N-1] = A(N-1,N)/A(N-1,N-1); 
    #pragma omp parallel for shared(A, X) private(k,j) schedule(dynamic)
    for (j=N-2; j>=0; j--) {
		for (k=0; k <= j; k++)
			A(k,N) = A(k,N)-A(k,j+1)*X[j+1];
		X[j]=A(j,N)/A(j,j);
	}
	wtime(&time1);
	printf("Time in seconds=%gs\n",time1-time0);
	prt1a("X=(", X,N>9?9:N,"...)\n");
	free(A);
	free(X);
	return 0;
}

void prt1a(char * t1, double *v, int n,char *t2) {
	int j;
	printf("%s",t1);
    for(j=0;j<n;j++)
		printf("%.4g%s",v[j], j%10==9? "\n": ", ");
	printf("%s",t2);
}