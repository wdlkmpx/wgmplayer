## Process this file with automake to produce Makefile.in
## Created by Anjuta

ACLOCAL_AMFLAGS = -I m4
SUBDIRS = gmtk src po icons

gnome_mplayerdocdir = ${docdir}
gnome_mplayerdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS \
	DOCS/keyboard_shortcuts.txt

desktopdir = ${prefix}/share/applications
desktop_DATA = gnome-mplayer.desktop

prefappdir = ${prefix}/share/gnome-control-center/default-apps
prefapp_DATA = gnome-mplayer.xml


manpagedir = @mandir@/man1
manpage_DATA = gnome-mplayer.1

EXTRA_DIST = $(gnome_mplayerdoc_DATA) $(desktop_DATA) $(schema_DATA) $(manpage_DATA) $(prefapp_DATA) config.h.in

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		$(gtk_update_icon_cache); \
	fi

