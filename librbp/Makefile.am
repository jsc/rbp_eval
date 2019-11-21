#noinst_LIBRARIES=librbp.a
lib_LIBRARIES=librbp.a

#include_HEADERS=*.h

check_PROGRAMS=persist strhash util qrels run qdocs depth rbp array strid \
	       dblheap

LDADD=../librbp/librbp.a
AM_CPPFLAGS=-I../librbp

persist_CPPFLAGS=-DPERSIST_MAIN
strhash_CPPFLAGS=-DSTRHASH_MAIN
util_CPPFLAGS=-DUTIL_MAIN
qrels_CPPFLAGS=-DQRELS_MAIN
run_CPPFLAGS=-DRUN_MAIN
qdocs_CPPFLAGS=-DQDOCS_MAIN
depth_CPPFLAGS=-DDEPTH_MAIN
rbp_CPPFLAGS=-DRBP_MAIN
array_CPPFLAGS=-DARRAY_MAIN
strid_CPPFLAGS=-DSTRID_MAIN
dblheap_CPPFLAGS=-DDBLHEAP_MAIN

librbp_a_SOURCES=depth.c error.c persist.c qdocs.c qrels.c rbp.c \
    res.c run.c strhash.c util.c strid.c dblheap.c futil.c args.c \
    $(wildcard *.h)
