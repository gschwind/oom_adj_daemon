AC_INIT([page], [1.9.6])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/main.cxx])
AC_CONFIG_HEADERS([src/config.hxx])

dnl Check for required programs
AC_PROG_CXX
AC_PROG_LIBTOOL

# check for C++11
AX_CXX_COMPILE_STDCXX_11

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  ])
AC_OUTPUT
