#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "strhash.h"
#include "util.h"

#define INIT_TBL_SIZE 4096
#define RESIZE_LOAD 0.4 

struct strhash_elem {
    char * key;
    /*  make sure there is room to hold primitive data types directly */
    strhash_data_t data;
};

struct strhash {
    struct strhash_elem * tbl;
    unsigned tbl_size;
    unsigned elem_count;
};

struct strhash_iter {
    strhash_t * hash;
    unsigned index;
};

static unsigned int _str_hash(const char * key);

static void _strhash_expand(strhash_t * strhash);
static struct strhash_elem * _strhash_find_slot(strhash_t * strhash, 
  const char * key);

strhash_t * new_strhash() {
    strhash_t * strhash;
    unsigned i;

    strhash = util_malloc_or_die(sizeof(*strhash));
    strhash->tbl_size = INIT_TBL_SIZE;
    strhash->tbl = util_malloc_or_die(sizeof(*strhash->tbl) * INIT_TBL_SIZE);
    for (i = 0; i < strhash->tbl_size; i++) {
        strhash->tbl[i].key = NULL;
        strhash->tbl[i].data.v = NULL;
    }
    strhash->elem_count = 0;
    return strhash;
}

void strhash_foreach(strhash_t * strhash, strhash_foreach_fn_t fn,
  void * userdata) {
    int i;
    for (i = 0; i < strhash->tbl_size; i++) {
        if (strhash->tbl[i].key != NULL) {
            fn(strhash->tbl[i].key, strhash->tbl[i].data, userdata);
        }
    }
}

void strhash_delete(strhash_t ** strhash_p,
  strhash_free_data_fn_t free_data_fn) {
    strhash_t * strhash = *strhash_p;
    unsigned i;

    for (i = 0; i < strhash->tbl_size; i++) {
        if (strhash->tbl[i].key != NULL) {
            free(strhash->tbl[i].key);
            if (free_data_fn) {
                free_data_fn(strhash->tbl[i].data.v);
            }
        }
    }
    free(strhash->tbl);
    free(strhash);
    *strhash_p = NULL;
}

strhash_data_t * strhash_update(strhash_t * strhash, const char * key,
  int * found) {
    return strhash_update_grab_key(strhash, key, found, NULL);
}

strhash_data_t * strhash_update_grab_key(strhash_t * strhash, const char * key,
  int * found, const char ** hashkey) {
    struct strhash_elem * slot;

    if (strhash->elem_count > strhash->tbl_size * RESIZE_LOAD) {
        _strhash_expand(strhash);
    }
    slot = _strhash_find_slot(strhash, key);
    if (found) {
        if (slot->key)
            *found = 1;
        else
            *found = 0;
    }
    if (slot->key == NULL) {
        slot->key = util_strdup_or_die(key);
        strhash->elem_count++;
        slot->data.v = NULL;
    }
    if (hashkey != NULL) {
        *hashkey = slot->key;
    }
    return &slot->data;
}

strhash_data_t strhash_get(strhash_t * strhash, const char * key, int * found) {
    struct strhash_elem * slot;
    slot = _strhash_find_slot(strhash, key);
    if (found) {
        if (slot->key) {
            *found = 1;
        } else {
            *found = 0;
        }
    }
    return slot->data;
}

unsigned strhash_num_entries(strhash_t * strhash) {
    return strhash->elem_count;
}

strhash_iter_t * strhash_get_iter(strhash_t * strhash) {
    strhash_iter_t * iter;
    iter = util_malloc_or_die(sizeof(*iter));
    iter->hash = strhash;
    iter->index = 0;
    return iter;
}

const char * strhash_iter_next(strhash_iter_t * iter, strhash_data_t * dat) {
    unsigned i;
    strhash_t * hash = iter->hash;
    struct strhash_elem * tbl = hash->tbl;
    for (i = iter->index; i < hash->tbl_size && tbl[i].key == NULL; i++)
        ;
    iter->index = i + 1;
    if (i >= hash->tbl_size) {
        return NULL;
    } else {
        if (dat != NULL)
            *dat = tbl[i].data;
        return tbl[i].key;
    }
}

void strhash_iter_delete(strhash_iter_t ** iter_p) {
    strhash_iter_t * iter = *iter_p;
    free(iter);
    *iter_p = NULL;
}

#define LARGEPRIME 100000007

static unsigned int _str_hash(const char *str) {
    /* Author J. Zobel, April 2001. */
    char c;
    unsigned int h = LARGEPRIME;
    const char* word = str;

    for (; (c = *word) != '\0'; word++) {
        h ^= ((h << 5) + c + (h >> 2));
    }

    return h;
}

static void _strhash_expand(strhash_t * strhash) {
    unsigned old_tbl_size = strhash->tbl_size;
    struct strhash_elem * old_tbl = strhash->tbl;
    unsigned i;

    strhash->tbl_size = old_tbl_size * 2;
    strhash->tbl = util_malloc_or_die(strhash->tbl_size 
      * sizeof(*strhash->tbl));
    for (i = 0; i < strhash->tbl_size; i++) {
        strhash->tbl[i].key = NULL;
        strhash->tbl[i].data.v = NULL;
    }
    for (i = 0; i < old_tbl_size; i++) {
        if (old_tbl[i].key != NULL) {
            struct strhash_elem * slot;
            slot = _strhash_find_slot(strhash, old_tbl[i].key);
            slot->key = old_tbl[i].key;
            slot->data = old_tbl[i].data;
        }
    }
    free(old_tbl);
}

static struct strhash_elem * _strhash_find_slot(strhash_t * strhash,
  const char * key) {
    unsigned s;
    unsigned hval;

    hval = _str_hash(key);
    for (s = hval % strhash->tbl_size; ; s++) {
        struct strhash_elem * slot;

        if (s == strhash->tbl_size) {
            s = 0;
        }
        slot = &strhash->tbl[s];
        if (slot->key == NULL || strcmp(slot->key, key) == 0) {
            return slot;
        }
        assert((s + 1) % strhash->tbl_size != hval);
    }
    /* never reach here. */
}

#ifdef STRHASH_MAIN

#include <stdio.h>

#define LINE_BUF_SZ 1024

void foreach_test_fn(const char * key, strhash_data_t data,
  void * userdata) {
    unsigned * recount = userdata;
    (*recount)++;
}

int main(int argc, char ** argv) {
    char * fname;
    FILE * fp;
    char line_buf[LINE_BUF_SZ];
    unsigned elem_count = 0;
    strhash_t * strhash;
    int found;
    unsigned recount;
    strhash_iter_t * iter;
    strhash_data_t dat;
    const char * key;

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <datafile>\n", argv[0]);
        return -1;
    }
    fname = argv[1];
    fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open %s for reading\n", fname);
        return -1;
    }
    strhash = new_strhash();
    while (fgets(line_buf, LINE_BUF_SZ, fp) != NULL) {
        strhash_data_t * dat;
        dat = strhash_update(strhash, line_buf, &found);
        if (!found) {
            elem_count++;
            dat->lf = (double) strlen(line_buf);
        } else {
            assert(found == 1);
            assert(dat->lf == strlen(line_buf));
        }
    }
    assert(strhash->elem_count == elem_count);
    recount = 0;
    strhash_foreach(strhash, foreach_test_fn, &recount);
    assert(recount == elem_count);
    rewind(fp);
    while (fgets(line_buf, LINE_BUF_SZ, fp) != NULL) {
        strhash_data_t dat;
        dat = strhash_get(strhash, line_buf, &found);
        assert(found);
        assert(dat.lf == strlen(line_buf));
    }

    iter = strhash_get_iter(strhash);
    elem_count = 0;
    while ( (key = strhash_iter_next(iter, &dat)) != NULL) {
        int found;
        strhash_data_t d2;

        d2 = strhash_get(strhash, key, &found);
        assert(found == 1);
        assert(d2.lf == dat.lf);
        elem_count++;
    }
    assert(elem_count == strhash_num_entries(strhash));

    strhash_delete(&strhash, NULL);
    return 0;
}

#endif /* STRHASH_MAIN */
