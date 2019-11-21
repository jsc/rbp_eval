#ifndef DEPTH_H
#define DEPTH_H

#include <limits.h>

#define DEPTH_MAX_NUM 128
#define DEPTH_FULL UINT_MAX

/*
 *  Spec for run ranks depths to evaluate rbp at.
 */
typedef struct depth {
    unsigned d[DEPTH_MAX_NUM];
    unsigned d_num;
} depth_t;

/*
 *  Parse a depth spec.
 *
 *  The spec has the form '<p1>,<p2>...'
 */
int parse_depth(depth_t * depth, char * spec, char * err_buf, 
  unsigned err_buf_len);

#endif /* DEPTH_H */
