#include <stdio.h>
#define WriteLine() printf("\n");
#define WriteLong(x) printf(" %lld", (long)x);
#define ReadLong(a) if (fscanf(stdin, "%lld", &a) != 1) a = 0;
#define long long long


void main()
{
    long s, a, i, k, b, n;
    s = 0;
    a = 4;
    i = 0;
    if(k == 0){
        b = 1;
    }
    else{
        b = 2;
    }
    while(i < n){
        s = s + a*b;
        i = i + 1;
    }
    WriteLong(s);
}
