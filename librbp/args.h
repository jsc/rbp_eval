#ifndef ARGS_H
#define ARGS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>  /* long opts */

void print_options(FILE * stream, struct option * long_options);

void print_usage(FILE * stream, const char * usage, const char * progname,
  struct option * long_options);

int parse_key_val(char ** s, char ** key, char ** val);

#endif /* ARGS_H */
