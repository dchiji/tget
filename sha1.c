#include <string.h>
#include <stdlib.h>

#include "sha1.h"

#define SHA1_SHIFT(word, bit)    \
    (((word) << (bit)) | ((word) >> (32 - (bit))))

#define F(t, b, c, d, x)    \
    do {    \
        if(0 <= t && t <= 19)    \
            x = (b & c) | ((~b) & d);    \
        else if(20 <= t && t <= 39)    \
            x = b ^ c ^ d;    \
        else if(40 <= t && t <= 59)    \
            x = (b & c) | (b & d) | (c & d);    \
        else    \
            x = b ^ c ^ d;    \
    }while(0);

#define K(t, x)    \
    do {    \
        if(0 <= t && t <= 19)    \
            x = 0x5A827999;    \
        else if(20 <= t && t<= 39)    \
            x = 0x6ED9EBA1;    \
        else if(40 <= t && t <= 59)    \
            x = 0x8F1BBCDC;    \
        else    \
            x = 0xCA62C1D6;    \
    }while(0);

int _sha1_copy(unsigned char *, unsigned char *, size_t);
unsigned int endian_reverse(unsigned int);

/* length = num of 32-bit integers
 * len(words)   = sizeof(int) * 16n
 * len(wordseq) = sizeof(int) * 80
 */
unsigned int *sha1(unsigned char *p, unsigned int length, unsigned int *wordseq)
{
    unsigned int h[5] =
    {
        0x67452301,
        0xEFCDAB89,
        0x98BADCFE,
        0x10325476,
        0xC3D2E1F0
    };
    unsigned int h1[5] = {0, 0, 0, 0, 0};
    unsigned int i;
    unsigned int *result;

    for(i = 0; i < length / 16; i++) {
        unsigned int j;

        for(j = 0; j < 16; j++) {
            wordseq[j] = p[i * 64 + j * 4] << 24;
            wordseq[j] |= p[i * 64 + j * 4 + 1] << 16;
            wordseq[j] |= p[i * 64 + j * 4 + 2] << 8;
            wordseq[j] |= p[i * 64 + j * 4 + 3];
        }

        for(j = 16; j < 80; j++)
            wordseq[j] = SHA1_SHIFT(wordseq[j - 3]
                    ^ wordseq[j - 8]
                    ^ wordseq[j - 14]
                    ^ wordseq[j - 16],
                    1);

        for(j = 0; j < 5; j++) h1[j] = h[j];
        for(j = 0; j < 80; j++) {
            unsigned int x;
            unsigned int y;
            unsigned int tmp;

            F(j, h1[1], h1[2], h1[3], x);
            K(j, y);
            tmp = SHA1_SHIFT(h1[0], 5) + x + h1[4] + wordseq[j] + y;

            h1[4] = h1[3];
            h1[3] = h1[2];
            h1[2] = SHA1_SHIFT(h1[1], 30);
            h1[1] = h1[0];
            h1[0] = tmp;
        }
        for(j = 0; j < 5; j++) h[j] += h1[j];
    }

    result = (unsigned int *)malloc(sizeof(int) * 5);

    *result = 1;
    if(*((unsigned char *)result) == 1) {
        for(i = 0; i < 5; i++)
            result[i] = endian_reverse(h[i]);
    } else {
        for(i = 0; i < 5; i++)
            result[i] = h[i];
    }

    return result;
}

unsigned char *sha1_padding(unsigned char *raw, unsigned int byte, size_t *size)
{
    unsigned int n = byte / 64;
    unsigned int m = byte % 64;
    unsigned char *padded;
    unsigned long long int u64_bit;

    if(64 - m < 9){
        padded = (char *)malloc(sizeof(char) * ((n + 2) * 64));
        memcpy(padded, raw, byte);
        memset(padded + byte, 0x80, 1);

        memset(padded + byte + 1, 0, (64 - m - 1) + (64 - 8));
        memset(padded + ((n + 2) * 64 - 8), 0, 4);

        u64_bit = (unsigned long long int)byte * 8;
        _sha1_copy(padded + ((n + 2) * 64 - 8) + 4, (unsigned char *)&u64_bit, 4);

        if(size != NULL) {
            *size = (n + 2) * 64;
        }
    } else {
        padded = (char *)malloc(sizeof(char) * ((n + 1) * 64));
        memcpy(padded, raw, byte);
        memset(padded + byte, 0x80, 1);

        memset(padded + byte + 1, 0, 64 - m - 9);
        u64_bit = (unsigned long long int)byte * 8;
        _sha1_copy(padded + ((n + 1) * 64 - 8), (unsigned char *)&u64_bit, 8);

        if(size != NULL) {
            *size = (n + 1) * 64;
        }
    }

    return padded;
}

int _sha1_copy(unsigned char *buf, unsigned char *p, size_t size)
{
    int n = 1;
    unsigned int i;

    if(*((char *)&n) == 1) {
        // little endian
        for(i = 0; i < size; i++) {
            buf[size - i - 1] = p[i];
        }
    } else {
        // big endian
        for(i = 0; i < size; i++) {
            buf[i] = p[i];
        }
    }
    return size;
}

unsigned int endian_reverse(unsigned int n)
{
    unsigned int m = 0;
    m |= n << 24;
    m |= ((n >> 8) << 24) >> 8;
    m |= ((n << 8) >> 24) << 8;
    m |= n >> 24;
    return m;
}
