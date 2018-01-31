SUBDIRS = icons

noinst_PROGRAMS = launcher-gtk

AM_CPPFLAGS = $(DEPS_CFLAGS) $(WARN_CFLAGS) -I$(top_srcdir)
AM_LDFLAGS = -lm -lnanomsg


launcher_gtk_SOURCES = launcher-gtk.c markers.c chassis_adaptor.c radar_view.c
launcher_gtk_CPPFLAGS = $(GTK_CFLAGS) $(WARN_CFLAGS) -I$(top_srcdir)
launcher_gtk_LDADD = $(GTK_LIBS) $(DEPS_LIBS) \
		../champlain-gtk/libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la \
		../champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

EXTRA_DIST = markers.h default-rules.xml high-contrast.xml

