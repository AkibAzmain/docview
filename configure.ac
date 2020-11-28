dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(docview, 0.1)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()
LT_INIT

AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CXX
AC_PROG_CC
PKG_CHECK_MODULES([gtkmm], [gtkmm-3.0 >= 3.24])
PKG_CHECK_MODULES([gtk], [gtk+-3.0 >= 3.24])
PKG_CHECK_MODULES([glibmm], [glibmm-2.4 >= 2.64])
PKG_CHECK_MODULES([glib], [glib-2.0 >= 2.64])
PKG_CHECK_MODULES([webkit2gtk], [webkit2gtk-4.0 >= 2.28])
PKG_CHECK_MODULES([libxmlxx], [libxml++-2.6 >= 2.40])

AC_CONFIG_FILES(Makefile src/Makefile src/libdocview/Makefile)
AC_OUTPUT
