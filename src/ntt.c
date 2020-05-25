#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "ntt.h"
/*
* Computes forward NTT
* in Z[X]/p
*    /(X^N-1)
* Flags adjust parallelism
*/
void ntt(long* vec, long* dest, long N, long P, long root){
	long i,j,sum; 
    for (i=0; i<N; i++){
        sum=0;
        for (j=0; j<N; j++){
            sum+=vec[j] * power_mod(root,i*j,P);
            sum%=P;
        }
        dest[i]=sum;
    }
}
/*
* Computes reverse NTT
*/
void inv_ntt(long* vec, long* dest, long N, long P, long root){
    ntt(vec,dest,N,P,inverse(root,P));
    long inv_N = inverse(N,P);
    for (int i=0; i<N; i++)
        dest[i]=(dest[i]*inv_N)%P;
}

/*
* Multiplies two elements in Z[X]/P
*                            /(X^N-1)
* root has order N in F_P
*/
void convolution(long* vec1, long* vec2, long* dest, long* temp, long N, long P, long root){
    ntt(vec1,dest,N,P,root);
    ntt(vec2,temp,N,P,root);
	//printf("F(v)\n");
	//printPoly(dest,N);
	//printf("F(v2)\n");
	//printPoly(temp,N);
    long i;
    for (i=0; i<N; i++){
        temp[i]=(temp[i]*dest[i])%P;
    }
	//printf("F(v1)*F(v2)\n");
	//printPoly(temp,N);
    inv_ntt(temp,dest,N,P,root);
}

/*
* Slow convolution algorithm to check results of NTT
*/
void check_conv(long* vec1, long* vec2, long* dest, long N, long P){
	long i,j,di;
	for (i=0; i<N; i++)
		dest[i]=0;
	for (i=0; i<N; i++){
		for (j=0; j<N; j++){
			di=(i+j)%N;
			dest[di]+=(vec1[i]*vec2[j]);
			dest[di]%=P;
		}
	}
}
void printPoly(long* vec, long N){
    printf("	%ldX^%d ",vec[0],0);
    for (long i=1; i<N; i++){
        printf("+ %ldX^%ld",vec[i],i);
    }
    printf("\n");
}

/*
* checks if g is generator for F_P
* O(P*log(P))
*/
int is_generator(long g,long P){
	for (long i=1; i<=P/2; i++){
		if (i|(P-1) && power_mod(g,i,P)==1)
			return 0;
	}
	return 1;
}
/*
* Finds generator for F_P
* (P must be prime)
* O(P^2*log(P))
*/
long generator(long P){
	for (long i=1; i<P; i++){
		if (is_generator(i,P)){
			printf("%ld is generator for %ld\n",i,P);
			return i;
		}
	}
	printf("Unable to find generator for %ld\n",P);
	return -1;
}
/*
* finds element of F_P which has order N
* this produces the w for NTT (Equivalent to N-th root of 1 in DFT)
*/
long get_root(long N, long P){
	long gen = generator(P);
	long k = (P-1)/N;
	return power_mod(gen,k,P);
}

//implements fast power algorithm O(log(exp))
long power_mod(long base, long exp, long mod){
    long res=1;
    long cur=base;
    while (exp>0){
        if (exp%2==1)
            res=(res*cur)%mod;
        cur=(cur*cur)%mod;
        exp/=2;
    }
    return res;
}

long true_mod(long val, long mod){
    long ret=val%mod;
    if (ret<0)
        return ret+mod;
    return ret;
}
/*
 Takes O(log(mod)) time,
 Uses extended euclidian algorithm (although it does not look like it)
*/
long inverse(long val, long mod){
	long x,y,a,b,temp;
	x=mod;
	y=val;
	a=0;
	b=1;
	while (y!=0){
	  	temp=b;
	 	b=a-(x/y)*b;
	 	a=temp;
	  	temp=y;
	  	y=x%y;
	  	x=temp;
	}
	if (x==1)
		return true_mod(a,mod);
	else
		printf("Unable to find inverse for %ld in F%ld\n",val,mod);
}

void test_ntt(){
	long vec[3]={2,3,3};
	long trans[3]={1,6,6};
	long dest[3];
	ntt(vec,dest,3,7,2);
	printf("Forward transform result\n");
	printPoly(dest,3);
	printf("Correct\n");
	printPoly(trans,3);
	printf("Inverse transform result\n");
	inv_ntt(trans,dest,3,7,2);
	printPoly(dest,3);
	printf("Correct\n");
	printPoly(vec,3);
}

void test_inverse(){
    long mods[5]={3,5,7,11,13};
    long val=2;
    long invs[5]={2,3,4,6,7};
    for (int i=0; i<5; i++){
        long res=inverse(val,mods[i]);
        if (true_mod(res,mods[i])!=invs[i])
            printf("Inverse Error: 2*%ld!=1 mod %ld\n",true_mod(res,mods[i]),mods[i]);
    }
}
long power(long l1, long l2){
	long res=1;
	while(l2-->0)
		res*=l1;
	return res;
}
void test_power_mod(){
    long mods[5]={3,5,7,11,13};
	for (int i=0; i<5; i++){
		for (int j=1; j<6; j++){
			long res = power_mod(2,j,mods[i]);
			long cor = power(2,j)%mods[i];
			if (res!=cor)
				printf("2^%ld!=%ld mod %ld\n",res,cor,mods[i]);
		}
	}
}

/*Requirements
*  kN+1=P (k in Z)
*  
*/
int main(int argc, char** argv){
	//test_ntt();
	test_inverse();
	test_power_mod();
	printf("Tests Completed\n");
	if (argc<3){
        printf("Recieved %d args. Usage: ntt <N> <P>\n",argc);
        exit(0);
    }
    long N,P,root,i;
    N = atoi(argv[1]);
    P = atoi(argv[2]);
    root = get_root(N,P);
	printf("root: %ld\n",root);

    long* vec1 = (long*)malloc(sizeof(long)*N);
    long* vec2= (long*)malloc(sizeof(long)*N);
    long* res = (long*)malloc(sizeof(long)*N);
    long* temp = (long*)malloc(sizeof(long)*N);

    srand(time(NULL));
    for (i=0; i<N; i++){
        vec1[i] = rand()%P;
        vec2[i] = rand()%P;
    }

    printf("Polynomial 1\n");
    printPoly(vec1,N);
    printf("Polynomial 2\n");
    printPoly(vec2,N);

    convolution(vec1,vec2,res,temp,N,P,root);

    printf("NTT Result\n");
    printPoly(res,N);
	
	check_conv(vec1,vec2,temp,N,P);

	printf("Correct Result\n");
	printPoly(temp,N);

	//check results
	int success=1;
	for (i=0; i<N; i++){
		if (res[i]!=temp[i])
			success=0;
	}
	if (success)
		printf("Test is Success\n");
	else
		printf("Test is Failure.\n");
	free(vec1);
	free(vec2);
	free(res);
	free(temp);
}
