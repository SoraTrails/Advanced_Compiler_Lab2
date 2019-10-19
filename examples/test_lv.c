#include <stdio.h>
#define WriteLine() printf("\n");
#define WriteLong(x) printf(" %lld", (long)x);
#define ReadLong(a) if (fscanf(stdin, "%lld", &a) != 1) a = 0;
#define long long long


void main()
{
    long a,b,c,x,y,z,t;
    a = x + y;
    t = a;
    c = a + x;
    if (x == 0){
        b = z + t;
    }
    c = y + 1;

    WriteLong(a);
    WriteLong(b);
    WriteLong(c);
}
