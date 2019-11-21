#ifndef STRHASH_H
#define STRHASH_H

/*
 *  Simple hash table with strings as keys.
 */

typedef struct strhash strhash_t;

typedef union {
    void * v;
    float f;
    double lf;
    unsigned u;
    unsigned long ul;
} strhash_data_t;

typedef struct strhash_iter strhash_iter_t;

strhash_t * new_strhash(void);

typedef void (*strhash_free_data_fn_t)(void * data);

typedef void (*strhash_foreach_fn_t)(const char * key, strhash_data_t data,
  void * userdata);

void strhash_delete(strhash_t ** strhash_p, 
  strhash_free_data_fn_t free_data_fn);

/* Retrieve the value slot for an entry, creating it if necessary.
 * FOUND reports whether the retrieve slot was for a pre-existing
 * entry (1) or a new one has been created (0). */
strhash_data_t * strhash_update(strhash_t * strhash, const char * key, 
  int * found);

/* As with strhash_update, but grab a reference to the in-dictionary
 * key. */
strhash_data_t * strhash_update_grab_key(strhash_t * strhash, const char * key, 
  int * found, const char ** hashkey);

strhash_data_t strhash_get(strhash_t * strhash, const char * key, int * found);

unsigned strhash_num_entries(strhash_t * strhash);

strhash_iter_t * strhash_get_iter(strhash_t * strhash);

const char * strhash_iter_next(strhash_iter_t * iter, strhash_data_t * dat);

void strhash_iter_delete(strhash_iter_t ** iter_p);

void strhash_foreach(strhash_t * strhash, strhash_foreach_fn_t fn,
  void * userdata);

#endif /* STRHASH_H */
