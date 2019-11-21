#include "strid.h"
#include "strhash.h"
#include "util.h"
#include "array.h"

ARRAY_TYPE_DECL(strarray_t, char *);

struct strid {
    strhash_t * hash;
    unsigned ids;
    strarray_t strs;
};

strid_t * new_strid(void) {
    strid_t * si;

    si = util_malloc_or_die(sizeof(*si));
    si->hash = new_strhash();
    si->ids = 0;
    ARRAY_INIT(si->strs);
    return si;
}

void strid_delete(strid_t ** si_p) {
    strid_t * si = *si_p;

    strhash_delete(&si->hash, NULL);
    free(si->strs.elems);
    free(si);
    *si_p = NULL;
}

unsigned strid_get_id(strid_t * si, char * str) {
    strhash_data_t * data;
    char * key;
    int found;

    data = strhash_update_grab_key(si->hash, str, &found, (const char **) &key);
    if (!found) {
        data->u = si->ids++;
        ARRAY_ADD(si->strs, key);
    }
    return data->u;
}

unsigned strid_lookup_id(strid_t * si, char * str) {
    strhash_data_t data;
    int found;

    data = strhash_get(si->hash, str, &found);
    if (found) {
        return data.u;
    } else {
        return UINT_MAX;
    }
}

char * strid_get_str(strid_t * si, unsigned id) {
    if (id >= si->ids)
        return NULL;
    else
        return si->strs.elems[id];
}

unsigned strid_num_ids(strid_t * si) {
    return si->ids;
}

#ifdef STRID_MAIN

#include <assert.h>
#include <string.h>

int main(void) {
    strid_t * si;

    si = new_strid();
    assert(strid_get_id(si, "d") == 0);
    assert(strid_get_id(si, "a") == 1);
    assert(strid_get_id(si, "d") == 0);
    assert(strid_get_id(si, "c") == 2);
    assert(si->ids == 3);

    assert(strcmp(strid_get_str(si, 0), "d") == 0);
    assert(strcmp(strid_get_str(si, 1), "a") == 0);
    assert(strcmp(strid_get_str(si, 2), "c") == 0);
    assert(strid_get_str(si, 3) == NULL);

    strid_delete(&si);
    return 0;
}

#endif /* STRID_MAIN */
