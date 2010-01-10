#ifndef SHA1_H
#define SHA1_H

unsigned int *sha1(unsigned char *, unsigned int, unsigned int *);
unsigned char *sha1_padding(unsigned char *, unsigned int, size_t *);

#endif
