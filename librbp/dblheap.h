#ifndef DBLHEAP_H
#define DBLHEAP_H

/*
 *  Heap where objects are scored by a double.
 */

enum dblheap_order {
    DBLHEAP_MIN,
    DBLHEAP_MAX
};

typedef struct dblheap dblheap_t;

dblheap_t * new_dblheap(enum dblheap_order);

void dblheap_push(dblheap_t * heap, double score, void * data);

void * dblheap_pop(dblheap_t * heap, double * score);

void * dblheap_peek(dblheap_t * heap, double * score);

unsigned dblheap_size(dblheap_t * heap);

void dblheap_delete(dblheap_t ** heap_p);

#endif /* DBLHEAP_H */
