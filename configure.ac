dnl Process this file with autoconf to produce a configure script.

AC_INIT([g-mplayer],[1.2])
AC_CONFIG_AUX_DIR([autoconf])
AC_CONFIG_MACRO_DIR([autoconf-m4])
AM_INIT_AUTOMAKE([-Wall subdir-objects foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
m4_ifdef([AM_MAINTAINER_MODE], [AM_MAINTAINER_MODE])

AM_CONFIG_HEADER(config.h)
AM_GNU_GETTEXT([external])

#m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
#L#T_INIT

AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CC

dnl Set gettext package name
GETTEXT_PACKAGE=g-mplayer
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

PKG_CHECK_MODULES([X11], [x11],[AC_DEFINE([X11_ENABLED],[1],[Define to 1 if X11 support is needed])],[AC_MSG_NOTICE([Compiling without X11 support])])
AC_SUBST(X11_LIBS)

AC_ARG_ENABLE(gtk3,
AS_HELP_STRING([--enable-gtk3],[enable to use gtk-3.0 instead of gtk-2.0]),
[case "${enableval}" in
  yes)  enable_gtk3=yes ;;
  no)   enable_gtk3=no ;;
esac],[])

if test "x$enable_gtk3" = "xyes" ; then
  gtk_modules="gtk+-3.0 >= 3.0.0"
else
  gtk_modules="gtk+-2.0 >= 2.14.0"
fi
PKG_CHECK_MODULES(GTK, [$gtk_modules glib-2.0 >= 2.18.0 gthread-2.0])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AC_ARG_WITH([gio],
    [AS_HELP_STRING([--with-gio],
        [Use GIO for file I/O])],
        [],
        [with_gio=check])
AS_IF([test "x$with_gio" != xno], 
    [PKG_CHECK_MODULES(GIO,[gio-2.0],[GIO_DEFINES="-DGIO_ENABLED" AC_DEFINE([GIO_ENABLED],[1],[Define to 1 if you have GIO support])],AC_MSG_NOTICE([Compiling without gio support]))])
    
PKG_CHECK_MODULES(XSCRNSAVER,[xscrnsaver],[AC_DEFINE([XSCRNSAVER_ENABLED],[1],[When defined we have direct XScrnSaver control])], AC_MSG_NOTICE([Compiling without xscrnsaver support]))
AC_SUBST(XSCRNSAVER_CFLAGS)
AC_SUBST(XSCRNSAVER_LIBS)

AC_ARG_WITH([alsa],
    [AS_HELP_STRING([--with-alsa],
        [Read volume from alsa devices])],
        [],
        [with_alsa=check])
AS_IF([test "x$with_alsa" != xno], 
    [PKG_CHECK_MODULES(ALSA, [alsa], [ALSA_DEFINES="-DHAVE_ASOUNDLIB" AC_DEFINE([HAVE_ASOUNDLIB],[1],[Define to 1 if you have asoundlib development packages])],AC_MSG_NOTICE([Compiling without asound support]))
    ])
    
AC_ARG_WITH([pulseaudio],
    [AS_HELP_STRING([--with-pulseaudio],
        [PulseAudio device and volume support])],
        [],
        [with_pulseaudio=check])
AS_IF([test "x$with_pulseaudio" != xno], 
	[PKG_CHECK_MODULES(PULSE, [libpulse >= 2.0 libpulse-mainloop-glib >= 2.0],[AC_DEFINE([HAVE_PULSEAUDIO],[1],[Define to 1 if you have PulseAudio development packages])],AC_MSG_NOTICE([Compiling without PulseAudio support]))])

AC_ARG_ENABLE(gseal,
		AS_HELP_STRING([--enable-gseal],[Enable flag for testing]),
        [case "${enableval}" in
        yes) ENABLE_GSEAL=yes ;;
        no) ENABLE_GSEAL=no ;;
        *) AC_MSG_ERROR(bad value ${enableval} for --enable-gseal) ;;
        esac],
        [ENABLE_GSEAL=no]) dnl Default value

if test x$ENABLE_GSEAL = "xyes"; then
   GTK_CFLAGS="$GTK_CFLAGS -DGSEAL_ENABLE"
   AC_SUBST(GTK_CFLAGS)
fi

#if test x$ENABLE_GTK2 = "xyes"; then
#   CPPFLAGS="$CPPFLAGS -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED -DGTK_DISABLE_SINGLE_INCLUDES"
#fi

case "$CC" in
  gcc*) CPPFLAGS="$CPPFLAGS -Wall -Wno-unused-parameter -Wno-missing-field-initializers" ;;
  *) ;;
esac

AC_CHECK_HEADERS([mntent.h])
AC_CHECK_HEADERS([sys/ucred.h])
AC_CHECK_HEADERS([sys/mount.h])
AC_CHECK_HEADERS([fstab.h])
AC_CHECK_HEADERS([signal.h])
AC_CONFIG_FILES([
Makefile
data/Makefile
src/Makefile
po/Makefile
])
AC_OUTPUT
