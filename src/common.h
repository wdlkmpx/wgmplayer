/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * common.h
 * Copyright (C) Kevin DeKorte 2006 <kdekorte@gmail.com>
 *
 * common.h is free software.
 *
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * common.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with common.h.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include <gtk/gtk.h>
#include "gmlib.h"
#include <gio/gio.h>

#ifndef _COMMON_H
#define _COMMON_H

#define VO					"vo"
#define AUDIO_DEVICE_NAME   "audio-device-name"
#define ALSA_MIXER			"alsa-mixer"
#define VOLUME				"volume"
#define AUDIO_CHANNELS		"audio-channels"
#define USE_HARDWARE_CODECS "use-hardware-codecs"
#define USE_CRYSTALHD_CODECS "use-crystalhd-codecs"
#define USE_HW_AUDIO		"use-hw-audio"
#define SOFTVOL				"softvol"
#define VOLUME_GAIN			"volume-gain"
#define REMEMBER_SOFTVOL	"remember-softvol"
#define VOLUME_SOFTVOL		"volume-softvol"

#define CACHE_SIZE				"cache-size"
#define OSDLEVEL				"osd-level"
#define PPLEVEL					"pp-level"
#define VERBOSE					"verbose"
#define VERTICAL				"vertical"
#define FORCECACHE				"force-cache"
#define LAST_DIR				"last-dir"
#define SHOWPLAYLIST			"show-playlist"
#define SHOWDETAILS				"show-details"
#define SHOW_CONTROLS			"show-controls"
#define RESUME_MODE				"resume-mode"

#define AUDIO_LANG				"audio-lang"
#define SUBTITLE_LANG			"subtitle-lang"

#define DISABLEASS      		"disable-ass"
#define DISABLEEMBEDDEDFONTS    "disable-embeddedfonts"
#define DISABLEDEINTERLACE		"disable-deinterlace"
#define DISABLEFRAMEDROP		"disable-framedrop"
#define DISABLECONTEXTMENU		"disable-contextmenu"
#define DISABLEPAUSEONCLICK		"disable-pause-on-click"
#define DISABLEANIMATION		"disable-animation"
#define DISABLE_COVER_ART_FETCH "disable-cover-art-fetch"

#define AUTOHIDETIMEOUT		"auto-hide-timeout"
#define METADATACODEPAGE	"metadata-codepage"

#define SHOW_SUBTITLES		"show-subtitles"
#define SUBTITLEFONT		"subtitle-font"
#define SUBTITLESCALE		"subtitle-scale"
#define SUBTITLECODEPAGE	"subtitle-codepage"
#define SUBTITLECOLOR		"subtitle-color"
#define SUBTITLEOUTLINE		"subtitle-outline"
#define SUBTITLESHADOW		"subtitle-shadow"
#define SUBTITLE_MARGIN		"subtitle-margin"
#define SUBTITLE_FUZZINESS  "subtitle-fuzziness"

#define MOUSE_WHEEL_CHANGES_VOLUME "mouse-wheel-changes-volume"
#define USE_MEDIAKEYS		"use-mediakeys"
#define USE_DEFAULTPL		"use-defaultpl"
#define FULLSCREEN			"fullscreen"
#define MPLAYER_BIN			"mplayer-bin"
#define EXTRAOPTS			"extraopts"
#define MPLAYER_DVD_DEVICE  "mplayer-dvd-device"

#define REMEMBER_LOC		"remember-loc"
#define WINDOW_X			"window-x"
#define WINDOW_Y			"window-y"
#define WINDOW_HEIGHT		"window-height"
#define WINDOW_WIDTH		"window-width"
#define PANEL_POSITION	    "panel-position"
#define RESIZE_ON_NEW_MEDIA "resize-on-new-media"
#define KEEP_ON_TOP			"keep-on-top"
#define SINGLE_INSTANCE		"single-instance"
#define REPLACE_AND_PLAY	"replace-and-play"
#define BRING_TO_FRONT		"bring-to-front"
#define SHOW_NOTIFICATION   "show-notification"
#define SHOW_STATUS_ICON	"show-status-icon"

#define DISABLE_QT			"disable-qt"
#define DISABLE_REAL		"disable-real"
#define DISABLE_WMP			"disable-wmp"
#define DISABLE_DVX			"disable-dvx"
#define DISABLE_MIDI		"disable-midi"
#define DISABLE_EMBEDDING	"disable-embedding"
#define DISABLE_EMBEDDED_SCALING "disable-embedded-scaling"
#define DEBUG_LEVEL			"debug-level"

#define ACCELERATOR_KEYS	"accelerator-keys"

// JavaScript Playstates
#define STATE_UNDEFINED     0
#define STATE_STOPPED       1
#define STATE_PAUSED        2
#define STATE_PLAYING       3
#define STATE_SCANFORWARD   4
#define STATE_SCANREVERSE   5
#define STATE_BUFFERING	    6
#define STATE_WAITING       7
#define STATE_MEDIAENDED    8
#define STATE_TRANSITIONING 9
#define STATE_READY	        10
#define STATE_RECONNECTING  11


typedef enum {
    PLAYING,
    PAUSED,
    STOPPED,
    QUIT
} PLAYSTATE;

PLAYSTATE guistate;
PLAYSTATE lastguistate;

typedef struct _IdleData {
    gchar info[1024];
    gchar display_name[1024];
    gchar media_info[2048];
    gchar media_notification[2048];
    gchar af_export[1024];
    gchar url[2048];
    GMappedFile *mapped_af_export;
    gchar *device;
    gdouble cachepercent;
    gint streaming;
    gchar progress_text[1024];
    gchar vol_tooltip[128];
    gint width;
    gint height;
    gboolean videopresent;
    gboolean fullscreen;
    gboolean showcontrols;
    gdouble position;
    gdouble length;
    gdouble start_time;
    glong byte_pos;
    gchar demuxer[64];
    gchar metadata[1024];
    gboolean fromdbus;
    gboolean window_resized;
    gboolean tmpfile;
    gboolean retry_on_full_cache;

    GFile *src;
    GFile *tmp;
    GCancellable *cancel;
    WGMutex caching;
#if GLIB_CHECK_VERSION (2, 32, 0)
    GCond caching_complete;
#else
    GCond * caching_complete;
#endif

} IdleData;

IdleData *idledata;

enum {
    ITEM_COLUMN,
    DESCRIPTION_COLUMN,
    COUNT_COLUMN,
    PLAYLIST_COLUMN,
    ARTIST_COLUMN,
    ALBUM_COLUMN,
    LENGTH_COLUMN,
    LENGTH_VALUE_COLUMN,
    SUBTITLE_COLUMN,
    AUDIOFILE_COLUMN,
    COVERART_COLUMN,
    AUDIO_CODEC_COLUMN,
    VIDEO_CODEC_COLUMN,
    DEMUXER_COLUMN,
    VIDEO_WIDTH_COLUMN,
    VIDEO_HEIGHT_COLUMN,
    PLAY_ORDER_COLUMN,
    ADD_ORDER_COLUMN,
    START_COLUMN,
    END_COLUMN,
    PLAYABLE_COLUMN,
    N_COLUMNS
};

typedef enum {
    FILE_OPEN_LOCATION,
    EDIT_SCREENSHOT,
    EDIT_PREFERENCES,
    VIEW_PLAYLIST,
    VIEW_INFO,
    VIEW_DETAILS,
    VIEW_METER,
    VIEW_FULLSCREEN,
    VIEW_ASPECT,
    VIEW_SUBTITLES,
    VIEW_DECREASE_SIZE,
    VIEW_INCREASE_SIZE,
    VIEW_ANGLE,
    VIEW_CONTROLS,
    KEY_COUNT
} AcceleratorKeys;

typedef enum {
    RESUME_ALWAYS_ASK,
    RESUME_BUT_NEVER_ASK,
    DO_NOT_RESUME
} ResumeMode;

#define ACCEL_PATH_OPEN_LOCATION "<WGmplayer>/File/Open Location"
#define ACCEL_PATH_EDIT_SCREENSHOT "<WGmplayer>/Edit/Screenshot"
#define ACCEL_PATH_EDIT_PREFERENCES "<WGmplayer>/Edit/Preferences"
#define ACCEL_PATH_VIEW_PLAYLIST "<WGmplayer>/View/Playlist"
#define ACCEL_PATH_VIEW_INFO "<WGmplayer>/View/Info"
#define ACCEL_PATH_VIEW_DETAILS "<WGmplayer>/View/Details"
#define ACCEL_PATH_VIEW_METER "<WGmplayer>/View/Meter"
#define ACCEL_PATH_VIEW_FULLSCREEN "<WGmplayer>/View/Fullscreen"
#define ACCEL_PATH_VIEW_ASPECT "<WGmplayer>/View/Aspect"
#define ACCEL_PATH_VIEW_SUBTITLES "<WGmplayer>/View/Subtitles"
#define ACCEL_PATH_VIEW_DECREASE_SIZE "<WGmplayer>/View/Decrease Size"
#define ACCEL_PATH_VIEW_INCREASE_SIZE "<WGmplayer>/View/Increase Size"
#define ACCEL_PATH_VIEW_ANGLE "<WGmplayer>/View/Angle"
#define ACCEL_PATH_VIEW_CONTROLS "<WGmplayer>/View/Controls"
#define ACCEL_PATH_VIEW_NORMAL "<WGmplayer>/View/Normal"
#define ACCEL_PATH_VIEW_DOUBLE "<WGmplayer>/View/Double"

typedef struct _MetaData {
    gchar *uri;
    gchar *title;
    gchar *artist;
    gchar *album;
    gchar *length;
    gfloat length_value;
    gchar *subtitle;
    gchar *audio_codec;
    gchar *video_codec;
    gchar *demuxer;
    gint width;
    gint height;
    gboolean playable;
    gboolean resumable;
    gfloat position;
    gboolean valid;
} MetaData;

typedef struct _ButtonDef {
    gchar *uri;
    gchar *hrefid;
} ButtonDef;

#define METER_BARS 		40
gint buckets[METER_BARS];
gint max_buckets[METER_BARS];
gchar **accel_keys;
gchar **accel_keys_description;

//Define MIME for DnD
#define DRAG_NAME_0		"text/plain"
#define DRAG_INFO_0		0
#define DRAG_NAME_1		"text/uri-list"
#define DRAG_INFO_1		1
#define DRAG_NAME_2		"STRING"
#define DRAG_INFO_2		2

gint cache_size;
gboolean forcecache;
gint osdlevel;
gint pplevel;
gint streaming;
gboolean showcontrols;
gboolean showsubtitles;
gint fullscreen;
gint init_fullscreen;
gboolean videopresent;
gboolean playlist;
gint window_x;
gint window_y;
gboolean softvol;
gboolean remember_softvol;
gdouble volume_softvol;
gint volume_gain;
gint pref_volume;
gint verbose;
gint autostart;
//gint actual_x, actual_y;
//gint play_x, play_y;
gint last_window_width, last_window_height;
//gchar vm[10];
gchar *vo;
gchar *option_vo;
gboolean use_hardware_codecs;
gboolean use_crystalhd_codecs;
AudioDevice audio_device;
gchar *audio_device_name;
gint audio_channels;
gboolean use_hw_audio;
gboolean disable_deinterlace;
gboolean disable_framedrop;
gboolean disable_context_menu;
gboolean disable_fullscreen;
gboolean disable_pause_on_click;
gboolean enable_global_menu;
gint loop;
gint start_second;
gint play_length;
gint random_order;
gboolean dontplaynext;
gboolean autopause;
gchar *path;
gint js_state;
gchar *rpconsole;
gchar *rpcontrols;
gboolean control_instance;
gchar *playlistname;
gboolean ok_to_play;
gchar *subtitle;
gchar *alang;
gchar *slang;
gchar *metadata_codepage;
gboolean vertical_layout;
gboolean playlist_visible;
gboolean details_visible;
gboolean restore_controls;
gboolean update_control_flag;
//gint restore_pane;
gboolean disable_ass;
gboolean disable_embeddedfonts;
gboolean disable_animation;
gint auto_hide_timeout;
gboolean always_hide_after_timeout;
gboolean mouse_over_controls;
gchar *subtitlefont;
gdouble subtitle_scale;
gchar *subtitle_codepage;
gchar *subtitle_color;
gboolean subtitle_outline;
gboolean subtitle_shadow;
gint subtitle_margin;
gint subtitle_fuzziness;
gboolean quit_on_complete;
gchar *mplayer_bin;
gchar *mplayer_dvd_device;
gchar *option_dvd_device;
gchar *extraopts;
gboolean resize_on_new_media;
gboolean single_instance;
gboolean replace_and_play;
gboolean bring_to_front;
gboolean show_notification;
gboolean show_status_icon;
gboolean disable_cover_art_fetch;
//gboolean updating_recent;
gboolean large_buttons;
GtkIconSize button_size;
gboolean skip_fixed_allocation_on_show;
gboolean skip_fixed_allocation_on_hide;
gboolean mouse_wheel_changes_volume;
ResumeMode resume_mode;

gboolean remember_loc;
gboolean use_remember_loc;
gboolean save_loc;
gint loc_window_x;
gint loc_window_y;
gint loc_window_height;
gint loc_window_width;
gint loc_panel_position;
gboolean keep_on_top;

gboolean cancel_folder_load;

GThreadPool *retrieve_metadata_pool;
WGMutex retrieve_mutex;
WGMutex set_mutex;

gboolean use_mediakeys;
gboolean use_defaultpl;

GArray *data;
GArray *max_data;
gboolean reading_af_export;

//gboolean sub_source_file;

// layout variables
gint non_fs_width;
gint non_fs_height;

// playlist stuff
GtkListStore *playliststore;
GtkTreeIter iter;
GtkTreeIter *next_iter;
GtkTreeSelection *selection;
GtkWidget *list;

// preference store
GmPrefStore *gm_store;

// cover
gchar *cover_art_uri;

GtkWidget *create_window();
void show_window();
void present_main_window();
gint get_player_window();
void adjust_layout();
gboolean hide_buttons(void *data);

gboolean set_gui_state(void *data);
gboolean set_title_bar(void *data);
gboolean set_media_label(void *data);
gboolean set_progress_value(void *data);
gboolean set_progress_text(void *data);
gboolean set_progress_time(void *data);
gboolean set_volume_from_slider(gpointer data);
gboolean set_volume_tip(void *data);
gboolean resize_window(void *data);
gboolean set_play(void *data);
gboolean set_pause(void *data);
gboolean set_stop(void *data);
gboolean set_ff(void *data);
gboolean set_rew(void *data);
gboolean set_prev(void *data);
gboolean set_next(void *data);
gboolean set_quit(void *data);
gboolean set_kill_mplayer(void *data);
gboolean set_position(void *data);
gboolean hookup_volume(void *data);
gboolean set_volume(void *data);
gboolean set_fullscreen(void *data);
gboolean set_show_controls(void *data);
gboolean get_show_controls();
gboolean set_window_visible(void *data);
gboolean set_update_gui(void *data);
gboolean set_subtitle_visibility(void *data);
gboolean set_item_add_info(void *data);
gboolean set_metadata(gpointer data);
gboolean set_pane_position(void *data);
gboolean set_raise_window(void *data);

void remove_langs(GtkWidget * item, gpointer data);
gboolean set_new_lang_menu(gpointer data);
gboolean set_new_audio_menu(gpointer data);
gboolean make_panel_and_mouse_invisible(gpointer data);
gboolean idle_make_button(gpointer data);
gboolean set_show_seek_buttons(gpointer data);
void make_button(gchar * src, gchar * href);
void menuitem_edit_random_callback(GtkMenuItem * menuitem, void *data);

gboolean update_audio_meter(gpointer data);

void set_media_player_attributes(GtkWidget * widget);

void retrieve_metadata(gpointer data, gpointer user_data);

gchar *default_playlist;
gboolean safe_to_save_default_playlist;

gboolean async_play_iter(void *data);

GtkRecentManager *recent_manager;
void recent_manager_changed_callback(GtkRecentManager * recent_manager, gpointer data);

#endif                          /* _COMMON_H */
