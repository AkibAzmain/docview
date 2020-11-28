assetsdir                    =      $(datarootdir)/docview
icons_scalabledir            =      $(datarootdir)/icons/hicolor/scalable/apps
icons48dir                   =      $(datarootdir)/icons/hicolor/48x48/apps
icons96dir                   =      $(datarootdir)/icons/hicolor/96x96/apps
icons128dir                  =      $(datarootdir)/icons/hicolor/128x128/apps
icons256dir                  =      $(datarootdir)/icons/hicolor/256x256/apps

bin_PROGRAMS                 =      docview
assets_HEADERS               =      window.ui
icons_scalable_HEADERS       =      docview.svg
icons48_HEADERS              =      docview.svg docview48x48.png
icons96_HEADERS              =      docview96x96.png
icons128_HEADERS             =      docview128x128.png
icons256_HEADERS             =      docview256x256.png

docview_SOURCES              =      docview.cpp

docview_CPPFLAGS             =      -Wall -Wextra -pedantic
docview_CPPFLAGS            +=      -std=c++17
docview_CPPFLAGS            +=      -I$(top_srcdir)/src/libdocview
docview_CPPFLAGS            +=      -DASSETS_DIR=\"$(assetsdir)\"
docview_CPPFLAGS            +=      -DICONS_DIR=\"$(iconsdir)\" \
                                    -DICONS96_DIR=\"$(icons96dir)\" \
                                    -DICONS128_DIR=\"$(icons128dir)\" \
                                    -DICONS256_DIR=\"$(icons256dir)\"

docview_LDADD                =      $(top_builddir)/src/libdocview/libdocview.la

docview_CPPFLAGS            +=      $(gtkmm_CFLAGS)
docview_CPPFLAGS            +=      $(webkit2gtk_CFLAGS)
docview_CPPFLAGS            +=      $(gtkmm_CFLAGS)
docview_CPPFLAGS            +=      $(gtk_CFLAGS)
docview_CPPFLAGS            +=      $(glibmm_CFLAGS)
docview_CPPFLAGS            +=      $(glib_CFLAGS)
docview_CPPFLAGS            +=      $(webkit2gtk_CFLAGS)
docview_CPPFLAGS            +=      $(libxmlxx_CFLAGS)

docview_LDADD               +=      $(gtkmm_LIBS)
docview_LDADD               +=      $(webkit2gtk_LIBS)
docview_LDADD               +=      $(gtkmm_LIBS)
docview_LDADD               +=      $(gtk_LIBS)
docview_LDADD               +=      $(glibmm_LIBS)
docview_LDADD               +=      $(glib_LIBS)
docview_LDADD               +=      $(webkit2gtk_LIBS)
docview_LDADD               +=      $(libxmlxx_LIBS)
