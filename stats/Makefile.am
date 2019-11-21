SUBDIRS=. test

noinst_LIBRARIES=libstat.a

bin_PROGRAMS=binomial sign wilcoxon t bootstrap kendall

LDADD=libstat.a ../librbp/librbp.a
AM_CPPFLAGS=-I$(srcdir)/../librbp -I.

binomial_CPPFLAGS=-DBINOMIAL_MAIN
sign_CPPFLAGS=-DSIGN_MAIN
wilcoxon_CPPFLAGS=-DWILCOXON_MAIN $(AM_CPPFLAGS)
t_CPPFLAGS=-DT_MAIN $(AM_CPPFLAGS)
bootstrap_CPPFLAGS=-DBOOTSTRAP_MAIN $(AM_CPPFLAGS)
kendall_CPPFLAGS=-DKENDALL_MAIN $(AM_CPPFLAGS)

libstat_a_SOURCES=binomial.c sign.c wilcoxon.c t.c bootstrap.c kendall.c \
		  tau_ap.cpp binomial.h bootstrap.h kendall.h \
		  sign.h stats.h tau_ap.h t.h wilcoxon.h
