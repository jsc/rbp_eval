#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h> /* for size_t */

/**
 *  Malloc memory and die if it fails.
 */
void * util_malloc_or_die(size_t size);

void * util_realloc_or_die(void * mem, size_t size);

char * util_strdup_or_die(const char * str);

char * util_next_nonspace(char * str);

char * util_next_space(char * str);

char * util_next_word(char * str);

char * util_delim_word(char * str);

int util_is_nonneg_int(char * str);

int util_parse_cols(char * line, char ** colp, unsigned num_cols);

void util_downcase_str(char * str);

void util_ensure_array_space(void ** a, unsigned * a_sz, unsigned a_num,
  unsigned elem_size, unsigned init_size, double expand_factor);

#endif /* UTIL_H */
