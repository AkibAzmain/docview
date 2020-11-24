dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(docview, 1.0)


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_PROG_CXX
PKG_CHECK_MODULES([gtkmm], [gtkmm-3.0 >= 3.24])
PKG_CHECK_MODULES([gtk], [gtk+-3.0 >= 3.24])
PKG_CHECK_MODULES([glibmm], [glibmm-2.4 >= 2.64])
PKG_CHECK_MODULES([glib], [glib-2.0 >= 2.64])
PKG_CHECK_MODULES([webkit2gtk], [webkit2gtk-4.0 >= 2.28])

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

