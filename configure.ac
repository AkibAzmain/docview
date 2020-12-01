dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(docview, 0.9)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()
LT_INIT

AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CXX
AC_PROG_CC
PKG_CHECK_MODULES([gtkmm], [gtkmm-3.0 >= 3.24])
PKG_CHECK_MODULES([webkit2gtk], [webkit2gtk-4.0 >= 2.28])
PKG_CHECK_MODULES([libxmlxx], [libxml++-2.6 >= 2.40])

AC_CONFIG_FILES(Makefile src/Makefile src/libdocview/Makefile)
AC_OUTPUT
