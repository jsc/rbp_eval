#include <stdio.h>
#include <stdlib.h>
#include "dqhash.h"

#define MAX_DOCID_LEN 256
#define MAX_QID_LEN 8

#define MAX_KEY_LEN (MAX_DOCID_LEN + 1 + MAX_QID_LEN)

strhash_data_t * dqhash_update(strhash_t * strhash, const char * key, 
  unsigned qidd, int * found) {
    char buf[MAX_KEY_LEN + 1];
    snprintf(buf, MAX_KEY_LEN + 1, "%0*d %s", MAX_QID_LEN, qidd, key);
    return strhash_update(strhash, buf, found);
}

strhash_data_t dqhash_get(strhash_t * strhash, const char * key,
  unsigned qidd, int * found) {
    char buf[MAX_KEY_LEN + 1];
    snprintf(buf, MAX_KEY_LEN + 1, "%0*d %s", MAX_QID_LEN, qidd, key);
    return strhash_get(strhash, buf, found);
}

const char * dqhash_iter_next(strhash_iter_t * iter, unsigned * qidd_p,
  strhash_data_t * dat) {
    const char * key = strhash_iter_next(iter, dat);
    if (key == NULL)
        return NULL;
    *qidd_p = atoi(key);
    return key + MAX_QID_LEN + 1;
}

#ifdef DQHASH_MAIN

#include <assert.h>
#include <limits.h>
#include <string.h>

int main(void) {
    strhash_t * hash;
    int found;
    strhash_iter_t * iter;
    const char * docid;
    unsigned qidd;
    strhash_data_t dat;
    struct {
        char * key;
        unsigned qidd;
        unsigned seen;
    } testdat[] = { {"foo", 23, 0}, {"foo", 22, 0}, {"bar", 23, 0},
        { NULL, UINT_MAX, UINT_MAX } };
    int i;

    hash = new_strhash();
    for (i = 0; testdat[i].key != NULL; i++) {
        dqhash_update(hash, testdat[i].key, testdat[i].qidd, &found);
        assert(found == 0);
    }
    for (i = 0; testdat[i].key != NULL; i++) {
        dqhash_update(hash, testdat[i].key, testdat[i].qidd, &found);
        assert(found == 1);
    }

    assert(strhash_num_entries(hash) == 3);
    for (i = 0; testdat[i].key != NULL; i++) {
        dqhash_get(hash, testdat[i].key, testdat[i].qidd, &found);
        assert(found == 1);
    }
    dqhash_get(hash, "foo", 21, &found);
    assert(found == 0);

    iter = strhash_get_iter(hash);
    while ( (docid = dqhash_iter_next(iter, &qidd, &dat)) != NULL) {
        for (i = 0; testdat[i].key != NULL; i++) {
            if (strcmp(testdat[i].key, docid) == 0 && 
              testdat[i].qidd == qidd) {
                testdat[i].seen = 1;
                break;
            }
        }
        assert(testdat[i].key != NULL);
    }
    for (i = 0; testdat[i].key != NULL; i++) {
        assert(testdat[i].seen == 1);
    }
    strhash_iter_delete(&iter);
    strhash_delete(&hash, NULL);
    return 0;
}

#endif /* DQHASH_MAIN */
