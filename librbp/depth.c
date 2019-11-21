#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include "error.h"
#include "depth.h"

int unsigned_cmp(const void * va, const void * vb) {
    const unsigned * a = va;
    const unsigned * b = vb;
    if (*a > *b)
        return 1;
    else if (*b > *a)
        return -1;
    else
        return 0;
}

int parse_depth(depth_t * depth, char * spec, char * err_buf, 
  unsigned err_buf_len) {
    unsigned d;
    char * cp;

    for (d = 0, cp = spec; *cp != '\0' && d < DEPTH_MAX_NUM; d++) {
        char * end;
        long dval;
        
        /* strtoul, unaccountably, performs automatic negation on
         * negative values.  This is not what we want. */
        dval = strtol(cp, &end, 10);
        if (*end != ',' && *end != '\0') {
            if (err_buf)
                snprintf(err_buf, err_buf_len, 
                  "invalid format for element %u of depth specification "
                  "'%s'.  Must be in form '<d1>,<d2>...'", d, spec);
            return -1;
        }
        if (dval < 0 || dval == LONG_MAX) {
            if (err_buf)
                snprintf(err_buf, err_buf_len, 
                  "element %u of depth specificaton '%s' with value "
                  "%ld not in range [1, LONG_MAX)", d, spec, dval);
            return -1;
        }
        if (dval == 0)
            dval = DEPTH_FULL;
        depth->d[d] = dval;
        if (d > 0 && depth->d[d] <= depth->d[d - 1]) {
            if (err_buf)
                snprintf(err_buf, err_buf_len, 
                  "depths in specification '%s' not in strictly ascending "
                  "order", spec);
            return -1;
        }
        cp = end;
        if (*cp == ',')
            cp++;
    }
    if (d == DEPTH_MAX_NUM && *cp != '\0') {
        if (err_buf)
            snprintf(err_buf, err_buf_len, 
              "depth spec '%s' exceeds maximum number of elements %d",
              spec, DEPTH_MAX_NUM);
        return -1;
    }
    depth->d_num = d;
    return 0;
}

#ifdef DEPTH_MAIN

#include <stdio.h>
#include <assert.h>

int main(void) {
    char * d1 = "10";
    char * d2 = "10,100,1000";
    char * d3 = "-1";
    char * d4 = "10,1";
    char * d5 = "1;2";
    char * d6 = "10, 100";
    char * d7 = "";
    char * d8 = "10,";
    char * d9 = "20,10";
    char * d10 = "20,20";
    char * d11 = "10,20,15";
    char * d12 = "0";  /* valid: 0 means "unlimited" */
    char * d13 = "10,0";  /* valid: 0 means "unlimited" */
    depth_t d;

    assert(parse_depth(&d, d1, NULL, 0) == 0);
    assert(d.d_num = 1);
    assert(d.d[0] == 10);

    assert(parse_depth(&d, d2, NULL, 0) == 0);
    assert(d.d_num = 3);
    assert(d.d[0] == 10);
    assert(d.d[1] == 100);
    assert(d.d[2] == 1000);

    assert(parse_depth(&d, d3, NULL, 0) == -1);
    assert(parse_depth(&d, d4, NULL, 0) == -1);
    assert(parse_depth(&d, d5, NULL, 0) == -1);
    assert(parse_depth(&d, d6, NULL, 0) == 0);
    assert(d.d_num == 2);
    assert(d.d[0] == 10);
    assert(d.d[1] == 100);
    assert(parse_depth(&d, d7, NULL, 0) == 0);
    assert(d.d_num == 0);
    assert(parse_depth(&d, d8, NULL, 0) == 0);
    assert(d.d_num == 1);
    assert(d.d[0] == 10);

    assert(parse_depth(&d, d9, NULL, 0) == -1);
    assert(parse_depth(&d, d10, NULL, 0) == -1);
    assert(parse_depth(&d, d11, NULL, 0) == -1);
    assert(parse_depth(&d, d12, NULL, 0) == 0);
    assert(parse_depth(&d, d13, NULL, 0) == 0);

    return 0;
}

#endif /* DEPTH_MAIN */
