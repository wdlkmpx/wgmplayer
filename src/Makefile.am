## Process this file with automake to produce Makefile.in

## Created by Anjuta
SUBDIRS = 

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I../pixmaps \
	$(GTK_CFLAGS) \
    $(GIO_CFLAGS) \
	$(ALSA_CFLAGS) \
	$(PULSE_CFLAGS) \
    $(NOTIFY_CFLAGS) \
	$(XSCRNSAVER_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g\
     $(GIO_DEFINES) \
	 $(ALSA_DEFINES) \
     $(NOTIFY_DEFINES) \
     -Igmtk
     ## -DGTK_DISABLE_DEPRECATED 
     ## -DGSEAL_ENABLE

bin_PROGRAMS = gnome-mplayer 

gnome_mplayer_SOURCES = \
	../config.h \
	gmtk/gmlib.h \
	gmtk/gm_audio.c \
	gmtk/gm_audio.h \
	gmtk/gm_file.c \
	gmtk/gm_file.h \
	gmtk/gm_log.c \
	gmtk/gm_log.h \
	gmtk/gm_parse.c \
	gmtk/gm_parse.h \
	gmtk/gm_pref_store.c \
	gmtk/gm_pref_store.h \
	gmtk/gm_strfuncs.c \
	gmtk/gm_strfuncs.h \
	gmtk/gmtk.h \
	gmtk/gmtk_common.h \
	gmtk/gmtk_common.c \
	gmtk/gmtk_audio_meter.c \
	gmtk/gmtk_audio_meter.h \
	gmtk/gmtk_media_player.c \
	gmtk/gmtk_media_player.h \
	gmtk/gmtk_media_tracker.c \
	gmtk/gmtk_media_tracker.h \
	gmtk/gmtk_output_combo_box.c \
	gmtk/gmtk_output_combo_box.h \
	gui.c \
	gui.h \
	playlist.c \
	playlist.h \
	support.c \
	support.h \
	main.c \
	common.h \
	../pixmaps/gnome_mplayer.xpm \
	../pixmaps/media-playback-pause.xpm \
	../pixmaps/media-playback-start.xpm \
	../pixmaps/media-playback-stop.xpm \
	../pixmaps/media-seek-backward.xpm \
	../pixmaps/media-seek-forward.xpm \
	../pixmaps/media-skip-backward.xpm \
	../pixmaps/media-skip-forward.xpm \
	../pixmaps/view-fullscreen.xpm \
	langlist.h



gnome_mplayer_LDADD = \
    -lm \
    $(X11_LIBS) \
    $(GTK_LIBS) \
    $(GIO_LIBS) \
    $(ALSA_LIBS) \
	$(PULSE_LIBS) \
    $(NOTIFY_LIBS) \
    $(XSCRNSAVER_LIBS)

indent:
	indent -kr -l120 -i4 -nut *.c
	indent -kr -l120 -i4 -nut *.h
	rm *~
	
cppcheck:
	cppcheck -q -a -v -f -j3 .
	
valgrind:
	G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=20 --log-file=vgdump ./gnome-mplayer 
valgrind-suppress:
	G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=20 --log-file=vgdump --suppressions=$(HOME)/gtk.suppression ./gnome-mplayer 
