bin_PROGRAMS=rbp_eval
check_PROGRAMS=rbp_tester opt

dist_man_MANS=rbp_eval.1

LDADD=../librbp/librbp.a
AM_CPPFLAGS=-I$(srcdir)/../librbp

rbp_eval_SOURCES=main.c opt.c help.c trec_fmt.c desc_fmt.c \
		 opt.h desc_fmt.h help.h trec_fmt.h fmt.h

rbp_tester_SOURCES=tester.c opt.c help.c

opt_CPPFLAGS=-DOPT_MAIN $(AM_CPPFLAGS)
opt_SOURCES=opt.c help.c

EXTRA_DIST=rbp_eval.1.in $(wildcard test/data/*.spec) \
	   $(wildcard test/data/*.qrels) \
	   test/data/README.txt $(wildcard test/data/*.run)

