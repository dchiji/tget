#ifndef BENCODE_H
#define BENCODE_H

#define BE_INTEGER 0
#define BE_STRING 1
#define BE_LIST 2
#define BE_DICT 3

typedef struct list {
    struct bencode {
        int type;
        union {
            unsigned int *n;
            struct string {
                unsigned char *data;
                unsigned int length;
            } *str;
            struct list *list;
            struct list *dict;
        };
    } *bencode;

    struct list *next;
} list_t;

typedef struct bencode bencode_t;
typedef struct string string_t;

int be_decode(const char *, bencode_t *);
int _be_decode_to_integer(char *, char end, int *offset);
list_t *_be_decode_to_list(const char *, int *);
list_t *_be_decode_to_dict(const char *, int *);

int be_decode_stream(FILE *, bencode_t *);
int _be_decode_to_integer_stream(FILE *, unsigned char *, char, int *);
list_t *_be_decode_to_list_stream(FILE *, int *);
list_t *_be_decode_to_dict_stream(FILE *, int *);
bencode_t *be_decode_file(const char *);

bencode_t *be_dict_lookup(bencode_t *, const char *);
unsigned char *be_encode(bencode_t *, size_t *);

#endif
