#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>  /* long opts */

#include "args.h"

void print_options(FILE * stream, struct option * long_options) {
    unsigned o;
    fprintf(stream, "Options are: ");   
    for (o = 0; long_options[o].name != NULL; o++) {
        fprintf(stream, "%s", long_options[o].name);
        if (long_options[o].has_arg)    
            fprintf(stream, "=");
        if (long_options[o + 1].name != NULL)
            fprintf(stream, ", ");
    }
    fprintf(stream, "\n"); 
}

void print_usage(FILE * stream, const char * usage, const char * progname,
  struct option * long_options) {
    fprintf(stream, usage, progname);
    print_options(stream, long_options);
}

int parse_key_val(char ** s, char ** key, char ** val) {
    char * cp;
    assert(*s != NULL);
    assert(**s != '\0');
    cp = strchr(*s, '=');
    if (cp == NULL) {
        return -1;
    }
    *key = *s;
    *cp = '\0';
    cp++;
    *val = cp;
    cp = strchr(cp, ',');
    if (cp) {
        *cp = '\0';
        *s = cp + 1;
    } else {
        *s = NULL;
    }
    return 0;
}
