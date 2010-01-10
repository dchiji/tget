#include <stdio.h>
#include <stdlib.h>

#include "bencode.h"
#include "sha1.h"

int main(int argc, char *argvs[])
{
    char *filename;
    bencode_t *metainfo;
    unsigned char *p;
    unsigned int size;
    int i;

    if(argc < 2) {
        printf("too few arguments\n");
        return -1;
    }

    filename = argvs[1];
    if(!(metainfo = be_decode_file(filename))) {
        return -1;
    }

    p = be_encode(be_dict_lookup(metainfo, "info"), &size);
    p = sha1_padding(p, size, &size);

    p = (unsigned char *)sha1(p, size / 4, (int *)malloc(sizeof(int) * 80));
    for(i = 0; i < 20; i++) {
        printf("%02x", p[i]);
    }
    printf("\n");

    return 0;
}
