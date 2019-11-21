bin_PROGRAMS=jlog qidspec parselist rungroups run syslist \
	     qrels rbp

# XXX sysrank doesn't compile, infrel depends on it

LDADD=../librbp++.a ../../librbp/librbp.a
CPPFLAGS=-I$(srcdir)/.. -I$(srcdir)/../..

jlog_SOURCES=jlog.cpp
qidspec_SOURCES=qidspec.cpp
parselist_SOURCES=parselist.cpp
rungroups_SOURCES=rungroups.cpp
run_SOURCES=run.cpp
syslist_SOURCES=syslist.cpp
#sysrank_SOURCES=sysrank.cpp
#infrel_SOURCES=infrel.cpp
qrels_SOURCES=qrels.cpp
rbp_SOURCES=rbp.cpp
