#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bencode.h"

int be_decode(const char *str, bencode_t *p)
{
    int offset = 0;
    unsigned char *buf;

    switch(str[0]) {
        case 'i':
            buf = (unsigned char *)strdupa(str + 1);

            p->n = (int *)malloc(sizeof(int));
            *(p->n) = _be_decode_to_integer(buf, 'e', &offset);
            p->type = BE_INTEGER;

            return 1 + offset;

        case 'l':
            p->list = _be_decode_to_list(str + 1, &offset);
            p->type = BE_LIST;

            return 1 + offset;

        case 'd':
            p->dict = _be_decode_to_dict(str + 1, &offset);
            p->type = BE_DICT;

            return 1 + offset;

        default:
            if(0x30 <= str[0] && str[0] <= 0x39) {
                buf = (unsigned char *)alloca(sizeof(char) * (strlen(str) + 1));
                strcpy(buf, str);
                int length = _be_decode_to_integer(buf, ':', &offset);

                str = str + offset;

                p->str = (string_t *)malloc(sizeof(string_t));
                p->str->data = (char *)malloc(sizeof(char) * (length + 1));
                p->str->length = length;

                memcpy(p->str->data, str, length);
                p->str->data[length] = '\0';

                p->type = BE_STRING;

                return offset + length;
            } else {
                return -1;
            }
    }
}

int _be_decode_to_integer(char *str, char end, int *offset)
{
    int i;

    for(i = 0; str[i] != end; i++);
    str[i] = '\0';

    if(offset != NULL) {
        *offset = ++i;
    }
    return atoi(str);
}

list_t *_be_decode_to_list(const char *str, int *offset)
{
    list_t *top, *p;

    if(*str == 'e') {
        return NULL;
    }

    top = p = (list_t *)malloc(sizeof(list_t));

    do {
        int n;

        p->bencode = (bencode_t *)malloc(sizeof(bencode_t));
        str += (n = be_decode(str, p->bencode));

        if(offset != NULL) {
            *offset += n;
        }

        if(*str != 'e') {
            p->next = (list_t *)malloc(sizeof(list_t));
            p = p->next;
        } else {
            p->next = NULL;
        }
    }while(*str != 'e');

    if(offset != NULL) {
        *offset += 1;    // 'e'
    }
    return top;
}

list_t *_be_decode_to_dict(const char *str, int *offset)
{
    list_t *top, *p;
    bencode_t *pair;

    if(*str == 'e') {
        return NULL;
    }

    top = p = (list_t *)malloc(sizeof(list_t));

    do {
        int n;
        int m;

        pair = (bencode_t *)malloc(sizeof(bencode_t) * 2);

        str += (n = be_decode(str, pair));
        str += (m = be_decode(str, pair + 1));

        if(offset != NULL) {
            *offset += n + m;
        }

        p->bencode = pair;
        if(*str != 'e') {
            p->next = (list_t *)malloc(sizeof(list_t));
            p = p->next;
        } else {
            p->next = NULL;
        }
    }while(*str != 'e');

    if(offset != NULL) {
        *offset += 1;
    }
    return top;
}

int be_decode_stream(FILE *stream, bencode_t *p)
{
    int offset = 0;
    unsigned char *buf;
    int ch;

    switch(ch = getc(stream)) {
        case EOF:
            return -1;

        case 'i':
            //printf("%c\n", ch);
            buf = (unsigned char *)alloca(sizeof(char) * 10);

            p->n = (int *)malloc(sizeof(int));
            *(p->n) = _be_decode_to_integer_stream(stream, buf, 'e', &offset);
            p->type = BE_INTEGER;

            return 1 + offset;

        case 'l':
            //printf("%c\n", ch);
            p->list = _be_decode_to_list_stream(stream, &offset);
            p->type = BE_LIST;

            return 1 + offset;

        case 'd':
            //printf("%c\n", ch);
            p->dict = _be_decode_to_dict_stream(stream, &offset);
            p->type = BE_DICT;

            return 1 + offset;

        default:
            if(0x30 <= ch && ch <= 0x39) {
                int i;
                int length;

                fseek(stream, -1L, SEEK_CUR);
                //printf("%c\n", ch);

                buf = (unsigned char *)alloca(sizeof(char) * 20);
                length = _be_decode_to_integer_stream(stream, buf, ':', &offset);
                //printf("length=%d\n", length);

                p->str         = (string_t *)malloc(sizeof(string_t));
                p->str->data   = (char *)malloc(sizeof(char) * length + 1);
                p->str->length = length;

                for(i = 0; i < length; i++) {
                    p->str->data[i] = getc(stream);
                }

                p->str->data[length] = '\0';
                p->type = BE_STRING;

                return offset + length;
            } else {
                return -1;
            }
    }
}

int _be_decode_to_integer_stream(FILE *stream, unsigned char *buf, char end, int *offset)
{
    int i;
    int ch;

    for(i = 0; (ch = getc(stream)) != end; i++) {
        buf[i] = (unsigned char)ch;
    }
    buf[i] = '\0';

    if(offset != NULL) {
        *offset = ++i;
    }

    return atoi(buf);
}

list_t *_be_decode_to_list_stream(FILE *stream, int *offset)
{
    int ch;
    list_t *top, *p;

    if(getc(stream) == 'e') {
        return NULL;
    }
    fseek(stream, -1L, SEEK_CUR);

    top = p = (list_t *)malloc(sizeof(list_t));

    do {
        int n;

        p->bencode = (bencode_t *)malloc(sizeof(bencode_t));
        n = be_decode_stream(stream, p->bencode);

        if(offset != NULL) {
            *offset += n;
        }

        if((ch = getc(stream)) != 'e') {
            p->next = (list_t *)malloc(sizeof(list_t));
            p = p->next;
        } else {
            p->next = NULL;
        }
        fseek(stream, -1L, SEEK_CUR);
    }while(ch != 'e');

    if(offset != NULL) {
        *offset += 1;    // 'e'
    }

    getc(stream);
    return top;
}

list_t *_be_decode_to_dict_stream(FILE *stream, int *offset)
{
    int ch;
    list_t *top, *p;
    bencode_t *pair;

    if(getc(stream) == 'e') {
        return NULL;
    }
    fseek(stream, -1L, SEEK_CUR);

    top = p = (list_t *)malloc(sizeof(list_t));

    do {
        int n;
        int m;

        pair = (bencode_t *)malloc(sizeof(bencode_t) * 2);

        n = be_decode_stream(stream, pair);
        m = be_decode_stream(stream, pair + 1);

        if(offset != NULL) {
            *offset += n + m;
        }

        p->bencode = pair;
        if((ch = getc(stream)) != 'e') {
            p->next = (list_t *)malloc(sizeof(list_t));
            p = p->next;
        } else {
            p->next = NULL;
        }
        fseek(stream, -1L, SEEK_CUR);
    }while(ch != 'e');

    if(offset != NULL) {
        *offset += 1;
    }

    getc(stream);
    return top;
}

bencode_t *be_decode_file(const char *filename)
{
    FILE *fp;
    bencode_t *metainfo = (bencode_t *)malloc(sizeof(bencode_t));

    if((fp = fopen(filename, "rb")) == NULL) {
        //printf("Not found: %s\n", filename);
        return NULL;
    }
    be_decode_stream(fp, metainfo);
    fclose(fp);

    return metainfo;
}

bencode_t *be_dict_lookup(bencode_t *be_dict, const char *key)
{
    list_t *dict = be_dict->dict;

    if(be_dict->type != BE_DICT ||
            be_dict->dict == NULL) {
        return NULL;
    }

    while(strcmp(dict->bencode[0].str->data, key)) {
        if(!(dict = dict->next)) break;
    }

    return dict->bencode + 1;
}

unsigned char *be_encode(bencode_t *bencode, size_t *size)
{
    unsigned char *buf;
    unsigned int length;    // size of buf without '\0'

    switch(bencode->type) {
        case BE_INTEGER:
            //printf("int\t%d\n", *(bencode->n));

            length = (unsigned int)log10((double)*bencode->n) + 1;
            //printf("length=%d\n", length);
            buf = (unsigned char *)malloc(sizeof(char) * (length + 3));    // 3: 'i', 'e', '\0'

            sprintf(buf, "i%de", *bencode->n);
            length += 2;    // 2: 'e', 'i'
            buf[length] = '\0';
            break;

        case BE_STRING:
            //printf("str\n");
            {
                int digits;

                length = bencode->str->length;
                digits = (int)log10((double)length) + 1;
                buf = (unsigned char *)malloc(sizeof(char) * (length + digits + 2));    // 2: ':', '\0'

                sprintf(buf, "%d:", length);
                memcpy(buf + digits + 1, bencode->str->data, length);    // 1: ':'a

                length += digits + 1;    // 1: ':'
                buf[length] = '\0';
            }
            break;

        case BE_LIST:
            //printf("list\n");
            {
                list_t *list;
                unsigned char *buf1;
                size_t buf_size  = 512;    // current size of allocated memory for buf
                size_t buf1_size = 0;

                length = 0;

                list = bencode->list;
                buf = (unsigned char *)malloc(sizeof(char) * buf_size);

                *buf = 'l';
                length++;

                while(list) {
                    buf1 = be_encode(list->bencode, &buf1_size);

                    while(length + buf1_size + 1 > buf_size) {    // 1: '\0'
                        buf_size *= 2;
                        buf = (unsigned char*)realloc(buf, sizeof(char) * buf_size);
                    }

                    memcpy(buf + length, buf1, buf1_size);
                    length += buf1_size;

                    free(buf1);

                    list = list->next;
                }

                length += 1;    // 1: 'e'
                buf = (unsigned char *)realloc(buf, sizeof(char) * (length + 1));    // 1:'\0'
                buf[length - 1] = 'e';
                buf[length] = '\0';

                //printf("list\tend:%d\t\t[%s]\n", length, buf);
            }
            break;

        case BE_DICT:
            //printf("dict\n");
            {
                list_t *dict;
                unsigned char *buf1;
                unsigned char *buf2;
                size_t buf_size  = 512;    // current size of allocated memory for buf
                size_t buf1_size = 0;
                size_t buf2_size = 0;

                length = 0;

                dict = bencode->dict;
                buf = (unsigned char *)malloc(sizeof(char) * buf_size);

                *buf = 'd';
                length++;

                while(dict) {
                    buf1 = be_encode(dict->bencode + 0, &buf1_size);
                    //printf("dict\tbuf1\t%s\n", buf1);
                    buf2 = be_encode(dict->bencode + 1, &buf2_size);
                    //printf("dict\tbuf2\t%s\n", buf2);

                    while(length + buf1_size + buf2_size + 1 > buf_size) {    // 1: '\0'
                        buf_size *= 2;
                        buf = (unsigned char*)realloc(buf, sizeof(char) * buf_size);
                    }

                    memcpy(buf + length, buf1, buf1_size);
                    length += buf1_size;
                    memcpy(buf + length, buf2, buf2_size);
                    length += buf2_size;

                    free(buf1);
                    free(buf2);

                    dict = dict->next;
                }

                length += 1;    // 1: 'e'
                buf = (unsigned char *)realloc(buf, sizeof(char) * (length + 1));    // 1: '\0'
                buf[length - 1] = 'e';
                buf[length] = '\0';

                //printf("dict\tend:%d\t\t{%s}\n", length, buf);
            }
            break;

        default:
            //printf("\n\n\n\ndefault\t%d\n\n\n\n", bencode->type);
            return NULL;
    }

    if(size != NULL) {
        *size = length;
    }
    return buf;
}
