## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(GMTK_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = libgmlib.la libgmtk.la 

libgmtk_la_SOURCES = \
	gmtk.h \
	gmtk_common.h \
	gmtk_common.c \
	gmtk_audio_meter.c \
	gmtk_audio_meter.h \
	gmtk_media_player.c \
	gmtk_media_player.h \
	gmtk_media_tracker.c \
	gmtk_media_tracker.h \
	gmtk_output_combo_box.c \
	gmtk_output_combo_box.h \
    libgmlib.la

libgmtk_la_CFLAGS = -fPIC \
	$(GTK_CFLAGS) \
	$(ALSA_CFLAGS) \
	$(PULSE_CFLAGS)	

libgmtk_la_LDFLAGS = -L.libs \
	-version-number $(LIB_VERSION) \
	-no-undefined \
	$(AM_LDFLAGS)

libgmtk_la_LIBADD = \
    libgmlib.la \
    $(GTK_LIBS) \
    $(ALSA_LIBS) \
    $(PULSE_LIBS)
    

libgmlib_la_SOURCES = \
	gm_common.c \
	gm_common.h \
	gmlib.h \
	gm_audio.c \
	gm_audio.h \
	gm_file.c \
	gm_file.h \
	gm_log.c \
	gm_log.h \
	gm_parse.c \
	gm_parse.h \
	gm_pref_store.c \
	gm_pref_store.h \
	gm_strfuncs.c \
	gm_strfuncs.h 
	
libgmlib_la_CFLAGS = -fPIC \
	$(GLIB_CFLAGS) \
	$(GCONF_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GCONF_DEFINES) \
	$(GIO_DEFINES) \
	$(GLIB2_26_DEFINES) \
	$(ALSA_CFLAGS) \
	$(PULSE_CFLAGS)	

libgmlib_la_LDFLAGS = \
	-version-number $(LIB_VERSION) \
	-no-undefined \
	$(AM_LDFLAGS)

libgmlib_la_LIBADD = \
    $(LIBS) \
	$(GLIB_LIBS) \
	$(GCONF_LIBS) \
	$(GIO_LIBS) \
	$(ALSA_LIBS) \
	$(PULSE_LIBS)	


includedir = $(prefix)/include/gmtk
include_HEADERS = \
	gmlib.h \
	gm_common.h \
	gm_audio.h \
	gm_file.h \
	gm_log.h \
	gm_parse.h \
	gm_pref_store.h \
	gm_strfuncs.h \
	gmtk.h \
	gmtk_audio_meter.h \
	gmtk_media_player.h \
	gmtk_media_tracker.h \
	gmtk_output_combo_box.h \
	gmtk_common.h

indent:
	indent -kr -l120 -i4 -nut *.c
	indent -kr -l120 -i4 -nut *.h
	rm *~
	
cppcheck:
	cppcheck -q -a -v -f -j3 .

