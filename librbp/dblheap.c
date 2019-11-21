#include <assert.h>

#include "dblheap.h"
#include "array.h"
#include "util.h"

#define PARENT(pos) (((pos) - 1) / 2)
#define LEFT(pos) (((pos) * 2) + 1)
#define RIGHT(pos) (((pos) * 2) + 2)

struct dblheap_elem {
    double score;
    void * data;
};

ARRAY_TYPE_DECL(dblheap_elem_arr, struct dblheap_elem);

struct dblheap {
    dblheap_elem_arr heap;
    enum dblheap_order order;
};

static void _dblheap_promote(dblheap_t * heap, unsigned pos);
static void _dblheap_demote(dblheap_t * heap, unsigned pos);

dblheap_t * new_dblheap(enum dblheap_order order) {
    dblheap_t * heap = util_malloc_or_die(sizeof(*heap));

    ARRAY_INIT(heap->heap);
    heap->order = order;
    return heap;
}

void dblheap_push(dblheap_t * heap, double score, void * data) {
    struct dblheap_elem elem;
    elem.score = score;
    elem.data = data;
    ARRAY_ADD(heap->heap, elem);
    _dblheap_promote(heap, heap->heap.elem_count - 1);
}

void * dblheap_pop(dblheap_t * heap, double * score) {
    struct dblheap_elem top;
    assert(heap->heap.elem_count > 0);
    top = heap->heap.elems[0];
    heap->heap.elems[0] = heap->heap.elems[heap->heap.elem_count - 1];
    heap->heap.elem_count--;
    _dblheap_demote(heap, 0);
    *score = top.score;
    return top.data;
}

void * dblheap_peek(dblheap_t * heap, double * score) {
    struct dblheap_elem top;
    assert(heap->heap.elem_count > 0);
    top = heap->heap.elems[0];
    *score = top.score;
    return top.data;
}

unsigned dblheap_size(dblheap_t * heap) {
    return heap->heap.elem_count;
}

void dblheap_delete(dblheap_t ** heap_p) {
    dblheap_t * heap = *heap_p;
    ARRAY_DELETE(heap->heap);
    free(heap);
    *heap_p = NULL;
}

static void _dblheap_promote(dblheap_t * heap, unsigned pos) {
    unsigned parent;
    double score, parent_score;
    
    if (pos == 0) {
        return;
    }
    parent = PARENT(pos);
    score = heap->heap.elems[pos].score;
    parent_score = heap->heap.elems[parent].score;
    if ((score > parent_score && heap->order == DBLHEAP_MAX)
      || (score < parent_score && heap->order == DBLHEAP_MIN)) {
        struct dblheap_elem tmp;
        tmp = heap->heap.elems[pos];
        heap->heap.elems[pos] = heap->heap.elems[parent];
        heap->heap.elems[parent] = tmp;
        _dblheap_promote(heap, parent);
    }
}

static void _dblheap_demote(dblheap_t * heap, unsigned pos) {
    unsigned left, right, to;
    double score;
    to = pos;
    left = LEFT(pos);
    right = RIGHT(pos);
    score = heap->heap.elems[pos].score;
    if (left < heap->heap.elem_count) {
        double left_score;
        left_score = heap->heap.elems[left].score;
        if ((heap->order == DBLHEAP_MAX && left_score > score)
          || (heap->order == DBLHEAP_MIN && left_score < score)) {
            to = left;
            score = left_score;
        }
        if (right < heap->heap.elem_count) {
            double right_score;
            right_score = heap->heap.elems[right].score;
            if ((heap->order == DBLHEAP_MAX && right_score > score)
              || (heap->order == DBLHEAP_MIN && right_score < score)) {
                to = right;
            }
        }
    }
    if (to != pos) {
        struct dblheap_elem tmp;
        tmp = heap->heap.elems[pos];
        heap->heap.elems[pos] = heap->heap.elems[to];
        heap->heap.elems[to] = tmp;
        _dblheap_demote(heap, to);
    }
}

#ifdef DBLHEAP_MAIN

#include <stdio.h>

static void _dblheap_check_invariant(dblheap_t * heap) {
    unsigned pos;

    for (pos = 0; pos < heap->heap.elem_count; pos++) {
        unsigned l = LEFT(pos);
        unsigned r = RIGHT(pos);

        assert(l >= heap->heap.elem_count || 
          (heap->order == DBLHEAP_MAX && heap->heap.elems[pos].score
           > heap->heap.elems[l].score) ||
          (heap->order == DBLHEAP_MIN && heap->heap.elems[pos].score
           < heap->heap.elems[l].score));
        assert(r >= heap->heap.elem_count || 
          (heap->order == DBLHEAP_MAX && heap->heap.elems[pos].score
           > heap->heap.elems[r].score) ||
          (heap->order == DBLHEAP_MIN && heap->heap.elems[pos].score
           < heap->heap.elems[r].score));
    }
}

int main(void) {
    dblheap_t * heap;
    double vals[] = { 3.1, 2.9, 8.2, 4.3, 3.3, 9.9, 0.1, -3.2 };
    double num_vals = sizeof(vals) / sizeof(vals[0]);
    unsigned v;
    double max, prev;

    heap = new_dblheap(DBLHEAP_MAX);
    max = -10000000.0;
    for (v = 0; v < num_vals; v++) {
        double pk;
        double * pk_p;
        dblheap_push(heap, vals[v], &vals[v]);
        if (vals[v] > max) {
            max = vals[v];
        }
        pk_p = dblheap_peek(heap, &pk);
        assert(pk == max);
        assert(*pk_p == max);
        assert(dblheap_size(heap) == v + 1);
        _dblheap_check_invariant(heap);
    }
    prev = 10000000.0;
    for (v = 0; v < num_vals; v++) {
        double pp;
        double * pp_p;
        pp_p = dblheap_pop(heap, &pp);
        assert(pp < prev);
        assert(*pp_p == pp);
        prev = pp;
    }
    assert(dblheap_size(heap) == 0);
    dblheap_delete(&heap);

    heap = new_dblheap(DBLHEAP_MIN);
    max = 10000000.0;
    for (v = 0; v < num_vals; v++) {
        double pk;
        double * pk_p;
        dblheap_push(heap, vals[v], &vals[v]);
        if (vals[v] < max) {
            max = vals[v];
        }
        pk_p = dblheap_peek(heap, &pk);
        assert(pk == max);
        assert(*pk_p == max);
        assert(dblheap_size(heap) == v + 1);
        _dblheap_check_invariant(heap);
    }
    prev = -10000000.0;
    for (v = 0; v < num_vals; v++) {
        double pp;
        double * pp_p;
        pp_p = dblheap_pop(heap, &pp);
        assert(pp > prev);
        assert(*pp_p == pp);
        prev = pp;
    }
    assert(dblheap_size(heap) == 0);
    dblheap_delete(&heap);

    return 0;
}

#endif /* DBLHEAP_MAIN */
