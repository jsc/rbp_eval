#ifndef DQHASH_H
#define DQHASH_H

#include "strhash.h"

/* Methods to minimally modify a strhash so that the key is a
 * <docid, qidd> pair. */

strhash_data_t * dqhash_update(strhash_t * strhash, const char * docid, 
  unsigned qidd, int * found);

strhash_data_t dqhash_get(strhash_t * strhash, const char * docid,
  unsigned qidd, int * found);

const char * dqhash_iter_next(strhash_iter_t * iter, unsigned * qidd_p,
  strhash_data_t * dat);

#endif /* DQHASH_H */
