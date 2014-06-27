#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int i = 3;
    int ciao;
    ciao = -3;
    if (i % 2 == 0) { 
       ciao = i;
       i++;
       i++;
    }
    printf("%d", ciao);
}
