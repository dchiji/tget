#include <stdio.h>
#include <stdlib.h>
#include "../sha1.h"

int main()
{
    unsigned int i;
    unsigned int size;
    unsigned char *buf;

    buf = (unsigned char *)sha1_padding("hello", 5, (size_t *)&size);
    buf = (char *)sha1(buf, size / 4, (int *)malloc(sizeof(int) * 80));

    for(i = 0; i < 20; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");
}
