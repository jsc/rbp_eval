#ifndef STRID_H
#define STRID_H

#include <limits.h>

/* Map from a string to a sequential integer id, and vice versa. 
 * The ids start counting from 0, and are allocated sequential. */

typedef struct strid strid_t;

strid_t * new_strid(void);

/*
 *  Get the id of a string.  If none has been assigned, create a new id.
 */
unsigned strid_get_id(strid_t * si, char * str);

/*
 *  Look up the id of a string.  If none has been assigned, return UINT_MAX.
 */
unsigned strid_lookup_id(strid_t * si, char * str);

char * strid_get_str(strid_t * si, unsigned id);

unsigned strid_num_ids(strid_t * si);

void strid_delete(strid_t ** si_p);


#endif /* STRID_H */
