#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../bencode.h"

int main()
{
    int n;
    bencode_t *p = (bencode_t *)malloc(sizeof(bencode_t));
    char *buf;

    n = be_decode("12:hello, world", p);
    printf("n=%d\np->str = %s\n\n", n, p->str->data);

    n = be_decode("l12:hello, worldi32ee", p);
    printf("n=%d\np->list->bencode->str=%s\np->list->next->bencode->n=%d\n\n",
            n,
            p->list->bencode->str->data,
            *(p->list->next->bencode->n));

    if((n = be_decode("d12:hello, worldi32ee", p)) == -1) {
        printf("error\n");
    } else {
        printf("n=%d\np->dict->bencode[0].str=%s\np->dict->bencode[1].n=%d\n",
                n,
                p->dict->bencode[0].str->data,
                *(p->dict->bencode[1].n));
    }
    printf("p[\"hello, world\"]=%d\n\n", *(be_dict_lookup(p, "hello, world")->n));

    be_decode("d5:hellod4:hogei32ee12:hello, worldli1ei2ei3eee", p);
    buf = be_encode(p, NULL);
    printf("%s\n", buf);

    return 0;
}
