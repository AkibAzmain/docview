assetsdir            =      $(datarootdir)/docview

bin_PROGRAMS         =      docview
assets_HEADERS       =      window.ui

docview_SOURCES      =      docview.cpp

docview_CPPFLAGS     =      -Wall -Wextra -pedantic
docview_CPPFLAGS    +=      -std=c++17
docview_CPPFLAGS    +=      -I$(top_srcdir)/src/libdocview
docview_CPPFLAGS    +=      -DASSETS_DIR=\"$(assetsdir)\"

docview_LDADD        =      $(top_builddir)/src/libdocview/libdocview.la

docview_CPPFLAGS    +=      $(gtkmm_CFLAGS)
docview_CPPFLAGS    +=      $(webkit2gtk_CFLAGS)
docview_CPPFLAGS    +=      $(gtkmm_CFLAGS)
docview_CPPFLAGS    +=      $(gtk_CFLAGS)
docview_CPPFLAGS    +=      $(glibmm_CFLAGS)
docview_CPPFLAGS    +=      $(glib_CFLAGS)
docview_CPPFLAGS    +=      $(webkit2gtk_CFLAGS)
docview_CPPFLAGS    +=      $(libxmlxx_CFLAGS)

docview_LDADD       +=      $(gtkmm_LIBS)
docview_LDADD       +=      $(webkit2gtk_LIBS)
docview_LDADD       +=      $(gtkmm_LIBS)
docview_LDADD       +=      $(gtk_LIBS)
docview_LDADD       +=      $(glibmm_LIBS)
docview_LDADD       +=      $(glib_LIBS)
docview_LDADD       +=      $(webkit2gtk_LIBS)
docview_LDADD       +=      $(libxmlxx_LIBS)
