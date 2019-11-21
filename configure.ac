#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([rbp_eval], [0.2], [wew@csse.unimelb.edu.au])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([librbp/qrels.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([m], [pow])
AC_CHECK_LIB([ssl], [MD5_Init])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h openssl/md5.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([pow strchr strdup strerror strtol gethostname getcwd getpwuid getuid])

AC_CONFIG_FILES([Makefile rbp_eval/rbp_eval.1 librbp/Makefile rbp_eval/Makefile rbp_util/Makefile stats/Makefile librbp++/Makefile librbp++/test/Makefile dcg_eval/Makefile stats/test/Makefile])
AC_OUTPUT
