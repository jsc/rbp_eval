#ifndef PERSIST_H
#define PERSIST_H

#define PERSIST_MAX_NUM 128

/*
 *  Specifications for user persistence ($p$ in the rbp paper).
 */
typedef struct persist {
    double p[PERSIST_MAX_NUM];
    unsigned p_num;
} persist_t;

/*
 *  Parse a persistence spec.
 *
 *  The spec has the form '<p1>,<p2>...'
 */
int parse_persist(persist_t * persist, char * spec, char * err_buf,
  unsigned err_buf_len);

#endif /* PERSIST_H */
