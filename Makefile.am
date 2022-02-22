## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I autoconf-m4
SUBDIRS = src po icons

g_mplayerdocdir = ${docdir}
g_mplayerdoc_DATA = README COPYING

desktopdir = ${prefix}/share/applications
desktop_DATA = g-mplayer.desktop

manpagedir = ${mandir}/man1
manpage_DATA = g-mplayer.1

EXTRA_DIST = $(g_mplayerdoc_DATA) $(desktop_DATA) $(schema_DATA) $(manpage_DATA) $(prefapp_DATA) config.h.in
