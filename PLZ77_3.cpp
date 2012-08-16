/*
 * Parallel suffix tree + sequential searching
 * Current version doesn't work. We need to compute
 * minimum index at each internal node of suffix
 * tree to know when to stop searching
 */

#include <cstdio>
#include <iostream>

#include "suffixTree.h"
#include "sequence.h"
#include "Base.h"
#include "test.h"
#include "utils.h"

using namespace std;

pair<int *, int> ParallelLPFtoLZ(int *lpf, int n);

pair<int *, int> ParallelLZ77(int *s, int n) {
    startTime();
    suffixTree st = buildSuffixTree(s, n);
    nextTime("\tSuffix Tree");
    //references
    stNode<int> *nodes = st.nodes;
    int root = st.getRoot(); //I'm not sure!

    int *minLabel = new int[st.m];

    //initialize


    //first round rake, only for children
    parallel_for (int i = 0; i < n; i++) { //does it really gurantee i is ith suffix?
        minLabel[i] = i;
        int pid = nodes[i].parentID;
        utils::writeMin(minLabel + pid, i);
    }

    bool changed = true;
    while (changed) {
        changed = false;
        parallel_for (int i = n; i < st.m; i++) {
            int pid = nodes[i].parentID;
            if (utils::writeMin(minLabel + pid, minLabel[i]))
                changed = true;
        }
    }

    int dep = getDepth(st.m);
    int **up = new int*[dep];
    int **minup = new int*[dep];
    
    for (int i = 0; i < dep; i++) {
        up[i] = new int[st.m];  //this should be computed for all nodes
        minup[i] = new int[st.m]; 
    }

    //compute up
    parallel_for (int i = 1; i < st.m; i++) {
        up[0][i] = nodes[i].parentID;
    }

    for (int d = 1; d < dep; d++) {
        parallel_for (int i = 0; i < st.m; i++) {
            up[d][i] = up[d-1][up[d-1][i]];
        }   
    }

    //compute minup
    parallel_for (int i = 0; i < dep; i++) {
       minup[0][i] = minLabel[i];
    }

    for (int d = 1; d < dep; d++) {
        parallel_for (int i = 0; i < st.m; i++) {
            minup[d][i] = min(minup[d-1][i], minup[d-1][up[d-1][i]]); 
        } 
    }

    //compute lpf by binary search
    int * lpf = new int[n];

    //the efficience of following code can be improved by using additional spaces
    parallel_for (int i = 0; i < n; i++) {
        int cur = i;
        int val = i;

        int d = dep;
        while(d >= 0 && cur != root) {
            if (minup[d][cur] < i) {
                val = minup[d][cur];        
                d--;    //scala down the scope
            } else {
                cur = up[d-1][cur];
            }
        }
        lpf[i] = val;
    }

    pair<int *, int> r = ParallelLPFtoLZ(lpf, n);
    delete lpf;
    return r;
}

int main(int argc, char *argv[]) {
    return test_main(argc, argv, (char *)"LZ77 using suffix tree (nlog n)", ParallelLZ77);
}
