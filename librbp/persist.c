#include <stdlib.h>
#include "error.h"
#include "persist.h"

int parse_persist(persist_t * persist, char * spec, char * err_buf,
  unsigned err_buf_len) {
    unsigned p;
    char * cp;
    
    for (p = 0, cp = spec; *cp != '\0' && p < PERSIST_MAX_NUM; p++) {
        char * end;
        persist->p[p] = strtod(cp, &end);
        if (*end != ',' && *end != '\0') {
            if (err_buf)
                snprintf(err_buf, err_buf_len, 
                  "invalid format for element %d of persistence specification"
                  " '%s'.  Must be in form '<p1>,<p2>...'", p, spec);
            return -1;
        } else if (persist->p[p] < 0.0 || persist->p[p] > 1.0) {
            if (err_buf)
                snprintf(err_buf, err_buf_len, 
                  "element %d of persistence specification '%s' with "
                  "value %lf not in range [0.0, 1.0]", p, spec, persist->p[p]);
            return -1;
        }
        cp = end;
        if (*cp == ',')
            cp++;
    }
    if (p == PERSIST_MAX_NUM && *cp != '\0') {
        if (err_buf)
            snprintf(err_buf, err_buf_len, 
              "persistence spec '%s' exceeds maximum number of elements %d",
              spec, PERSIST_MAX_NUM);
        return -1;
    }
    persist->p_num = p;
    return 0;
}

#ifdef PERSIST_MAIN

#include <stdio.h>
#include <assert.h>

int main(void) {
    char * p1 = "0.8";
    char * p2 = "0.8,0.95,0.4";
    char * p3 = "1.1";
    char * p4 = "-0.2";
    char * p5 = "0.2,2.0";
    char * p6 = ".3;.9";
    char * p7 = ".4, .8";
    char * p8 = "";
    char * p9 = ".2,";
    persist_t p;

    error_set_log_stream(NULL);
    assert(parse_persist(&p, p1, NULL, 0) == 0);
    assert(p.p_num == 1);
    assert(p.p[0] == 0.8);

    assert(parse_persist(&p, p2, NULL, 0) == 0);
    assert(p.p_num == 3);
    assert(p.p[0] == 0.8);
    assert(p.p[1] == 0.95);
    assert(p.p[2] == 0.4);

    assert(parse_persist(&p, p3, NULL, 0) < 0);
    assert(parse_persist(&p, p4, NULL, 0) < 0);
    assert(parse_persist(&p, p5, NULL, 0) < 0);
    assert(parse_persist(&p, p6, NULL, 0) < 0);
    assert(parse_persist(&p, p7, NULL, 0) == 0);
    assert(p.p_num = 2);
    assert(p.p[0] == 0.4);
    assert(p.p[1] == 0.8);
    assert(parse_persist(&p, p8, NULL, 0) == 0);
    assert(p.p_num == 0);
    assert(parse_persist(&p, p9, NULL, 0) == 0);
    assert(p.p_num = 1);
    assert(p.p[0] == 0.2);

    return 0;
}

#endif /* PERSIST_MAIN */
