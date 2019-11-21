noinst_LIBRARIES=librbputil.a

librbputil_a_SOURCES=docwgt.c dococcur.c dqhash.c runerr.c \
    common.h dococcur.h docwgt.h dqhash.h runerr.h

bin_PROGRAMS=minavgerr minmaxerr pooljudge reltrans
check_PROGRAMS=docwgt dococcur dqhash

minavgerr_SOURCES=minavgerr.c common.c
minmaxerr_SOURCES=minmaxerr.c common.c
pooljudge_SOURCES=pooljudge.c common.c
reltrans_SOURCES=reltrans.c

LDADD=librbputil.a ../librbp/librbp.a ../stats/libstat.a
#LDADD=-L. -L../librbp -lrbputil -lrbp
AM_CPPFLAGS=-I$(srcdir)/../librbp -I. -I$(srcdir)/../stats

docwgt_CPPFLAGS=-DDOCWGT_MAIN $(AM_CPPFLAGS)
dococcur_CPPFLAGS=-DDOCOCCUR_MAIN $(AM_CPPFLAGS)
dqhash_CPPFLAGS=-DDQHASH_MAIN $(AM_CPPFLAGS)
