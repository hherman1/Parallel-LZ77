//LZ77 implementation of "A simple algorithm for computing the
//Lempel-Ziv factorization", DCC 2008

#include <stdio.h>
#include <iostream>
#include "gettime.h"
#include "utils.h"
#include "cilk.h"
#include "test.h"
using namespace std;

timer saTime;

pair<int*,int*> suffixArray(int *s, int n, bool findLCPs);

pair<int*,int> compute(int* A, int n){
  startTime();
  saTime.start();
  pair<int*,int*> SA_LCP = suffixArray(A,n,true);
  _tm.stop();
  cout<<"\tsuffix array time:";
  saTime.reportNext();

  //getting arrays in right format
  int* LCP = newA(int,n+1); LCP[0]=0; LCP[n]=0;
  int* SA = newA(int,n+1); SA[n]=-1;
  for(int i=0;i<n;i++)SA[i]=SA_LCP.first[i];
  for(int i=1;i<n;i++)LCP[i]=SA_LCP.second[i-1];
  free(SA_LCP.first); free(SA_LCP.second);
  
  startTime();
  int* LPF = newA(int,n);
  int top = 0;
  int* stack = newA(int,n);
  stack[0] = 0;
  for(int i=1;i<=n;i++){ //compute LPF array
    while(top != -1 && 
	  ((SA[i] < SA[stack[top]]) || 
	   ((SA[i] > SA[stack[top]]) && (LCP[i] <= LCP[stack[top]])))) {
      int stack_top = stack[top];
      if(SA[i] < SA[stack_top]){
	LPF[SA[stack_top]] = max(LCP[i],LCP[stack_top]);
	LCP[i] = min(LCP[i],LCP[stack_top]);
      } else {
	LPF[SA[stack_top]] = LCP[stack_top];
      }
      top--;
    }
    if(i < n) stack[++top] = i;
  }

  free(stack); free(SA); free(LCP);

  //compute LZ array
  int* LZ = newA(int,n);
  LZ[0] = 0;
  int j = 0;
  while(LZ[j] < n){
    LZ[j+1] = LZ[j] + max(1,LPF[LZ[j]]);
    j++;
  }
  reportTime("\tsequential LZ77");
  free(LPF);
  return pair<int*,int>(LZ,j);
}

int main(int argc, char *argv[]) {
    return test_main(argc, argv, (char *)"Seq LZ77 DCC 2008", compute);
}

