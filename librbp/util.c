#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "util.h"
#include "error.h"

void * util_malloc_or_die(size_t size) {
    void * mem = malloc(size);
    if (mem == NULL) {
        ERROR1("out of memory trying to allocate %d bytes", size);
        exit(1);
    }
    return mem;
}

void * util_realloc_or_die(void * mem, size_t size) {
    mem = realloc(mem, size);
    if (mem == NULL) {
        ERROR1("out of memory trying to reallocate to %d bytes", size);
        exit(1);
    }
    return mem;
}

char * util_strdup_or_die(const char * str) {
    char * dup = strdup(str);
    if (dup == NULL) {
        ERROR1("out of memory trying to duplicate string of length %d bytes",
          strlen(str));
        exit(1);
    }
    return dup;
}

char * util_next_nonspace(char * str) {
    while (*str != '\0' && isspace(*str)) {
        str++;
    }
    return str;
}

char * util_next_space(char * str) {
    while (*str != '\0' && !isspace(*str)) {
        str++;
    }
    return str;
}

char * util_next_word(char * str) {
    str = util_next_space(str);
    return util_next_nonspace(str);
}

char * util_delim_word(char * str) {
    str = util_next_space(str);
    if (*str == '\0')
        return str;
    *str = '\0';
    return str + 1;
}

int util_parse_cols(char * line, char ** colp, unsigned num_cols) {
    char * pos;
    unsigned c;

    for (c = 0, pos = line; c < num_cols && *pos != '\0' && *pos != '\n'; c++) {
        pos = util_next_nonspace(pos);
        colp[c] = pos;
        /* XXX inlining here and avoiding the isspace() function call leads 
         * to a big speedup. */
        pos = util_delim_word(pos);
    }
    if (c != num_cols || (*pos != '\0' && *pos != '\n'))
        return -1;
    else
        return 0;
}

void util_downcase_str(char * str) {
    while (*str != '\0') {
        *str = tolower(*str);
        str++;
    }
}

void util_ensure_array_space(void ** a, unsigned * a_sz, unsigned a_num,
  unsigned elem_size, unsigned init_size, double expand_factor) {
    assert(init_size > 0);
    assert(expand_factor > 1.0);
    assert(a_num <= *a_sz);
    assert(elem_size > 0);
    if (a_num == *a_sz) {
        if (*a_sz == 0)
            *a_sz = init_size;
        else
            *a_sz *= expand_factor;
        *a = realloc(*a, *a_sz * elem_size);
        if (*a == NULL) {
            ERROR1("out of memory trying to reallocate to %d bytes",
              *a_sz);
            exit(1);
        }
    }
    assert(a_num < *a_sz);
}

#ifdef UTIL_MAIN

#include <assert.h>
#include <string.h>

void test_cols(void) {
    char * cols1 = "abc def ghi\n";
    char * cols2 = "abc   def   ghi\n";
    char * cols3 = "abc def ghi";
    char * cols4 = "abc def ghi jkl\n";
    char * cols5 = "abc def\n";
    char * cols6 = "";
    char * cols[3];
    int ret;
    char line_buf[1024];

    strcpy(line_buf, cols1);
    ret = util_parse_cols(line_buf, cols, 3);
    assert(ret == 0);
    assert(cols[0][0] == 'a');
    assert(cols[1][0] == 'd');
    assert(cols[2][0] == 'g');

    strcpy(line_buf, cols2);
    ret = util_parse_cols(line_buf, cols, 3);
    assert(ret == 0);
    assert(cols[0][0] == 'a');
    assert(cols[1][0] == 'd');
    assert(cols[2][0] == 'g');

    strcpy(line_buf, cols3);
    ret = util_parse_cols(line_buf, cols, 3);
    assert(ret == 0);
    assert(cols[0][0] == 'a');
    assert(cols[1][0] == 'd');
    assert(cols[2][0] == 'g');

    strcpy(line_buf, cols4);
    assert(util_parse_cols(line_buf, cols, 3) < 0);
    strcpy(line_buf, cols5);
    assert(util_parse_cols(line_buf, cols, 3) < 0);
    strcpy(line_buf, cols6);
    assert(util_parse_cols(line_buf, cols, 3) < 0);
}

int main(void) {
    char * s1 = "abcd   efgh";
    char * s2 = "abcd";
    char * s3 = "abcd\tefgh";
    char * s4 = "  a  ";
    char * s5 = "\n\tb  ";
    char * s6 = "   ";
    char * s7 = "";

    assert(util_next_nonspace(s1) == s1);
    assert(util_next_nonspace(s2) == s2);
    assert(util_next_nonspace(s3) == s3);
    assert(util_next_nonspace(s4) == strchr(s4, 'a'));
    assert(util_next_nonspace(s5) == strchr(s5, 'b'));
    assert(util_next_nonspace(s6) == strchr(s6, '\0'));
    assert(util_next_nonspace(s7) == s7);

    assert(util_next_space(s1) == strchr(s1, ' '));
    assert(util_next_space(s2) == strchr(s2, '\0'));
    assert(util_next_space(s3) == strchr(s3, '\t'));
    assert(util_next_space(s4) == s4);
    assert(util_next_space(s5) == s5);
    assert(util_next_space(s6) == s6);
    assert(util_next_space(s7) == s7);

    assert(util_next_word(s1) == strchr(s1, 'e'));
    assert(util_next_word(s2) == strchr(s2, '\0'));
    assert(util_next_word(s3) == strchr(s3, 'e'));
    assert(util_next_word(s4) == strchr(s4, 'a'));
    assert(util_next_word(s5) == strchr(s5, 'b'));
    assert(util_next_word(s6) == strchr(s6, '\0'));
    assert(util_next_word(s7) == s7);
    test_cols();
    return 0;
}

#endif /* UTIL_MAIN */
