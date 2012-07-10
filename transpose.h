// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch, Julian Shun and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef A_TRANSPOSE_INCLUDED
#define A_TRANSPOSE_INCLUDED

#define _TRANS_THRESHHOLD 64
#include "parallel.h"

template <class E>
struct transpose {
  E *A, *B;
  transpose(E *AA, E *BB) : A(AA), B(BB) {}

  inline void transR(int rStart, int rCount, int rLength,
	      int cStart, int cCount, int cLength) {
    //cout << "cc,rc: " << cCount << "," << rCount << endl;
    if (cCount < _TRANS_THRESHHOLD && rCount < _TRANS_THRESHHOLD) {
      for (int i=rStart; i < rStart + rCount; i++) 
	for (int j=cStart; j < cStart + cCount; j++) 
	  B[j*cLength + i] = A[i*rLength + j];
    } else if (cCount > rCount) {
      int l1 = cCount/2;
      int l2 = cCount - cCount/2;
      parallel_spawn transR(rStart,rCount,rLength,cStart,l1,cLength);
      transR(rStart,rCount,rLength,cStart + l1,l2,cLength);
      parallel_sync;
    } else {
      int l1 = rCount/2;
      int l2 = rCount - rCount/2;
      parallel_spawn transR(rStart,l1,rLength,cStart,cCount,cLength);
      transR(rStart + l1,l2,rLength,cStart,cCount,cLength);
      parallel_sync;
    }	
  }

  inline void trans(int rCount, int cCount) {
    transR(0,rCount,cCount,0,cCount,rCount);
  }
};

template <class E>
struct blockTrans {
  E *A, *B;
  int *OA, *OB, *L;

  blockTrans(E *AA, E *BB, int *OOA, int *OOB, int *LL) 
    : A(AA), B(BB), OA(OOA), OB(OOB), L(LL) {}

  inline void transR(int rStart, int rCount, int rLength,
	     int cStart, int cCount, int cLength) {
    //cout << "cc,rc: " << cCount << "," << rCount << endl;
    if (cCount < _TRANS_THRESHHOLD && rCount < _TRANS_THRESHHOLD) {
      for (int i=rStart; i < rStart + rCount; i++) 
	for (int j=cStart; j < cStart + cCount; j++) {
	  E* pa = A+OA[i*rLength + j];
	  E* pb = B+OB[j*cLength + i];
	  int l = L[i*rLength + j];
	  //cout << "pa,pb,l: " << pa << "," << pb << "," << l << endl;
	  for (int k=0; k < l; k++) *(pb++) = *(pa++);
	}
    } else if (cCount > rCount) {
      int l1 = cCount/2;
      int l2 = cCount - cCount/2;
      parallel_spawn transR(rStart,rCount,rLength,cStart,l1,cLength);
      transR(rStart,rCount,rLength,cStart + l1,l2,cLength);
      parallel_sync;
    } else {
      int l1 = rCount/2;
      int l2 = rCount - rCount/2;
      parallel_spawn transR(rStart,l1,rLength,cStart,cCount,cLength);
      transR(rStart + l1,l2,rLength,cStart,cCount,cLength);
      parallel_sync;
    }	
  }
 
  inline    void trans(int rCount, int cCount) {
    transR(0,rCount,cCount,0,cCount,rCount);
  }
} ;

#endif // A_TRANSPOSE_INCLUDED
