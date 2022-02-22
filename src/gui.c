/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gui.c
 * Copyright (C) Kevin DeKorte 2006 <kdekorte@gmail.com>
 *
 * gui.c is free software.
 *
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * gui.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gui.c.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gui.h"
#include "support.h"
#include "common.h"
#include "../data/player.xpm"
#include "langlist.h"

static GtkWidget *fs_window;
static GtkWidget *fs_controls;

static GtkWidget *menubar;
static GtkMenuItem *menuitem_file;
static GtkMenu *menu_file;
static GtkMenuItem *menuitem_file_open;
static GtkMenuItem *menuitem_file_open_folder;
static GtkMenuItem *menuitem_file_open_location;
static GtkMenuItem *menuitem_file_disc;
static GtkMenu *menu_file_disc;
static GtkMenuItem *menuitem_file_open_acd;
static GtkMenuItem *menuitem_file_open_sep1;
static GtkMenuItem *menuitem_file_open_dvdnav;
static GtkMenuItem *menuitem_file_open_dvdnav_folder;
static GtkMenuItem *menuitem_file_open_dvdnav_iso;
static GtkMenuItem *menuitem_file_open_sep2;
static GtkMenuItem *menuitem_file_open_vcd;

static GtkMenuItem *menuitem_file_tv;
static GtkMenu *menu_file_tv;
static GtkMenuItem *menuitem_file_open_atv;
static GtkMenuItem *menuitem_file_open_dtv;
static GtkMenuItem *menuitem_file_recent;
static GtkWidget *menuitem_file_recent_items;
static GtkMenuItem *menuitem_file_sep2;
static GtkMenuItem *menuitem_file_quit;

static GtkMenuItem *menuitem_edit;
static GtkMenu *menu_edit;
static GtkMenuItem *menuitem_edit_switch_audio;
static GtkMenuItem *menuitem_edit_set_audiofile;
static GtkMenuItem *menuitem_edit_set_subtitle;
static GtkMenuItem *menuitem_edit_take_screenshot;
static GtkMenuItem *menuitem_edit_sep1;
static GtkMenuItem *menuitem_edit_config;
static GtkMenuItem *menuitem_help;
static GtkMenuItem *menuitem_view;
static GtkMenu *menu_view;
static GtkMenuItem *menuitem_view_details;
static GtkMenuItem *menuitem_view_sep0;
static GtkMenuItem *menuitem_view_fullscreen;
static GtkMenuItem *menuitem_view_sep1;
static GtkMenuItem *menuitem_view_onetoone;
static GtkMenuItem *menuitem_view_twotoone;
static GtkMenuItem *menuitem_view_onetotwo;
static GtkMenuItem *menuitem_view_onetoonepointfive;
static GtkMenuItem *menuitem_view_aspect;
static GtkMenu *menu_view_aspect;
static GtkMenuItem *menuitem_view_aspect_default;
static GtkMenuItem *menuitem_view_aspect_four_three;
static GtkMenuItem *menuitem_view_aspect_sixteen_nine;
static GtkMenuItem *menuitem_view_aspect_sixteen_ten;
static GtkMenuItem *menuitem_view_aspect_anamorphic;
static GtkMenuItem *menuitem_view_aspect_follow_window;
static GtkMenuItem *menuitem_view_sep2;
static GtkMenuItem *menuitem_view_subtitles;
static GtkMenuItem *menuitem_view_smaller_subtitle;
static GtkMenuItem *menuitem_view_larger_subtitle;
static GtkMenuItem *menuitem_view_decrease_subtitle_delay;
static GtkMenuItem *menuitem_view_increase_subtitle_delay;
static GtkMenuItem *menuitem_view_sep5;
static GtkMenuItem *menuitem_view_angle;
static GtkMenuItem *menuitem_view_controls;
static GtkMenuItem *menuitem_view_sep3;
static GtkMenuItem *menuitem_view_advanced;
static GtkMenu *menu_help;
static GtkMenuItem *menuitem_help_about;

static GtkMenu *popup_menu;
static GtkMenuItem *menuitem_open;
static GtkMenuItem *menuitem_sep3;
static GtkMenuItem *menuitem_play;
static GtkMenuItem *menuitem_stop;
static GtkMenuItem *menuitem_sep1;
static GtkMenuItem *menuitem_copyurl;
static GtkMenuItem *menuitem_sep2;
static GtkMenuItem *menuitem_showcontrols;
static GtkMenuItem *menuitem_fullscreen;
static GtkMenuItem *menuitem_config;
static GtkMenuItem *menuitem_quit;
static gulong delete_signal_id;

static GtkWidget *vbox_master;
static GtkWidget *vbox;
static GtkWidget *controls_box;

static GtkWidget *media_hbox;
static GtkWidget *media_label;
static GtkWidget *details_vbox;

static GtkWidget *details_video_size;
static GtkWidget *details_video_format;
static GtkWidget *details_video_codec;
static GtkWidget *details_video_fps;
static GtkWidget *details_video_bitrate;
static GtkWidget *details_video_chapters;
static GtkWidget *details_audio_format;
static GtkWidget *details_audio_codec;
static GtkWidget *details_audio_channels;
static GtkWidget *details_audio_bitrate;
static GtkWidget *details_audio_samplerate;

static GdkPixbuf *pb_icon;
static GdkPixbuf *pb_button;

static GList *icon_list;

static GtkWidget *button_event_box;
static GtkWidget *image_button;

static GtkWidget *stop_event_box;
static GtkWidget *ff_event_box;
static GtkWidget *rew_event_box;

static gboolean in_button;

static GtkWidget *image_play;
static GtkWidget *image_pause;
static GtkWidget *image_stop;
static GtkWidget *image_ff;
static GtkWidget *image_rew;
static GtkWidget *image_next;
static GtkWidget *image_prev;
static GtkWidget *image_menu;
static GtkWidget *image_fs;

static GtkStatusIcon *status_icon;
static GtkWidget *config_show_status_icon;

static GtkWidget *config_vo;
static GtkWidget *config_hardware_codecs;
static GtkWidget *config_crystalhd_codecs;
static GtkWidget *config_ao;
static GtkWidget *config_mixer;
static GtkWidget *config_audio_channels;
static GtkWidget *config_use_hw_audio;
#ifndef HAVE_ASOUNDLIB
static GtkWidget *config_volume;
#endif
static GtkWidget *config_cachesize;
static GtkWidget *config_osdlevel;
static GtkWidget *config_deinterlace;
static GtkWidget *config_framedrop;
static GtkWidget *config_pplevel;

static GtkWidget *config_resume_mode;
static GtkWidget *config_playlist_visible;
static GtkWidget *config_details_visible;
static GtkWidget *config_vertical_layout;
static GtkWidget *config_single_instance;
static GtkWidget *config_replace_and_play;
static GtkWidget *config_bring_to_front;
static GtkWidget *config_resize_on_new_media;
static GtkWidget *config_pause_on_click;
static GtkWidget *config_softvol;
static GtkWidget *config_remember_softvol;
static GtkWidget *config_volume_gain;
static GtkWidget *config_forcecache;
static GtkWidget *config_verbose;
static GtkWidget *config_use_mediakeys;
static GtkWidget *config_use_defaultpl;
static GtkWidget *config_disable_animation;
static GtkWidget *config_disable_cover_art_fetch;
static GtkWidget *config_mouse_wheel;

static GtkWidget *config_alang;
static GtkWidget *config_slang;
static GtkWidget *config_metadata_codepage;

static GtkWidget *config_ass;
static GtkWidget *config_embeddedfonts;
static GtkWidget *config_subtitle_font;
static GtkWidget *config_subtitle_scale;
static GtkWidget *config_subtitle_codepage;
static GtkWidget *config_subtitle_color;
static GtkWidget *config_subtitle_outline;
static GtkWidget *config_subtitle_shadow;
static GtkWidget *config_subtitle_margin;
static GtkWidget *config_subtitle_fuzziness;
static GtkWidget *config_show_subtitles;

static GtkWidget *config_mplayer_bin;
static GtkWidget *config_mplayer_dvd_device;
static GtkWidget *config_extraopts;
static GtkWidget *config_remember_loc;
static GtkWidget *config_keep_on_top;

static GtkWidget *open_location;

static GtkWidget *folder_progress_window;
static GtkWidget *folder_progress_label;
static GtkWidget *folder_progress_bar;

// Video Settings
static GtkWidget * vpa_scale_brightness;
static GtkWidget * vpa_scale_contrast;
static GtkWidget * vpa_scale_gamma;
static GtkWidget * vpa_scale_hue;
static GtkWidget * vpa_scale_saturation;

static glong last_movement_time;

static gboolean adjusting;
static GtkWidget *config_accel_keys[KEY_COUNT];

#define PAYLOADNCH 7
#define PAYLOADNI 512
typedef struct _Export {
    int nch;
    int size;
    unsigned long long counter;
    gint16 payload[PAYLOADNCH][PAYLOADNI];
} Export;

PLAYSTATE media_state_to_playstate(GmtkMediaPlayerMediaState media_state)
{
    PLAYSTATE ret;

    switch (media_state) {
    case MEDIA_STATE_PAUSE:
        ret = PAUSED;
        break;

    case MEDIA_STATE_PLAY:
        ret = PLAYING;
        break;

    case MEDIA_STATE_QUIT:
    case MEDIA_STATE_UNKNOWN:
        ret = QUIT;
        break;

    case MEDIA_STATE_STOP:
        ret = STOPPED;
        break;

    default:
        ret = QUIT;
        break;
    }

    return ret;
}

void set_media_player_attributes(GtkWidget * widget)
{
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ENABLE_DEBUG, verbose);
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_FORCE_CACHE, forcecache);
    gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_CACHE_SIZE, cache_size);
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ENABLE_HARDWARE_CODECS,
                                            use_hardware_codecs);
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ENABLE_CRYSTALHD_CODECS,
                                            use_crystalhd_codecs);
    if (option_vo != NULL) {
        gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_VO, option_vo);
        gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ENABLE_HARDWARE_CODECS, FALSE);
    } else {
        gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_VO, vo);
    }
    gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_EXTRA_OPTS, extraopts);
    gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_PREFERRED_AUDIO_LANGUAGE, alang);
    gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_PREFERRED_SUBTITLE_LANGUAGE, slang);
    gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MPLAYER_BINARY, mplayer_bin);
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ENABLE_ADVANCED_SUBTITLES,
                                            !disable_ass);
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_OUTLINE, subtitle_outline);
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_SHADOW, subtitle_shadow);
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ENABLE_EMBEDDED_FONTS,
                                            !disable_embeddedfonts);
    gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_SCALE, subtitle_scale);
    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_MARGIN, subtitle_margin);
    gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_COLOR, subtitle_color);
    gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_CODEPAGE, subtitle_codepage);
    gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_FONT, subtitlefont);
    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_FUZZINESS, subtitle_fuzziness);
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUB_VISIBLE, showsubtitles);

    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_DEINTERLACE, !disable_deinterlace);
    gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AO, audio_device.mplayer_ao);
    if (softvol) {
        gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SOFTVOL, TRUE);
        gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_VOLUME_GAIN, volume_gain);
    } else {
        gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SOFTVOL,
                                                audio_device.type == AUDIO_TYPE_SOFTVOL);
    }
    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_CHANNELS, audio_channels);
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HARDWARE_AC3, use_hw_audio);

    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_OSDLEVEL, osdlevel);
    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_POST_PROCESSING_LEVEL, pplevel);
    gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_START_TIME, (gdouble) start_second);
    gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_RUN_TIME, (gdouble) play_length);
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_DISABLE_UPSCALING, FALSE);

    if (option_dvd_device == NULL) {
        gmtk_media_player_set_media_device(GMTK_MEDIA_PLAYER(media), idledata->device);
    } else {
        gmtk_media_player_set_media_device(GMTK_MEDIA_PLAYER(media), option_dvd_device);
    }

}


gboolean add_to_playlist_and_play(gpointer data)
{
    gchar *s = (gchar *) data;
    gboolean playlist;
    gchar *buf = NULL;

    selection = NULL;

    gm_log(verbose, G_LOG_LEVEL_DEBUG, "adding '%s' to playlist", s);
    if (!uri_exists(s) && !streaming_media(s)) {
        buf = g_filename_to_uri(s, NULL, NULL);
    } else {
        buf = g_strdup(s);
    }
    if (buf != NULL) {
        playlist = detect_playlist(buf);
        if (!playlist) {
            add_item_to_playlist(buf, playlist);
        } else {
            if (!parse_playlist(buf)) {
                add_item_to_playlist(buf, playlist);
            }
        }
        g_free(buf);
    }
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "children = %i",
           gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL));

    if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL) == 1
        || !gtk_list_store_iter_is_valid(playliststore, &iter)) {
        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter)) {
            if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
                dontplaynext = TRUE;
            gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
            gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_FILE);
            g_idle_add(async_play_iter, &iter);
            if (bring_to_front)
                present_main_window();
        }
    }
    if (s != NULL)
        g_free(s);
    g_idle_add(set_update_gui, NULL);
    return FALSE;
}

gboolean clear_playlist_and_play(gpointer data)
{
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "clearing playlist");
    gtk_list_store_clear(playliststore);
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "adding '%s' to playlist", (gchar *) data);
    add_to_playlist_and_play(data);
    return FALSE;
}

void view_option_show_callback(GtkWidget * widget, gpointer data)
{
    skip_fixed_allocation_on_show = TRUE;
}

void view_option_hide_callback(GtkWidget * widget, gpointer data)
{
    skip_fixed_allocation_on_hide = TRUE;
    g_idle_add(set_adjust_layout, NULL);
}

void view_option_size_allocate_callback(GtkWidget * widget, GtkAllocation * allocation, gpointer data)
{
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "plvbox size_allocate_callback");
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "size_allocate_callback");
    g_idle_add(set_adjust_layout, NULL);
}

void player_size_allocate_callback(GtkWidget * widget, GtkAllocation * allocation)
{
    if (idledata->videopresent) {
        non_fs_width = allocation->width;
        non_fs_height = allocation->height;
    }
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "video: %s media size = %i x %i", gm_bool_to_string(idledata->videopresent),
           non_fs_width, non_fs_height);
}

gboolean set_adjust_layout(gpointer data)
{
    adjusting = FALSE;
    adjust_layout();
    return FALSE;
}

void adjust_layout()
{
    gint total_height;
    gint total_width;
    gint handle_size;
    GtkAllocation alloc = { 0 };
    GtkAllocation alloc2 = { 0 };

    gm_log(verbose, G_LOG_LEVEL_DEBUG, "media size = %i x %i", non_fs_width, non_fs_height);
    total_height = non_fs_height;
    total_width = non_fs_width;
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "total = %i x %i", total_width, total_height);

    if (idledata->videopresent) {
        gtk_widget_show(media);
    } else {
        gtk_widget_hide(media);
    }

    if (playlist_visible && remember_loc && !vertical_layout) {
        total_width = gtk_paned_get_position(GTK_PANED(pane));
    }

    if (playlist_visible && remember_loc && vertical_layout) {
        total_height = gtk_paned_get_position(GTK_PANED(pane));
    }

    if (total_width == 0) {
        if (playlist_visible && !vertical_layout) {
            total_width = gtk_paned_get_position(GTK_PANED(pane));
        } else {
            if (showcontrols) {
                gtk_widget_get_allocation(controls_box, &alloc);
                total_width = alloc.width;
            }
        }
    }

    if (total_height == 0) {
        if (playlist_visible && vertical_layout) {
            total_height = gtk_paned_get_position(GTK_PANED(pane));
        }
    }

    if (GTK_IS_WIDGET(media_hbox)
        && gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_info))) {
        if (gtk_widget_get_visible(media_hbox) == 0) {
            gtk_widget_show_all(media_hbox);
            //gm_log(verbose, G_LOG_LEVEL_DEBUG, "waiting for all events to drain");
            //while (gtk_events_pending()) gtk_main_iteration();
        }
        gtk_widget_get_allocation(media_hbox, &alloc);
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "media height = %i", alloc.height);
        total_height += alloc.height;
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "total_height = %i", total_height);
    } else {
        gtk_widget_hide(media_hbox);
    }

    if (GTK_IS_WIDGET(details_table)
        && gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_details))) {
        if (gtk_widget_get_visible(details_vbox) == 0) {
            gtk_widget_show_all(details_vbox);
            //return;
        }
        gtk_widget_get_allocation(details_vbox, &alloc);
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "detail height = %i", alloc.height);
        total_height += alloc.height;
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "total_height = %i", total_height);
    } else {
        gtk_widget_hide(details_vbox);
    }

    if (GTK_IS_WIDGET(plvbox)
        && gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist))) {
        if (gtk_widget_get_visible(plvbox) == 0) {
            gtk_widget_show_all(plvbox);
            //return;
        }
        gtk_widget_grab_focus(play_event_box);
        gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
        gtk_widget_grab_default(plclose);
        gtk_widget_style_get(pane, "handle-size", &handle_size, NULL);

        gtk_widget_get_allocation(plvbox, &alloc);
        if (vertical_layout) {
            gm_log(verbose, G_LOG_LEVEL_DEBUG, "totals = %i x %i", total_width, total_height);
            total_height += alloc.height + handle_size;
        } else {
            if (gtk_widget_get_visible(media_hbox) && idledata->videopresent == FALSE
                && gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist)) == FALSE) {
                gtk_widget_get_allocation(media_hbox, &alloc2);
                total_width = alloc2.width + handle_size + alloc.width;
            } else {
                total_width += handle_size + alloc.width;
            }
        }

        if (non_fs_height == 0) {
            gm_log(verbose, G_LOG_LEVEL_DEBUG, "height = %i", total_height);
            gm_log(verbose, G_LOG_LEVEL_DEBUG, "total_height = %i", total_height);
            if (alloc.height < 16) {
                total_height = 200;
            } else {
                total_height = MAX(total_height, alloc.height);
            }
            gm_log(verbose, G_LOG_LEVEL_DEBUG, "total_height = %i", total_height);
        }

    } else {
        if (!idledata->videopresent) {
            gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
        }
        gtk_paned_set_position(GTK_PANED(pane), -1);
        if (gtk_widget_get_visible(plvbox) == 1) {
            gtk_widget_hide(plvbox);
            //return;
        }
    }

    if (!fullscreen) {
        gtk_widget_get_allocation(menubar, &alloc);
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "menubar = %i", alloc.height);
        total_height += alloc.height;
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "total_height = %i", total_height);
    }

    if (showcontrols) {
        gtk_widget_get_allocation(controls_box, &alloc);
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "controls height = %i", alloc.height);
        total_height += alloc.height;
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "total_height = %i", total_height);

    }
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "total = %i x %i video = %s", total_width, total_height,
           gm_bool_to_string(idledata->videopresent));

    if (use_remember_loc) {
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "setting size to %i x %i", loc_window_width, loc_window_height);
        gtk_window_resize(GTK_WINDOW(window), loc_window_width, loc_window_height);
        use_remember_loc = FALSE;
    } else {
        if (total_height > 0 && total_width > 0 && idledata->videopresent) {
            gtk_window_resize(GTK_WINDOW(window), total_width, total_height);
        }
        if (total_height > 0 && total_width > 0 && !idledata->window_resized && !remember_loc) {
            gtk_window_resize(GTK_WINDOW(window), total_width, total_height);
        }
        if (total_height > 0 && total_width > 0 && !idledata->videopresent && !vertical_layout
            && gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist))) {
            gtk_window_resize(GTK_WINDOW(window), total_width, total_height);
        }
    }


    return;
}

gboolean hide_buttons(void *data)
{
    IdleData *idle = (IdleData *) data;

    if (GTK_IS_WIDGET(ff_event_box)) {
        if (idle->streaming) {
            gtk_widget_hide(ff_event_box);
            gtk_widget_hide(rew_event_box);
        } else {
            if (window_x > 250) {
                gtk_widget_show_all(ff_event_box);
                gtk_widget_show_all(rew_event_box);
            }
        }
    }
    if (GTK_IS_WIDGET(menuitem_view_details))
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_details), TRUE);

    if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL) < 2
        && !gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HAS_CHAPTERS)) {
        gtk_widget_hide(prev_event_box);
        gtk_widget_hide(next_event_box);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_random), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_play_single), FALSE);
        gtk_widget_hide(GTK_WIDGET(menuitem_prev));
        gtk_widget_hide(GTK_WIDGET(menuitem_next));
    } else {
        gtk_widget_show_all(prev_event_box);
        gtk_widget_show_all(next_event_box);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_random), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_play_single), TRUE);
        gtk_widget_show(GTK_WIDGET(menuitem_prev));
        gtk_widget_show(GTK_WIDGET(menuitem_next));
    }

    if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL) > 0) {
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_loop), TRUE);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_loop), FALSE);
    }

    return FALSE;
}

gboolean set_title_bar(void *data)
{

    IdleData *idle = (IdleData *) data;
    gchar *buf;
    gchar *name = NULL;
    GtkTreePath *path;
    gint current = 0, total;
    gchar *filename = NULL;

    if (data != NULL && idle != NULL && gtk_widget_get_visible(window)) {
        if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_VIDEO_PRESENT)
            && gmtk_media_player_get_media_type(GMTK_MEDIA_PLAYER(media)) == TYPE_FILE
            && g_strrstr(gmtk_media_player_get_uri(GMTK_MEDIA_PLAYER(media)), "/") != NULL) {
            filename = g_filename_from_uri(gmtk_media_player_get_uri(GMTK_MEDIA_PLAYER(media)), NULL, NULL);
            if (filename != NULL) {
                name = g_strdup(g_strrstr(filename, "/") + 1);
                g_free(filename);
            }
            filename = NULL;
        } else {
            name = g_strdup(idle->display_name);
        }

        total = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL);
        if (total > 0 && gtk_list_store_iter_is_valid(playliststore, &iter)) {
            path = gtk_tree_model_get_path(GTK_TREE_MODEL(playliststore), &iter);
            if (path != NULL) {
                buf = gtk_tree_path_to_string(path);
                current = (gint) g_strtod(buf, NULL);
                g_free(buf);
                gtk_tree_path_free(path);
            }
        }
        if (total > 1) {
            if (name == NULL || strlen(name) < 1) {
                buf = g_strdup_printf(_("(%i/%i) - Media Player"), current + 1, total);
            } else {
                buf = g_strdup_printf(_("%s - (%i/%i) - Media Player"), name, current + 1, total);
            }
        } else {
            if (name == NULL || strlen(name) < 1) {
                buf = g_strdup_printf(_("Media Player"));
            } else {
                buf = g_strdup_printf(_("%s - Media Player"), name);
            }
        }
        gtk_window_set_title(GTK_WINDOW(window), buf);
        g_free(buf);
        g_free(name);
    }
    return FALSE;
}

gboolean set_media_label(void *data)
{

    IdleData *idle = (IdleData *) data;
    gchar *cover_art_file = NULL;
    gpointer pixbuf;
    GdkPixbuf *scaled;
    gdouble aspect;

    if (data != NULL && idle != NULL && GTK_IS_WIDGET(media_label)) {
        if (idle->media_info != NULL && strlen(idle->media_info) > 0) {
            gtk_label_set_markup(GTK_LABEL(media_label), idle->media_info);
            gtk_label_set_max_width_chars(GTK_LABEL(media_label), 10);
        }

        pixbuf = NULL;
        gtk_image_clear(GTK_IMAGE(cover_art));
        if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL) > 0
            && gtk_list_store_iter_is_valid(playliststore, &iter)) {
            gtk_tree_model_get(GTK_TREE_MODEL(playliststore), &iter, COVERART_COLUMN, &cover_art_file, -1);
        }
        if (cover_art_file != NULL) {
            pixbuf = gdk_pixbuf_new_from_file(cover_art_file, NULL);
            if (pixbuf != NULL) {
                aspect = gdk_pixbuf_get_width(GDK_PIXBUF(pixbuf)) / gdk_pixbuf_get_height(GDK_PIXBUF(pixbuf));
                scaled = gdk_pixbuf_scale_simple(GDK_PIXBUF(pixbuf), 128, 128 * aspect, GDK_INTERP_BILINEAR);
                gtk_image_set_from_pixbuf(GTK_IMAGE(cover_art), scaled);
                g_object_unref(scaled);
                g_object_unref(pixbuf);
            }
        }
    } else {
        if (GTK_IS_IMAGE(cover_art))
            gtk_image_clear(GTK_IMAGE(cover_art));
        if (GTK_IS_WIDGET(media_label)) {
            gtk_label_set_markup(GTK_LABEL(media_label), "");
        }
        return FALSE;
    }

    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_info))
        && strlen(idle->media_info) > 0) {
        gtk_widget_show_all(media_hbox);
    } else {
        gtk_widget_hide(media_hbox);
    }

    g_idle_add(set_adjust_layout, NULL);

    return FALSE;
}

gboolean set_progress_value(void *data)
{

    IdleData *idle = (IdleData *) data;
    gchar *text;
    struct stat buf = { 0 };
    gchar *iterfilename;
    gchar *iteruri = NULL;

    if (idle == NULL)
        return FALSE;

    if (GTK_IS_WIDGET(tracker)) {
        if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_UNKNOWN && rpcontrols == NULL) {
            js_state = STATE_BUFFERING;
            text = g_strdup_printf(_("Buffering: %2i%%"), (gint) (idle->cachepercent * 100));
            gmtk_media_tracker_set_text(tracker, text);
            g_free(text);
            gtk_widget_set_sensitive(play_event_box, FALSE);
        } else {
            if (gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_POSITION_PERCENT) > 0.0) {
                if (autopause == FALSE)
                    gtk_widget_set_sensitive(play_event_box, TRUE);
            }
        }
        if (idle->cachepercent < 1.0
            && gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
            text = g_strdup_printf(_("Paused | %2i%% \342\226\274"), (gint) (idle->cachepercent * 100));
            gmtk_media_tracker_set_text(tracker, text);
            g_free(text);
        }
        gmtk_media_tracker_set_cache_percentage(tracker, idle->cachepercent);
    }

    if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL) > 0
        && gtk_list_store_iter_is_valid(playliststore, &iter)) {
        gtk_tree_model_get(GTK_TREE_MODEL(playliststore), &iter, ITEM_COLUMN, &iteruri, -1);
    }

    if (idle->cachepercent > 0.0 && idle->cachepercent < 0.9 && !forcecache && !streaming_media(iteruri)) {
        if (autopause == FALSE && gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PLAY) {
            if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
                g_free(iteruri);
                gtk_tree_model_get(GTK_TREE_MODEL(playliststore), &iter, ITEM_COLUMN, &iteruri, -1);
                if (iteruri != NULL) {
                    iterfilename = g_filename_from_uri(iteruri, NULL, NULL);
                    g_stat(iterfilename, &buf);
                    gm_log(verbose, G_LOG_LEVEL_DEBUG, "filename = %s", iterfilename);
                    gm_log(verbose, G_LOG_LEVEL_DEBUG, "disk size = %li, byte pos = %li",
                           (glong) buf.st_size, idle->byte_pos);
                    gm_log(verbose, G_LOG_LEVEL_DEBUG, "cachesize = %f, percent = %f", idle->cachepercent,
                           gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media),
                                                                  ATTRIBUTE_POSITION_PERCENT));
                    gm_log(verbose, G_LOG_LEVEL_DEBUG, "will pause = %s",
                           gm_bool_to_string(((idle->byte_pos + (cache_size * 512)) > buf.st_size)
                                             && !(playlist)));
                    // if ((idle->percent + 0.10) > idle->cachepercent && ((idle->byte_pos + (512 * 1024)) > buf.st_size)) {
                    // if ((buf.st_size > 0) && (idle->byte_pos + (cache_size * 512)) > buf.st_size) {
                    if (((idle->byte_pos + (cache_size * 1024)) > buf.st_size) && !(playlist)) {
                        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_PAUSE);
                        gtk_widget_set_sensitive(play_event_box, FALSE);
                        autopause = TRUE;
                    }
                    g_free(iterfilename);
                    g_free(iteruri);
                }
            }
        } else if (autopause == TRUE
                   && gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
            if (idle->cachepercent >
                (gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_POSITION_PERCENT) + 0.20)
                || (idle->cachepercent >= 0.99 || idle->cachepercent == 0.0)) {
                gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_PLAY);
                gtk_widget_set_sensitive(play_event_box, TRUE);
                autopause = FALSE;
            }
        }
    }

    if (idle->cachepercent > 0.95 || idle->cachepercent == 0.0) {
        if (autopause == TRUE && gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
            gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_PLAY);
            gtk_widget_set_sensitive(play_event_box, TRUE);
            autopause = FALSE;
        }
    }

    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_UNKNOWN) {
        gtk_widget_set_sensitive(play_event_box, TRUE);
    }

    update_status_icon();

    return FALSE;
}

gboolean set_progress_text(void *data)
{

    IdleData *idle = (IdleData *) data;


    if (GTK_IS_WIDGET(tracker)) {
        gmtk_media_tracker_set_text(tracker, idle->progress_text);
    }
    update_status_icon();
    return FALSE;
}

gboolean set_progress_time(void *data)
{
    gchar *text;

    IdleData *idle = (IdleData *) data;

    if (idle == NULL)
        return FALSE;

    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PLAY) {
        text = g_strdup_printf(_("Playing"));
    } else if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
        text = g_strdup_printf(_("Paused"));
    } else {
        text = g_strdup_printf(_("Idle"));
    }

    if (idle->cachepercent > 0 && idle->cachepercent < 1.0 && !(playlist) && !forcecache && !idle->streaming) {
        g_snprintf(idle->progress_text, sizeof(idle->progress_text), "%s | %2i%% \342\226\274", text,
                   (int) (idle->cachepercent * 100));
    } else {
        g_snprintf(idle->progress_text, sizeof(idle->progress_text), "%s", text);
    }

    gmtk_media_tracker_set_text(tracker, text);
    g_free(text);

    if (GTK_IS_WIDGET(tracker) && idle->position > 0
        && gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_PAUSE) {
        gmtk_media_tracker_set_length(tracker, idle->length);
        gmtk_media_tracker_set_position(tracker, idle->position);
    }

    update_status_icon();

    return FALSE;
}

gboolean set_volume_from_slider(gpointer data)
{
    gdouble vol;

    vol = gtk_scale_button_get_value(GTK_SCALE_BUTTON(vol_slider));
    if (!gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED)) {
        gmtk_media_player_set_volume(GMTK_MEDIA_PLAYER(media), vol);
        if (remember_softvol) {
            volume_softvol = vol;
            set_software_volume(&volume_softvol);
        }
    }

    return FALSE;
}

gboolean set_volume_tip(void *data)
{
    if (GTK_IS_WIDGET(vol_slider)) {
        // should be automatic
    }
    return FALSE;
}

gboolean set_window_visible(void *data)
{
    if (GTK_IS_WIDGET(media)) {
        if (vertical_layout) {
            gtk_widget_show(media);
        } else {
            gtk_widget_show(vbox);
        }
    }
    return FALSE;
}

gboolean set_subtitle_visibility(void *data)
{
    if (GTK_IS_WIDGET(menuitem_view_subtitles)) {
        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_subtitles))) {
            gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUB_VISIBLE, TRUE);
        } else {
            gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUB_VISIBLE, FALSE);
        }
    }
    return FALSE;
}

gboolean set_update_gui(void *data)
{
    GtkTreeViewColumn *column;
    gchar *coltitle;
    gint count;
    gchar **split;
    gchar *joined;

    if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL) < 2
        && !gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HAS_CHAPTERS)) {
        gtk_widget_hide(prev_event_box);
        gtk_widget_hide(next_event_box);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_random), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_play_single), FALSE);
        gtk_widget_hide(GTK_WIDGET(menuitem_prev));
        gtk_widget_hide(GTK_WIDGET(menuitem_next));
    } else {
        gtk_widget_show(prev_event_box);
        gtk_widget_show(next_event_box);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_random), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_play_single), TRUE);
        gtk_widget_show(GTK_WIDGET(menuitem_prev));
        gtk_widget_show(GTK_WIDGET(menuitem_next));
    }

    if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL) > 0) {
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_loop), TRUE);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_loop), FALSE);
    }

    if (GTK_IS_WIDGET(list)) {
        column = gtk_tree_view_get_column(GTK_TREE_VIEW(list), 0);
        count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL);
        if (playlistname != NULL && strlen(playlistname) > 0) {
            coltitle = g_strdup_printf(_("%s items"), playlistname);
        } else {
            coltitle = g_strdup_printf(ngettext("Item to Play", "Items to Play", count));
        }

        split = g_strsplit(coltitle, "_", 0);
        joined = g_strjoinv("__", split);
        gtk_tree_view_column_set_title(column, joined);
        g_free(coltitle);
        g_free(joined);
        g_strfreev(split);
    }
    return FALSE;
}

static const gchar *PLAYSTATE_to_string(const PLAYSTATE pstate)
{
    switch (pstate) {
    case PLAYING: return "playing";
    case PAUSED:  return "paused";
    case STOPPED: return "stopped";
    case QUIT:    return "quit";
    default:      return "???";
    }
}

gboolean set_gui_state(void *data)
{
    gchar *tip_text = NULL;
#ifdef GTK3_ENABLED
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    const gchar *icon_start;

    if (gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL)
        icon_start = "media-playback-start-rtl-symbolic";
    else
        icon_start = "media-playback-start-symbolic";
#endif

    gm_log(verbose, G_LOG_LEVEL_MESSAGE, "setting gui state to %s", PLAYSTATE_to_string(guistate));

    if (lastguistate != guistate) {
        if (guistate == PLAYING) {
#ifdef GTK3_ENABLED
            if (gtk_icon_theme_has_icon(icon_theme, "media-playback-pause-symbolic")) {
                gtk_image_set_from_icon_name(GTK_IMAGE(image_play), "media-playback-pause-symbolic", button_size);
            } else {
                gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-pause", button_size);
            }
#else
            gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-pause", button_size);
#endif

            tip_text = gtk_widget_get_tooltip_text(play_event_box);
            if (tip_text == NULL || g_ascii_strcasecmp(tip_text, _("Pause")) != 0)
                gtk_widget_set_tooltip_text(play_event_box, _("Pause"));
            g_free(tip_text);
            gtk_widget_set_sensitive(ff_event_box, TRUE);
            gtk_widget_set_sensitive(rew_event_box, TRUE);
            gtk_container_remove(GTK_CONTAINER(popup_menu), GTK_WIDGET(menuitem_play));
            menuitem_play = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-media-pause", NULL));
            g_signal_connect(G_OBJECT(menuitem_play), "activate", G_CALLBACK(menuitem_pause_callback), NULL);
            gtk_menu_shell_insert(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_play), 0);
            gtk_widget_show(GTK_WIDGET(menuitem_play));
        }

        if (guistate == PAUSED) {
#ifdef GTK3_ENABLED
            if (gtk_icon_theme_has_icon(icon_theme, icon_start)) {
                gtk_image_set_from_icon_name(GTK_IMAGE(image_play), icon_start, button_size);
            } else {
                gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
            }
#else
            gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
#endif
            tip_text = gtk_widget_get_tooltip_text(play_event_box);
            if (tip_text == NULL || g_ascii_strcasecmp(tip_text, _("Play")) != 0)
                gtk_widget_set_tooltip_text(play_event_box, _("Play"));
            g_free(tip_text);
            gtk_widget_set_sensitive(ff_event_box, FALSE);
            gtk_widget_set_sensitive(rew_event_box, FALSE);
            gtk_container_remove(GTK_CONTAINER(popup_menu), GTK_WIDGET(menuitem_play));
            menuitem_play = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-media-play", NULL));
            g_signal_connect(G_OBJECT(menuitem_play), "activate", G_CALLBACK(menuitem_play_callback), NULL);
            gtk_menu_shell_insert(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_play), 0);
            gtk_widget_show(GTK_WIDGET(menuitem_play));
        }

        if (guistate == STOPPED) {
#ifdef GTK3_ENABLED
            if (gtk_icon_theme_has_icon(icon_theme, icon_start)) {
                gtk_image_set_from_icon_name(GTK_IMAGE(image_play), icon_start, button_size);
            } else {
                gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
            }
#else
            gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
#endif
            tip_text = gtk_widget_get_tooltip_text(play_event_box);
            if (tip_text == NULL || g_ascii_strcasecmp(tip_text, _("Play")) != 0)
                gtk_widget_set_tooltip_text(play_event_box, _("Play"));
            g_free(tip_text);
            gtk_widget_set_sensitive(ff_event_box, FALSE);
            gtk_widget_set_sensitive(rew_event_box, FALSE);
            gtk_container_remove(GTK_CONTAINER(popup_menu), GTK_WIDGET(menuitem_play));
            menuitem_play = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-media-play", NULL));
            g_signal_connect(G_OBJECT(menuitem_play), "activate", G_CALLBACK(menuitem_play_callback), NULL);
            gtk_menu_shell_insert(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_play), 0);
            gtk_widget_show(GTK_WIDGET(menuitem_play));
        }
        lastguistate = guistate;
    }
    return FALSE;
}

gboolean set_metadata(gpointer data)
{
    MetaData *mdata = (MetaData *) data;
    GtkTreeIter *riter;

    if (mdata == NULL) {
        return FALSE;
    }

    Wg_mutex_lock(&set_mutex);
    riter = find_iter_by_uri(mdata->uri);

    if (gtk_list_store_iter_is_valid(playliststore, riter)) {
        if (mdata != NULL) {
            gtk_list_store_set(playliststore, riter,
                               DESCRIPTION_COLUMN, mdata->title,
                               ARTIST_COLUMN, mdata->artist,
                               ALBUM_COLUMN, mdata->album,
                               SUBTITLE_COLUMN, mdata->subtitle,
                               AUDIO_CODEC_COLUMN, mdata->audio_codec,
                               VIDEO_CODEC_COLUMN, mdata->video_codec,
                               LENGTH_COLUMN, mdata->length,
                               DEMUXER_COLUMN, mdata->demuxer,
                               LENGTH_VALUE_COLUMN, mdata->length_value,
                               VIDEO_WIDTH_COLUMN, mdata->width, VIDEO_HEIGHT_COLUMN,
                               mdata->height, PLAYABLE_COLUMN, mdata->playable, -1);

            if (mdata->playable == FALSE) {
                gtk_list_store_remove(playliststore, riter);
                g_idle_add(set_title_bar, idledata);
            }
        }
    }

    g_free(riter);
    free_metadata(mdata);

    Wg_mutex_unlock(&set_mutex);
    return FALSE;
}

gboolean set_show_seek_buttons(gpointer data)
{
    if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SEEKABLE)) {
        gtk_widget_show_all(ff_event_box);
        gtk_widget_show_all(rew_event_box);
    }

    return FALSE;
}

void cancel_clicked(GtkButton * button, gpointer user_data)
{
    cancel_folder_load = TRUE;
}

void create_folder_progress_window()
{
    GtkWidget *vbox;
    GtkWidget *cancel;
    GtkWidget *hbox;

    cancel_folder_load = FALSE;

    folder_progress_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(folder_progress_window), GDK_WINDOW_TYPE_HINT_UTILITY);
    gtk_window_set_transient_for(GTK_WINDOW(folder_progress_window), GTK_WINDOW(window));
    gtk_window_set_position(GTK_WINDOW(folder_progress_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_icon(GTK_WINDOW(folder_progress_window), pb_icon);
    gtk_window_set_resizable(GTK_WINDOW(folder_progress_window), FALSE);
    gtk_widget_set_size_request(folder_progress_window, 400, -1);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    folder_progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(folder_progress_bar), 0.10);
    folder_progress_label = gtk_label_new("");
    gtk_label_set_ellipsize(GTK_LABEL(folder_progress_label), PANGO_ELLIPSIZE_MIDDLE);

    cancel = gtk_button_new_from_stock("gtk-cancel");
    g_signal_connect(G_OBJECT(cancel), "clicked", G_CALLBACK(cancel_clicked), NULL);

    hbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_END);
    gtk_container_add(GTK_CONTAINER(hbox), cancel);

    gtk_container_add(GTK_CONTAINER(vbox), folder_progress_bar);
    gtk_container_add(GTK_CONTAINER(vbox), folder_progress_label);
    gtk_container_add(GTK_CONTAINER(vbox), hbox);
    gtk_container_add(GTK_CONTAINER(folder_progress_window), vbox);

    gtk_widget_show_all(folder_progress_window);
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "waiting for all events to drain");
    while (gtk_events_pending())
        gtk_main_iteration();
}

void destroy_folder_progress_window()
{
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "waiting for all events to drain");
    while (gtk_events_pending())
        gtk_main_iteration();
    if (GTK_IS_WIDGET(folder_progress_window))
        gtk_widget_destroy(folder_progress_window);
    folder_progress_window = NULL;
    if (cancel_folder_load)
        clear_playlist(NULL, NULL);
    cancel_folder_load = FALSE;
}


gboolean set_item_add_info(void *data)
{
    gchar *message;

    if (data == NULL)
        return FALSE;

    if (GTK_IS_WIDGET(folder_progress_window)) {
        message = g_strdup_printf(_("Adding %s to playlist"), (gchar *) data);
        gtk_label_set_text(GTK_LABEL(folder_progress_label), message);
        gtk_progress_bar_pulse(GTK_PROGRESS_BAR(folder_progress_bar));
        g_free(message);
        //gm_log(verbose, G_LOG_LEVEL_DEBUG, "waiting for all events to drain");
        //while (gtk_events_pending()) gtk_main_iteration();
    }

    return FALSE;
}

void remove_langs(GtkWidget * item, gpointer data)
{
    if (GTK_IS_WIDGET(item))
        gtk_widget_destroy(item);
}

void update_status_icon()
{
    gchar *text;

    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PLAY) {
        text = g_strdup_printf(_("Playing %s"), idledata->display_name);
    } else if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
        text = g_strdup_printf(_("Paused %s"), idledata->display_name);
    } else {
        text = g_strdup_printf(_("Idle"));
    }

    if (GTK_IS_WIDGET(status_icon)) {
        gtk_status_icon_set_tooltip_text(status_icon, text);
    }
    g_free(text);
}

gboolean resize_window(void *data)
{
    IdleData *idle = (IdleData *) data;
    GTimeVal currenttime;
    GValue resize_value = { 0 };
    GValue shrink_value = { 0 };

    g_value_init(&resize_value, G_TYPE_BOOLEAN);
    g_value_init(&shrink_value, G_TYPE_BOOLEAN);

    if (GTK_IS_WIDGET(window)) {
        if (idle->videopresent) {
            g_value_set_boolean(&resize_value, TRUE);
            g_value_set_boolean(&shrink_value, TRUE);
            gtk_container_child_set_property(GTK_CONTAINER(pane), vbox, "resize", &resize_value);
            gtk_container_child_set_property(GTK_CONTAINER(pane), vbox, "shrink", &shrink_value);
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_info), FALSE);
            g_get_current_time(&currenttime);
            last_movement_time = currenttime.tv_sec;
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_info), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_set_subtitle), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_set_audiofile), TRUE);
            gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
#ifdef GTK3_ENABLED
            gtk_window_set_has_resize_grip(GTK_WINDOW(window), idledata->videopresent);
#endif
            gtk_widget_show_all(GTK_WIDGET(media));
            gm_log(verbose, G_LOG_LEVEL_DEBUG, "waiting for all events to drain");
            while (gtk_events_pending())
                gtk_main_iteration();

            if (window_x == 0 && window_y == 0) {
                gm_log(verbose, G_LOG_LEVEL_INFO, "current size = %i x %i", non_fs_width, non_fs_height);
                if (resize_on_new_media == TRUE || gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL)
                    || idle->streaming) {
                    if (idle->width > 1 && idle->height > 1) {
                        gm_log(verbose, G_LOG_LEVEL_INFO, "Changing window size to %i x %i visible = %s", idle->width,
                               idle->height, gm_bool_to_string(gtk_widget_get_visible(media)));
                        last_window_width = idle->width;
                        last_window_height = idle->height;
                        non_fs_width = idle->width;
                        non_fs_height = idle->height;
                        adjusting = TRUE;
                        g_idle_add(set_adjust_layout, &adjusting);
                    }
                } else {
                    gm_log(verbose, G_LOG_LEVEL_INFO, "old aspect is %f new aspect is %f",
                           (gfloat) non_fs_width / (gfloat) non_fs_height,
                           (gfloat) (idle->width) / (gfloat) (idle->height));
                }
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_fullscreen), fullscreen);
                if (init_fullscreen) {
                    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_fullscreen), init_fullscreen);
                    init_fullscreen = 0;
                }

            } else {
                if (window_x > 0 && window_y > 0) {
                    gtk_window_resize(GTK_WINDOW(window), window_x, window_y);
                }
            }
        } else {
            // audio only file

            g_value_set_boolean(&resize_value, FALSE);
            g_value_set_boolean(&shrink_value, FALSE);
            gtk_container_child_set_property(GTK_CONTAINER(pane), vbox, "resize", &resize_value);
            gtk_container_child_set_property(GTK_CONTAINER(pane), vbox, "shrink", &shrink_value);
            non_fs_height = 0;
            non_fs_width = 0;
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_fullscreen), FALSE);
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_info), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_info), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_set_subtitle), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_set_audiofile), FALSE);
            last_window_height = 0;
            last_window_width = 0;
            g_idle_add(set_adjust_layout, NULL);
            if (window_x > 0) {
                gtk_widget_set_size_request(media_label, window_x, -1);
            } else {
                gtk_widget_set_size_request(media_label, 300, -1);
            }
            // adjust_layout();
        }
    }
    if (idle != NULL)
        idle->window_resized = TRUE;

    return FALSE;
}

gboolean set_play(void *data)
{
    if (bring_to_front)
        gtk_window_present(GTK_WINDOW(window));

    play_callback(NULL, NULL, data);
    return FALSE;
}

gboolean set_pause(void *data)
{

    pause_callback(NULL, NULL, data);
    return FALSE;
}

gboolean set_stop(void *data)
{

    stop_callback(NULL, NULL, data);
    return FALSE;
}

gboolean set_ff(void *data)
{

    ff_callback(NULL, NULL, NULL);      // ok is just not NULL which is what we want
    return FALSE;
}

gboolean set_rew(void *data)
{

    rew_callback(NULL, NULL, NULL);     // ok is just not NULL which is what we want
    return FALSE;
}

gboolean set_prev(void *data)
{

    prev_callback(NULL, NULL, NULL);
    return FALSE;
}

gboolean set_next(void *data)
{

    next_callback(NULL, NULL, NULL);
    return FALSE;
}

gboolean set_quit(void *data)
{

    delete_callback(NULL, NULL, NULL);
    g_idle_add(set_destroy, NULL);
    return FALSE;
}

gboolean set_kill_mplayer(void *data)
{
    gm_log(GMTK_MEDIA_PLAYER(media)->debug, G_LOG_LEVEL_DEBUG, "set_kill_mplayer()");
    gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);

    return FALSE;
}

gboolean set_pane_position(void *data)
{
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist))) {
        gtk_paned_set_position(GTK_PANED(pane), loc_panel_position);
    }
    return FALSE;
}

gboolean set_position(void *data)
{
    IdleData *idle = (IdleData *) data;

    if (idle != NULL) {
        gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), idle->position, SEEK_ABSOLUTE);
    }
    return FALSE;
}

gboolean set_raise_window(void *data)
{
    gtk_window_present(GTK_WINDOW(window));
    gdk_window_raise(gtk_widget_get_window(window));
    return FALSE;
}

gboolean set_software_volume(gdouble * data)
{
    gm_store = gm_pref_store_new("g-mplayer");
    gm_pref_store_set_float(gm_store, VOLUME_SOFTVOL, *data);
    gm_pref_store_free(gm_store);
    return FALSE;
}

gboolean hookup_volume(void *data)
{
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "hookup_volume is %f", audio_device.volume);
    if (softvol == FALSE) {
        if (gm_audio_update_device(&audio_device)) {
            if (softvol || audio_device.type == AUDIO_TYPE_SOFTVOL) {
                gm_audio_set_server_volume_update_callback(&audio_device, NULL);
            } else {
                gm_audio_set_server_volume_update_callback(&audio_device, set_volume);
            }
        }
    }
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "out hookup_volume is %f", audio_device.volume);
    return FALSE;
}


gboolean set_volume(void *data)
{
    IdleData *idle = (IdleData *) data;
    gchar *buf = NULL;

    gm_log(verbose, G_LOG_LEVEL_DEBUG, "set_volume new volume is %f", audio_device.volume);
    if (data == NULL) {
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "in set_volume without data");
        if (!(softvol || audio_device.type == AUDIO_TYPE_SOFTVOL)) {
            gm_audio_get_volume(&audio_device);
        }
        if (pref_volume != -1) {
            audio_device.volume = (gdouble) pref_volume / 100.0;
            gmtk_media_player_set_volume(GMTK_MEDIA_PLAYER(media), audio_device.volume);
            pref_volume = -1;
        }
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "data is null new volume is %f", audio_device.volume);
        gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider), audio_device.volume);
        return FALSE;
    }

    if (GTK_IS_WIDGET(vol_slider) && audio_device.volume >= 0.0 && audio_device.volume <= 1.0) {
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "setting slider to %f", audio_device.volume);
        if (remember_softvol) {
            volume_softvol = audio_device.volume;
            set_software_volume(&volume_softvol);
        }
        if (rpcontrols != NULL && g_ascii_strcasecmp(rpcontrols, "volumeslider") == 0) {
            gtk_range_set_value(GTK_RANGE(vol_slider), audio_device.volume);
            buf = g_strdup_printf(_("Volume %i%%"), (gint) (audio_device.volume * 100.0));
            g_strlcpy(idledata->vol_tooltip, buf, sizeof(idledata->vol_tooltip));
            g_free(buf);
        } else {
            gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider), audio_device.volume);
        }

        g_idle_add(set_volume_tip, idle);
    }

    return FALSE;
}

gboolean set_fullscreen(void *data)
{

    IdleData *idle = (IdleData *) data;

    // we need to flip the state since the callback reads the value of fullscreen
    // and if fullscreen is 0 it sets it to fullscreen.
    // fullscreen = ! (gint) idle->fullscreen;
    // printf("calling fs_callback with %s\n",gm_bool_to_string(fullscreen));
    // fs_callback(NULL, NULL, NULL); // ok is just not NULL which is what we want
    if (idle != NULL && idle->videopresent)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), idle->fullscreen);
    return FALSE;
}

gboolean set_show_controls(void *data)
{

    IdleData *idle = (IdleData *) data;

    if (idle == NULL)
        return FALSE;

    showcontrols = idle->showcontrols;

    if (gtk_widget_get_visible(GTK_WIDGET(menuitem_view_controls))) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_controls), showcontrols);
    } else {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_showcontrols), showcontrols);
    }
    return FALSE;
}

gboolean get_show_controls()
{
    return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_controls));
}

gboolean popup_handler(GtkWidget * widget, GdkEvent * event, void *data)
{
    GtkMenu *menu;
    GdkEventButton *event_button;
    GTimeVal currenttime;
    GtkAllocation alloc;

    g_get_current_time(&currenttime);
    last_movement_time = currenttime.tv_sec;
    g_idle_add(make_panel_and_mouse_visible, NULL);

    menu = GTK_MENU(widget);
    gtk_widget_grab_focus(media);
    if (event->type == GDK_BUTTON_PRESS) {

        event_button = (GdkEventButton *) event;

        if (event_button->button == 3 && disable_context_menu == 0) {
            gtk_menu_popup(menu, NULL, NULL, NULL, NULL, event_button->button, event_button->time);
            return TRUE;
        }

        if (event_button->button == 2) {

            // mute on button 2
            if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED)) {
                gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider), audio_device.volume);
            } else {
                gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider), 0.0);
            }
        }

        if (event_button->button == 1 && idledata->videopresent == TRUE && !disable_pause_on_click &&
            !gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_TITLE_IS_MENU)) {

            switch (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media))) {
            case MEDIA_STATE_PLAY:
                gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_PAUSE);
                break;
            case MEDIA_STATE_PAUSE:
                gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_PLAY);
                break;
            default:
                break;
            }
        }

    }
    // fullscreen on double click of button 1
    if (event->type == GDK_2BUTTON_PRESS) {
        event_button = (GdkEventButton *) event;
        if (event_button->button == 1 && idledata->videopresent == TRUE) {
            gtk_widget_get_allocation(media, &alloc);
            if (event_button->x > alloc.x
                && event_button->y > alloc.y
                && event_button->x < alloc.x + alloc.width && event_button->y < alloc.y + alloc.height) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);

            }
        }
    }

    if (event->type == GDK_BUTTON_RELEASE) {
        event_button = (GdkEventButton *) event;
    }

    return FALSE;
}

gboolean media_scroll_event_callback(GtkWidget * widget, GdkEventScroll * event, gpointer data)
{

    if (mouse_wheel_changes_volume == FALSE) {
        if (event->direction == GDK_SCROLL_UP) {
            set_ff(NULL);
        }

        if (event->direction == GDK_SCROLL_DOWN) {
            set_rew(NULL);
        }
    } else {
        if (event->direction == GDK_SCROLL_UP) {
            audio_device.volume += 0.01;
            g_idle_add(set_volume, idledata);
        }

        if (event->direction == GDK_SCROLL_DOWN) {
            audio_device.volume -= 0.01;
            g_idle_add(set_volume, idledata);
        }
    }

    return TRUE;
}

gboolean notification_handler(GtkWidget * widget, GdkEventCrossing * event, void *data)
{
    return FALSE;
}

gboolean set_destroy(gpointer data)
{
    gtk_widget_destroy(window);
    return FALSE;
}


static void destroy_callback(GtkWidget * widget, gpointer data)
{
    gtk_main_quit();
}

gboolean delete_callback(GtkWidget * widget, GdkEvent * event, void *data)
{
    loop = 0;
    ok_to_play = FALSE;
    dontplaynext = TRUE;

    gm_log(GMTK_MEDIA_PLAYER(media)->debug, G_LOG_LEVEL_DEBUG, "delete_callback()");

    g_idle_remove_by_data(idledata);
    gm_audio_set_server_volume_update_callback(&audio_device, NULL);

    if (remember_loc && !fullscreen) {
        gm_store = gm_pref_store_new("g-mplayer");
        gtk_window_get_position(GTK_WINDOW(window), &loc_window_x, &loc_window_y);
        gtk_window_get_size(GTK_WINDOW(window), &loc_window_width, &loc_window_height);
        loc_panel_position = gtk_paned_get_position(GTK_PANED(pane));

        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist))) {

        }
        gm_pref_store_set_int(gm_store, WINDOW_X, loc_window_x);
        gm_pref_store_set_int(gm_store, WINDOW_Y, loc_window_y);
        gm_pref_store_set_int(gm_store, WINDOW_HEIGHT, loc_window_height);
        gm_pref_store_set_int(gm_store, WINDOW_WIDTH, loc_window_width);
        gm_pref_store_set_int(gm_store, PANEL_POSITION, loc_panel_position);
        gm_pref_store_free(gm_store);
    }

    gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
    gm_log(verbose, G_LOG_LEVEL_DEBUG,
           "waiting for all events to drain and state to become \"unknown\" and the mplayer thread to exit");
    while (gtk_events_pending() || gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN
           || GMTK_MEDIA_PLAYER(media)->mplayer_thread != NULL) {
        gtk_main_iteration();
    }

    g_thread_pool_stop_unused_threads();
    if (retrieve_metadata_pool != NULL) {
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "waiting for all events to drain and retrieve_metadata_pool to empty");
        while (gtk_events_pending() || g_thread_pool_unprocessed(retrieve_metadata_pool)) {
            gtk_main_iteration();
        }
        g_thread_pool_free(retrieve_metadata_pool, TRUE, TRUE);
    }
    if (use_defaultpl)
        save_playlist_pls(default_playlist);
    return FALSE;
}

gboolean status_icon_callback(GtkStatusIcon * icon, gpointer data)
{

    if (gtk_widget_get_visible(window)) {
        gtk_window_get_position(GTK_WINDOW(window), &loc_window_x, &loc_window_y);
        gtk_window_iconify(GTK_WINDOW(window));
        gtk_widget_hide(GTK_WIDGET(window));
    } else {
        gtk_window_deiconify(GTK_WINDOW(window));
        gtk_window_present(GTK_WINDOW(window));
        gtk_window_move(GTK_WINDOW(window), loc_window_x, loc_window_y);
    }
    return FALSE;
}


void status_icon_context_callback(GtkStatusIcon * status_icon, guint button, guint activate_time, gpointer data)
{
    gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
}

gboolean motion_notify_callback(GtkWidget * widget, GdkEventMotion * event, gpointer data)
{
    GTimeVal currenttime;
    GtkAllocation fs_allocation;
    GtkAllocation fs_controls_allocation;

    g_get_current_time(&currenttime);
    last_movement_time = currenttime.tv_sec;
/*
	if (verbose > 1) {
		g_get_current_time(&currenttime);
		printf("motion noticed at %li\n",currenttime.tv_sec);
	}
*/
    if (fullscreen) {
        gtk_widget_get_allocation(window, &fs_allocation);
        gtk_widget_get_allocation(controls_box, &fs_controls_allocation);
        if (event->y > (fs_allocation.height - fs_controls_allocation.height)) {
            g_idle_add(make_panel_and_mouse_visible, NULL);
        } else {
            hide_fs_controls();
        }
    } else {
        g_idle_add(make_panel_and_mouse_visible, NULL);
    }
    return FALSE;
}

gboolean window_state_callback(GtkWidget * widget, GdkEventWindowState * event, gpointer user_data)
{
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "fullscreen = %s",
           gm_bool_to_string((event->new_window_state == GDK_WINDOW_STATE_FULLSCREEN)));
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "State = %i mask = %i flag = %i", event->new_window_state, event->changed_mask,
           GDK_WINDOW_STATE_FULLSCREEN);
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "restore controls = %s showcontrols = %s", gm_bool_to_string(restore_controls),
           gm_bool_to_string(showcontrols));
    if (fullscreen == 1 && (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED)) {
        // fullscreen, but window hidden
        hide_fs_controls();
    }

    if (fullscreen == 1 && (event->new_window_state == GDK_WINDOW_STATE_FULLSCREEN)) {
        if (showcontrols) {
            show_fs_controls();
        }
    }

    return FALSE;

    fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
    if (fullscreen) {
        gtk_widget_hide(menubar);
    } else {
        gtk_widget_show(menubar);
    }
    if (event->changed_mask == GDK_WINDOW_STATE_FULLSCREEN) {
        idledata->showcontrols = restore_controls;
        set_show_controls(idledata);
    }
    return FALSE;
}

gboolean configure_callback(GtkWidget * widget, GdkEventConfigure * event, gpointer user_data)
{
    if (use_remember_loc && event->width == loc_window_width && event->height == loc_window_height) {
        // this might be useful
    }
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_subtitles),
                                   gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media),
                                                                           ATTRIBUTE_SUB_VISIBLE));
    return FALSE;
}

gboolean window_key_callback(GtkWidget * widget, GdkEventKey * event, gpointer user_data)
{
    GTimeVal currenttime;
    gboolean title_is_menu;
    gint index;

    gm_log(verbose, G_LOG_LEVEL_DEBUG, "key = %i", event->keyval);
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "state = %i", event->state);
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "other = %i", event->state & ~GDK_CONTROL_MASK);

    gm_log(verbose, G_LOG_LEVEL_DEBUG, "key name=%s", gdk_keyval_name(event->keyval));
    // We don't want to handle CTRL accelerators here
    // if we pass in items with CTRL then 2 and Ctrl-2 do the same thing
    if (gtk_widget_get_visible(plvbox) && gtk_tree_view_get_enable_search(GTK_TREE_VIEW(list)))
        return FALSE;

    index = get_index_from_key_and_modifier(event->keyval, event->state);
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "index = %i", index);
    if (!event->is_modifier && index != -1) {

        switch (index) {
        case FILE_OPEN_LOCATION:
            break;
        case EDIT_SCREENSHOT:
            if (fullscreen)
                gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_TAKE_SCREENSHOT);
            return FALSE;
        case EDIT_PREFERENCES:
            break;
        case VIEW_PLAYLIST:
            break;
        case VIEW_INFO:
            break;
        case VIEW_DETAILS:
            break;
        case VIEW_FULLSCREEN:
            if (fullscreen) {
                if (idledata->videopresent) {
                    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), FALSE);
                }
            } else {
                if (idledata->videopresent) {
                    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), TRUE);
                }
            }
            return TRUE;
        case VIEW_ASPECT:
            if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_follow_window))) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_default), TRUE);
                return FALSE;
            }
            if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_ten))) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_anamorphic), TRUE);
                return FALSE;
            }
            if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_nine))) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_ten), TRUE);
                return FALSE;
            }
            if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_four_three))) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_nine), TRUE);
                return FALSE;
            }
            if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_anamorphic))) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_follow_window), TRUE);
                return FALSE;
            }
            if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_default)))
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_four_three), TRUE);
            return FALSE;
        case VIEW_SUBTITLES:
            if (fullscreen) {
                if (idledata->videopresent)
                    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUB_VISIBLE,
                                                            !gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER
                                                                                                     (media),
                                                                                                     ATTRIBUTE_SUB_VISIBLE));
            }
            return FALSE;
        case VIEW_DECREASE_SIZE:
        case VIEW_INCREASE_SIZE:
        case VIEW_ANGLE:
        case VIEW_CONTROLS:
            break;
        default:
            gm_log(verbose, G_LOG_LEVEL_MESSAGE, "Keyboard shortcut index %i, not handled in fullscreen at this time",
                   index);

        }
    }

    if (!event->is_modifier && (event->state & GDK_SHIFT_MASK) == 0 && (event->state & GDK_CONTROL_MASK) == 0
        && (event->state & GDK_MOD1_MASK) == 0) {

        g_get_current_time(&currenttime);
        last_movement_time = currenttime.tv_sec;

        title_is_menu = gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_TITLE_IS_MENU);

        g_idle_add(make_panel_and_mouse_visible, NULL);
        switch (event->keyval) {
        case GDK_ISO_Next_Group:
            setup_accelerators();
            return FALSE;
        case GDK_Right:
            if (title_is_menu) {
                return gmtk_media_player_send_key_press_event(GMTK_MEDIA_PLAYER(media), event, data);
            } else {
                if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP
                    && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist)))
                    return ff_callback(NULL, NULL, NULL);
            }
            break;
        case GDK_Left:
            if (title_is_menu) {
                return gmtk_media_player_send_key_press_event(GMTK_MEDIA_PLAYER(media), event, data);
            } else {
                if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP
                    && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist)))
                    return rew_callback(NULL, NULL, NULL);
            }
            break;
        case GDK_Page_Up:
            if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP
                && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist)))
                gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), 600.0, SEEK_RELATIVE);

            if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
                gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), 0.0, SEEK_RELATIVE);
                idledata->position += 600;
                if (idledata->position > idledata->length)
                    idledata->position = 0;
                gmtk_media_tracker_set_percentage(tracker, idledata->position / idledata->length);
            }
            return FALSE;
        case GDK_Page_Down:
            if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP
                && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist)))
                gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), -600.0, SEEK_RELATIVE);
            if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
                gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), 0.0, SEEK_RELATIVE);
                idledata->position -= 600;
                if (idledata->position < 0)
                    idledata->position = 0;
                gmtk_media_tracker_set_percentage(tracker, idledata->position / idledata->length);
            }
            return FALSE;
        case GDK_Up:
            if (title_is_menu) {
                return gmtk_media_player_send_key_press_event(GMTK_MEDIA_PLAYER(media), event, data);
            } else {
                if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP
                    && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist)))
                    gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), 60.0, SEEK_RELATIVE);
                if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
                    gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), 0.0, SEEK_RELATIVE);
                    idledata->position += 60;
                    if (idledata->position > idledata->length)
                        idledata->position = 0;
                    gmtk_media_tracker_set_percentage(tracker, idledata->position / idledata->length);
                }
            }

            return FALSE;
        case GDK_Down:
            if (title_is_menu) {
                return gmtk_media_player_send_key_press_event(GMTK_MEDIA_PLAYER(media), event, data);
            } else {
                if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP
                    && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist)))
                    gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), -60.0, SEEK_RELATIVE);
                if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
                    gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), 0.0, SEEK_RELATIVE);
                    idledata->position -= 60;
                    if (idledata->position < 0)
                        idledata->position = 0;
                    gmtk_media_tracker_set_percentage(tracker, idledata->position / idledata->length);
                }
            }
            return FALSE;
        case GDK_Return:
        case GDK_KP_Enter:
            if (title_is_menu) {
                return gmtk_media_player_send_key_press_event(GMTK_MEDIA_PLAYER(media), event, data);
            } else if (idledata->videopresent) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);
            }
            return FALSE;
        case GDK_less:
            prev_callback(NULL, NULL, NULL);
            return FALSE;
        case GDK_greater:
            next_callback(NULL, NULL, NULL);
            return FALSE;
        case GDK_space:
        case GDK_p:
            return play_callback(NULL, NULL, NULL);
            break;
        case GDK_m:
            if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED)) {
                gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED, FALSE);
                gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider), audio_device.volume);
            } else {
                gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED, TRUE);
                gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider), 0.0);
            }
            return FALSE;

        case GDK_1:
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_CONTRAST, -5);
            return FALSE;
        case GDK_2:
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_CONTRAST, 5);
            return FALSE;
        case GDK_3:
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_BRIGHTNESS, -5);
            return FALSE;
        case GDK_4:
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_BRIGHTNESS, 5);
            return FALSE;
        case GDK_5:
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HUE, -5);
            return FALSE;
        case GDK_6:
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HUE, 5);
            return FALSE;
        case GDK_7:
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SATURATION, -5);
            return FALSE;
        case GDK_8:
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SATURATION, 5);
            return FALSE;
        case GDK_bracketleft:
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SPEED_MULTIPLIER, 0.9091);
            return FALSE;
        case GDK_bracketright:
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SPEED_MULTIPLIER, 1.10);
            return FALSE;
        case GDK_BackSpace:
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SPEED_SET, 1.0);
            return FALSE;
        case GDK_9:
            gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider),
                                       gtk_scale_button_get_value(GTK_SCALE_BUTTON(vol_slider)) - 0.01);
            return FALSE;
        case GDK_0:
            gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider),
                                       gtk_scale_button_get_value(GTK_SCALE_BUTTON(vol_slider)) + 0.01);
            return FALSE;
        case GDK_period:
            if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE)
                gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_FRAME_STEP);
            return FALSE;
        case GDK_j:
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SUBTITLE_SELECT);
            return FALSE;
        case GDK_y:
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SUBTITLE_STEP_FORWARD);
            return FALSE;
        case GDK_g:
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SUBTITLE_STEP_BACKWARD);
            return FALSE;
        case GDK_q:
            delete_callback(NULL, NULL, NULL);
            g_idle_add(set_destroy, NULL);
            return FALSE;
        case GDK_v:
            /*
               if (fullscreen) {
               gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUB_VISIBLE,
               !gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER
               (media),
               ATTRIBUTE_SUB_VISIBLE));
               }
             */
            return FALSE;
        case GDK_KP_Add:
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_DELAY,
                                                   gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media),
                                                                                          ATTRIBUTE_AUDIO_DELAY) + 0.1);

            return FALSE;
        case GDK_minus:
        case GDK_KP_Subtract:
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_DELAY,
                                                   gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media),
                                                                                          ATTRIBUTE_AUDIO_DELAY) - 0.1);
            return FALSE;
            //case GDK_z:
            //    menuitem_view_decrease_subtitle_delay_callback(NULL, NULL);
            //    return FALSE;
            //case GDK_x:
            //    menuitem_view_increase_subtitle_delay_callback(NULL, NULL);
            //    return FALSE;
        case GDK_F11:
            if (idledata->videopresent)
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);
            return FALSE;
        case GDK_Escape:
            if (fullscreen) {
                if (idledata->videopresent)
                    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);
            } else {
                delete_callback(NULL, NULL, NULL);
                g_idle_add(set_destroy, NULL);
            }
            return FALSE;
        case GDK_f:
            if (idledata->videopresent)
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);
            return FALSE;
        case GDK_a:
            /*
               if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_follow_window))) {
               gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_default), TRUE);
               return FALSE;
               }
               if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_ten)))
               gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_follow_window), TRUE);
               if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_nine)))
               gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_ten), TRUE);
               if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_four_three)))
               gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_nine), TRUE);
               if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_default)))
               gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_four_three), TRUE);
             */
            return FALSE;
        case GDK_d:
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SWITCH_FRAME_DROP);
            return FALSE;
        case GDK_i:
            if (fullscreen) {
                //cmd = g_strdup_printf("osd_show_text '%s' 1500 0\n", idledata->display_name);
                //send_command(cmd, TRUE);
                //g_free(cmd);
            }
            return FALSE;
        case GDK_b:
            gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_POSITION,
                                                    gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media),
                                                                                            ATTRIBUTE_SUBTITLE_POSITION)
                                                    - 1);
            return FALSE;
        case GDK_s:
            // gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_TAKE_SCREENSHOT);
            return FALSE;
        default:
            gmtk_media_player_send_key_press_event(GMTK_MEDIA_PLAYER(media), event, data);
            return FALSE;
        }
    }

    if (!event->is_modifier && (event->state & GDK_SHIFT_MASK) == 1 && (event->state & GDK_CONTROL_MASK) == 0
        && (event->state & GDK_MOD1_MASK) == 0) {
        g_get_current_time(&currenttime);
        last_movement_time = currenttime.tv_sec;

        title_is_menu = gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_TITLE_IS_MENU);

        g_idle_add(make_panel_and_mouse_visible, NULL);
        switch (event->keyval) {
        case GDK_B:
            gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_POSITION,
                                                    gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media),
                                                                                            ATTRIBUTE_SUBTITLE_POSITION)
                                                    + 1);
            return FALSE;
        case GDK_S:
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_TAKE_SCREENSHOT);
            return FALSE;
        case GDK_J:
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SUBTITLE_SELECT);
            return FALSE;
        case GDK_numbersign:
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SWITCH_AUDIO);
            return FALSE;
        case GDK_plus:
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_DELAY,
                                                   gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media),
                                                                                          ATTRIBUTE_AUDIO_DELAY) + 0.1);
            return FALSE;
        case GDK_braceleft:
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SPEED_MULTIPLIER, 0.50);
            return FALSE;
        case GDK_braceright:
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SPEED_MULTIPLIER, 2.0);
            return FALSE;
        default:
            gmtk_media_player_send_key_press_event(GMTK_MEDIA_PLAYER(media), event, data);
            return FALSE;
        }

    }

    if ((fullscreen == 1)
        && (!event->is_modifier && (event->state & GDK_SHIFT_MASK) == 0 && (event->state & GDK_CONTROL_MASK) == 0
            && (event->state & GDK_MOD1_MASK) == 0)) {
        switch (event->keyval) {
        case GDK_f:
            idledata->fullscreen = FALSE;
            set_fullscreen(idledata);
            return TRUE;
        default:
            return FALSE;
        }
    }

    return FALSE;

}

gboolean drop_callback(GtkWidget * widget, GdkDragContext * dc,
                       gint x, gint y, GtkSelectionData * selection_data, guint info, guint t, gpointer data)
{
    gchar **list;
    gint i = 0;
    gboolean playlist;
    gint itemcount;
    gboolean added_single = FALSE;
    gchar *filename;
    /* Important, check if we actually got data.  Sometimes errors
     * occure and selection_data will be NULL.
     */
    if (selection_data == NULL)
        return FALSE;
    if (gtk_selection_data_get_length(selection_data) < 0)
        return FALSE;

    if ((info == DRAG_INFO_0) || (info == DRAG_INFO_1) || (info == DRAG_INFO_2)) {

        list = g_uri_list_extract_uris((const gchar *) gtk_selection_data_get_data(selection_data));
        itemcount = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL);

        while (list[i] != NULL) {
            if (strlen(list[i]) > 0) {
                if (is_uri_dir(list[i])) {
                    create_folder_progress_window();
                    add_folder_to_playlist_callback(list[i], NULL);
                    destroy_folder_progress_window();
                } else {
                    // subtitle?
                    if (g_strrstr(list[i], ".ass") != NULL || g_strrstr(list[i], ".ssa") != NULL
                        || g_strrstr(list[i], ".srt") != NULL) {
                        filename = g_filename_from_uri(list[i], NULL, NULL);
                        if (filename != NULL) {
                            gtk_list_store_set(playliststore, &iter, SUBTITLE_COLUMN, filename, -1);
                            gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_FILE,
                                                                   filename);
                            g_free(filename);
                        }
                    } else {
                        playlist = detect_playlist(list[i]);

                        if (!playlist) {
                            if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist))) {
                                if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
                                    dontplaynext = TRUE;
                                gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
                                gtk_list_store_clear(playliststore);
                                added_single = add_item_to_playlist(list[i], playlist);
                            } else {
                                add_item_to_playlist(list[i], playlist);
                            }
                        } else {
                            if (!parse_playlist(list[i])) {
                                if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist))) {
                                    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) !=
                                        MEDIA_STATE_UNKNOWN)
                                        dontplaynext = TRUE;
                                    gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
                                    gtk_list_store_clear(playliststore);
                                    added_single = add_item_to_playlist(list[i], playlist);
                                } else {
                                    add_item_to_playlist(list[i], playlist);
                                }
                            }
                        }
                    }
                }
            }
            i++;
        }

        if (itemcount == 0 || added_single) {
            gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter);
            gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
            gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_FILE);
            g_idle_add(async_play_iter, &iter);
        }

        g_strfreev(list);
        update_gui();
    }
    return TRUE;

}

gboolean pause_callback(GtkWidget * widget, GdkEventExpose * event, void *data)
{
    return play_callback(widget, event, data);
}

gboolean play_callback(GtkWidget * widget, GdkEventExpose * event, void *data)
{
    autostart = 1;
    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_STOP ||
        gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_PLAY);

    } else if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PLAY) {
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_PAUSE);
    }

    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_UNKNOWN) {
        if (next_item_in_playlist(&iter)) {
            gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_FILE);
            g_idle_add(async_play_iter, &iter);
        } else {
            if (is_first_item_in_playlist(&iter)) {
                gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_FILE);
                g_idle_add(async_play_iter, &iter);
            }
        }
    }

    return FALSE;
}

gboolean stop_callback(GtkWidget * widget, GdkEventExpose * event, void *data)
{
#ifdef GTK3_ENABLED
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    const gchar *icon_start;

    if (gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL)
        icon_start = "media-playback-start-rtl-symbolic";
    else
        icon_start = "media-playback-start-symbolic";
#endif

    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PLAY ||
        gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_STOP);
        autopause = FALSE;
        if (gmtk_media_player_get_media_type(GMTK_MEDIA_PLAYER(media)) == TYPE_NETWORK)
            dontplaynext = TRUE;
        gmtk_media_tracker_set_percentage(tracker, 0.0);
        gtk_widget_set_sensitive(play_event_box, TRUE);
#ifdef GTK3_ENABLED
        if (gtk_icon_theme_has_icon(icon_theme, icon_start)) {
            gtk_image_set_from_icon_name(GTK_IMAGE(image_play), icon_start, button_size);
        } else {
            gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
        }
#else
        gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
#endif
        gtk_widget_set_tooltip_text(play_event_box, _("Play"));
    }

    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_QUIT) {
        gmtk_media_tracker_set_percentage(tracker, 0.0);
#ifdef GTK3_ENABLED
        if (gtk_icon_theme_has_icon(icon_theme, icon_start)) {
            gtk_image_set_from_icon_name(GTK_IMAGE(image_play), icon_start, button_size);
        } else {
            gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
        }
#else
        gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
#endif
        gtk_widget_set_tooltip_text(play_event_box, _("Play"));
    }

    g_idle_add(set_progress_value, idledata);
    g_strlcpy(idledata->progress_text, _("Stopped"), sizeof(idledata->progress_text));
    g_idle_add(set_progress_text, idledata);

    return FALSE;
}

gboolean ff_callback(GtkWidget * widget, GdkEventExpose * event, void *data)
{
    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP) {
        gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), 10.0, SEEK_RELATIVE);

        if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
            gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), 0.0, SEEK_RELATIVE);
            idledata->position += 10;
            if (idledata->position > idledata->length)
                idledata->position = 0;
            gmtk_media_tracker_set_percentage(tracker, idledata->position / idledata->length);
        }
    }

    return FALSE;
}

gboolean rew_callback(GtkWidget * widget, GdkEventExpose * event, void *data)
{
    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP) {
        gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), -10.0, SEEK_RELATIVE);
        if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
            gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), 0.0, SEEK_RELATIVE);
            idledata->position -= 10;
            if (idledata->position < 0)
                idledata->position = 0;
            gmtk_media_tracker_set_percentage(tracker, idledata->position / idledata->length);
        }
    }

    return FALSE;
}

gboolean prev_callback(GtkWidget * widget, GdkEventExpose * event, void *data)
{
    gboolean valid = FALSE;
    GtkTreePath *path;
    GtkTreeIter previter;
    GtkTreeIter localiter;
#ifdef GTK3_ENABLED
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    const gchar *icon_start;

    if (gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL)
        icon_start = "media-playback-start-rtl-symbolic";
    else
        icon_start = "media-playback-start-symbolic";
#endif

    if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
        if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HAS_CHAPTERS)) {
            valid = FALSE;
            gmtk_media_player_seek_chapter(GMTK_MEDIA_PLAYER(media), -1, SEEK_RELATIVE);
        } else {
            valid = prev_item_in_playlist(&iter);
        }
    } else {
        // for the case where we have rolled off the end of the list, allow prev to work
        gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &localiter);
        do {
            previter = localiter;
            valid = TRUE;
            gtk_tree_model_iter_next(GTK_TREE_MODEL(playliststore), &localiter);
        } while (gtk_list_store_iter_is_valid(playliststore, &localiter));
        iter = previter;

        if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HAS_CHAPTERS)) {
            valid = FALSE;
            gmtk_media_player_seek_chapter(GMTK_MEDIA_PLAYER(media), -1, SEEK_RELATIVE);
        }
    }

    if (valid) {
        if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
            dontplaynext = TRUE;
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
        gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_FILE);
        g_idle_add(async_play_iter, &iter);
        if (autopause) {
            autopause = FALSE;
            gtk_widget_set_sensitive(play_event_box, TRUE);
#ifdef GTK3_ENABLED
            if (gtk_icon_theme_has_icon(icon_theme, icon_start)) {
                gtk_image_set_from_icon_name(GTK_IMAGE(image_play), icon_start, button_size);
            } else {
                gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
            }
#else
            gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
#endif
        }
        gtk_widget_set_sensitive(ff_event_box, TRUE);
        gtk_widget_set_sensitive(rew_event_box, TRUE);

    }

    if (GTK_IS_TREE_SELECTION(selection)) {
        if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
            path = gtk_tree_model_get_path(GTK_TREE_MODEL(playliststore), &iter);
            gtk_tree_selection_select_path(selection, path);
            if (GTK_IS_WIDGET(list))
                gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(list), path, NULL, FALSE, 0, 0);
            gtk_tree_path_free(path);
        }
    }

    return FALSE;
}

gboolean next_callback(GtkWidget * widget, GdkEventExpose * event, void *data)
{
    gboolean valid = FALSE;
    GtkTreePath *path;
#ifdef GTK3_ENABLED
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    const gchar *icon_start;

    if (gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL)
        icon_start = "media-playback-start-rtl-symbolic";
    else
        icon_start = "media-playback-start-symbolic";
#endif

    if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
        if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HAS_CHAPTERS)) {
            gmtk_media_player_seek_chapter(GMTK_MEDIA_PLAYER(media), 1, SEEK_RELATIVE);
        } else {
            valid = next_item_in_playlist(&iter);
        }
    } else {
        if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HAS_CHAPTERS)) {
            gmtk_media_player_seek_chapter(GMTK_MEDIA_PLAYER(media), 1, SEEK_RELATIVE);
        }
    }

    if (valid) {
        if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
            dontplaynext = TRUE;
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
        gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_FILE);
        g_idle_add(async_play_iter, &iter);
        if (autopause) {
            autopause = FALSE;
            gtk_widget_set_sensitive(play_event_box, TRUE);
#ifdef GTK3_ENABLED
            if (gtk_icon_theme_has_icon(icon_theme, icon_start)) {
                gtk_image_set_from_icon_name(GTK_IMAGE(image_play), icon_start, button_size);
            } else {
                gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
            }
#else
            gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
#endif
        }
        gtk_widget_set_sensitive(ff_event_box, TRUE);
        gtk_widget_set_sensitive(rew_event_box, TRUE);

    }

    if (GTK_IS_TREE_SELECTION(selection)) {
        if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
            path = gtk_tree_model_get_path(GTK_TREE_MODEL(playliststore), &iter);
            gtk_tree_selection_select_path(selection, path);
            if (GTK_IS_WIDGET(list))
                gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(list), path, NULL, FALSE, 0, 0);
            gtk_tree_path_free(path);
        }
    }

    return FALSE;
}

gboolean menu_callback(GtkWidget * widget, GdkEventExpose * event, void *data)
{
    gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SHOW_DVD_MENU);
    return FALSE;
}

void vol_slider_callback(GtkRange * range, gpointer user_data)
{
    gdouble vol = gtk_range_get_value(range);
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "vol_slider_callback new volume is %f", audio_device.volume);
    if (softvol || audio_device.type == AUDIO_TYPE_SOFTVOL) {
        if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED) && vol > 0) {
            gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED, FALSE);
        }
        if ((vol * 100) == 0) {
            gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED, TRUE);
        } else {
            gmtk_media_player_set_volume(GMTK_MEDIA_PLAYER(media), gtk_range_get_value(range));
        }

        if (remember_softvol) {
            volume_softvol = vol;
            set_software_volume(&volume_softvol);
        }
    } else {
        if (!gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED))
            gm_audio_set_volume(&audio_device, gtk_range_get_value(range));
    }

}

void vol_button_value_changed_callback(GtkScaleButton * volume, gdouble value, gpointer data)
{

    if (softvol || audio_device.type == AUDIO_TYPE_SOFTVOL) {
        if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED) && value > 0) {
            gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED, FALSE);
        }
        if ((gint) (value * 100) == 0) {
            gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED, TRUE);
        } else {
            gmtk_media_player_set_volume(GMTK_MEDIA_PLAYER(media), value);
        }

        if (remember_softvol) {
            volume_softvol = value;
            set_software_volume(&volume_softvol);
        }

    } else {
        if (!gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED))
            gm_audio_set_volume(&audio_device, value);
    }

}

gboolean make_panel_and_mouse_invisible(gpointer data)
{
    GTimeVal currenttime;
    GdkCursor *cursor;
#if GTK_CHECK_VERSION(3, 0, 0)
    cairo_surface_t *s;
#endif
    GdkPixbuf *cursor_pixbuf;

    if ((fullscreen || always_hide_after_timeout) && auto_hide_timeout > 0
        && (gtk_widget_get_visible(controls_box) || fs_controls != NULL)
        && mouse_over_controls == FALSE) {
        g_get_current_time(&currenttime);
        g_time_val_add(&currenttime, -auto_hide_timeout * G_USEC_PER_SEC);
        if (last_movement_time > 0 && currenttime.tv_sec > last_movement_time) {
            hide_fs_controls();
        }

    }

    g_get_current_time(&currenttime);
    g_time_val_add(&currenttime, -auto_hide_timeout * G_USEC_PER_SEC);
    /*
       printf("%i, %i, %i, %i, %i, %i\n",   currenttime.tv_sec,
       get_visible(menu_file),
       get_visible(menu_edit),
       get_visible(menu_view),
       get_visible(menu_help),
       gtk_tree_view_get_enable_search(GTK_TREE_VIEW(list)));
     */
    if (gtk_widget_get_visible(GTK_WIDGET(menu_file))
        || gtk_widget_get_visible(GTK_WIDGET(menu_edit))
        || gtk_widget_get_visible(GTK_WIDGET(menu_view))
        || gtk_widget_get_visible(GTK_WIDGET(menu_help))
        || gtk_tree_view_get_enable_search(GTK_TREE_VIEW(list))) {

        gdk_window_set_cursor(gtk_widget_get_window(window), NULL);

    } else {

        if (last_movement_time > 0 && currenttime.tv_sec > last_movement_time) {
#if GTK_CHECK_VERSION(3, 0, 0)
            s = cairo_image_surface_create(CAIRO_FORMAT_A1, 1, 1);
            cursor_pixbuf = gdk_pixbuf_get_from_surface(s, 0, 0, 1, 1);
            cairo_surface_destroy(s);
#else
            cursor_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 1, 1);
#endif
            cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), cursor_pixbuf, 0, 0);
            g_object_unref(cursor_pixbuf);
            gdk_window_set_cursor(gtk_widget_get_window(window), cursor);
#if GTK_CHECK_VERSION(3, 0, 0)
            g_object_unref(cursor);
#else
            // gtk2.24: g_object_unref segfaults..
            gdk_cursor_unref(cursor); // gdk_cursor_unref is not deprecated
#endif
        }
    }

    return FALSE;
}

gboolean make_panel_and_mouse_visible(gpointer data)
{

    if (showcontrols && GTK_IS_WIDGET(controls_box)) {
        show_fs_controls();
    }
    gdk_window_set_cursor(gtk_widget_get_window(window), NULL);

    return FALSE;
}

gboolean fs_callback(GtkWidget * widget, GdkEventExpose * event, void *data)
{
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);

    return FALSE;
}

gboolean fs_controls_entered(GtkWidget * widget, GdkEventCrossing * event, gpointer data)
{
    mouse_over_controls = TRUE;
    return FALSE;
}

gboolean fs_controls_left(GtkWidget * widget, GdkEventCrossing * event, gpointer data)
{
    mouse_over_controls = FALSE;
    return FALSE;
}


void menuitem_open_callback(GtkMenuItem * menuitem, void *data)
{

    GtkWidget *dialog;
    GSList *filename;
    gchar *last_dir;
    gint count;
    GtkTreeViewColumn *column;
    gchar *coltitle;

    dialog = gtk_file_chooser_dialog_new(_("Open File"),
                                         GTK_WINDOW(window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "gtk-cancel", GTK_RESPONSE_CANCEL,
                                         "gtk-open",   GTK_RESPONSE_ACCEPT, NULL);

    /*allow multiple files to be selected */
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), FALSE);

    gtk_widget_show(dialog);
    gm_store = gm_pref_store_new("g-mplayer");
    last_dir = gm_pref_store_get_string(gm_store, LAST_DIR);
    if (last_dir != NULL && is_uri_dir(last_dir)) {
        gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), last_dir);
        g_free(last_dir);
    }
    gm_pref_store_free(gm_store);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {

        filename = gtk_file_chooser_get_uris(GTK_FILE_CHOOSER(dialog));
        last_dir = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
        if (last_dir != NULL && is_uri_dir(last_dir)) {
            gm_store = gm_pref_store_new("g-mplayer");
            gm_pref_store_set_string(gm_store, LAST_DIR, last_dir);
            gm_pref_store_free(gm_store);
            g_free(last_dir);
        }
        if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
            dontplaynext = TRUE;
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
        gtk_list_store_clear(playliststore);

        if (filename != NULL) {
            g_slist_foreach(filename, &add_item_to_playlist_callback, NULL);
            g_slist_free(filename);

            gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter);
            gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_FILE);
            g_idle_add(async_play_iter, &iter);
        }
    }

    if (GTK_IS_WIDGET(list)) {
        column = gtk_tree_view_get_column(GTK_TREE_VIEW(list), 0);
        count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL);
        coltitle = g_strdup_printf(ngettext("Item to Play", "Items to Play", count));
        gtk_tree_view_column_set_title(column, coltitle);
        g_free(coltitle);
    }
    gtk_widget_destroy(dialog);

}

void open_location_callback(GtkWidget * widget, void *data)
{
    gchar *filename;

    filename = g_strdup(gtk_entry_get_text(GTK_ENTRY(open_location)));

    if (filename != NULL && strlen(filename) > 0) {
        if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
            dontplaynext = TRUE;
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
        gtk_list_store_clear(playliststore);

        if (filename != NULL) {

            playlist = detect_playlist(filename);

            if (!playlist) {
                add_item_to_playlist(filename, playlist);
            } else {
                if (!parse_playlist(filename)) {
                    add_item_to_playlist(filename, playlist);
                }

            }

            g_free(filename);
            gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter);
            gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
            gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_NETWORK);
            g_idle_add(async_play_iter, &iter);
        }
    }
    if (GTK_IS_WIDGET(widget))
        gtk_widget_destroy(widget);
}

void menuitem_open_location_callback(GtkMenuItem * menuitem, void *data)
{
    GtkWidget *open_window;
    GtkWidget *vbox;
    GtkWidget *item_box;
    GtkWidget *label;
    GtkWidget *button_box;
    GtkWidget *cancel_button;
    GtkWidget *open_button;

    open_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(open_window), GDK_WINDOW_TYPE_HINT_UTILITY);
    gtk_window_set_resizable(GTK_WINDOW(open_window), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(open_window), GTK_WINDOW(window));
    gtk_window_set_position(GTK_WINDOW(open_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_icon(GTK_WINDOW(open_window), pb_icon);

    gtk_window_set_resizable(GTK_WINDOW(open_window), FALSE);
    gtk_window_set_title(GTK_WINDOW(open_window), _("Open Location"));

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    label = gtk_label_new(_("Location:"));
    open_location = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(open_location), 50);
    gtk_entry_set_activates_default(GTK_ENTRY(open_location), TRUE);

    item_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(item_box), label, FALSE, FALSE, 12);
    gtk_box_pack_end(GTK_BOX(item_box), open_location, TRUE, TRUE, 0);

    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    cancel_button = gtk_button_new_from_stock("gtk-cancel");
    open_button = gtk_button_new_from_stock("gtk-open");
    gtk_widget_set_can_default(open_button, TRUE);
    gtk_box_pack_end(GTK_BOX(button_box), open_button, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(button_box), cancel_button, FALSE, FALSE, 0);

    g_signal_connect_swapped(G_OBJECT(cancel_button), "clicked", G_CALLBACK(config_close), open_window);
    g_signal_connect_swapped(G_OBJECT(open_button), "clicked", G_CALLBACK(open_location_callback), open_window);

    gtk_container_add(GTK_CONTAINER(vbox), item_box);
    gtk_container_add(GTK_CONTAINER(vbox), button_box);
    gtk_container_add(GTK_CONTAINER(open_window), vbox);
    gtk_widget_show_all(open_window);
    gtk_window_set_transient_for(GTK_WINDOW(open_window), GTK_WINDOW(window));
    gtk_window_set_keep_above(GTK_WINDOW(open_window), keep_on_top);
    gtk_window_present(GTK_WINDOW(open_window));
    gtk_widget_grab_default(open_button);

}

void menuitem_open_dvdnav_callback(GtkMenuItem * menuitem, void *data)
{
    gtk_list_store_clear(playliststore);
    if (idledata->device != NULL) {
        g_free(idledata->device);
        idledata->device = NULL;
    }
    gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
    add_item_to_playlist("dvdnav://", FALSE);
    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter);
    gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_DVD);
    gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MEDIA_DEVICE, mplayer_dvd_device);
    g_idle_add(async_play_iter, &iter);
    gtk_widget_show(menu_event_box);
}

void menuitem_open_dvdnav_folder_callback(GtkMenuItem * menuitem, void *data)
{
    GtkWidget *dialog;
    gchar *last_dir;

    dialog = gtk_file_chooser_dialog_new(_("Choose Disk Directory"),
                                         GTK_WINDOW(window),
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                         "gtk-cancel", GTK_RESPONSE_CANCEL,
                                         "gtk-open",   GTK_RESPONSE_ACCEPT, NULL);
    gtk_widget_show(dialog);
    gm_store = gm_pref_store_new("g-mplayer");
    last_dir = gm_pref_store_get_string(gm_store, LAST_DIR);
    if (last_dir != NULL && is_uri_dir(last_dir)) {
        gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), last_dir);
        g_free(last_dir);
    }
    gm_pref_store_free(gm_store);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {

        gtk_list_store_clear(playliststore);
        idledata->device = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));

        add_item_to_playlist("dvdnav://", FALSE);
        gtk_widget_show(menu_event_box);

        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter)) {
            gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
            gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MEDIA_DEVICE, idledata->device);
            gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_DVD);
            g_idle_add(async_play_iter, &iter);
        }
    }
    if (GTK_IS_WIDGET(dialog))
        gtk_widget_destroy(dialog);

}

void menuitem_open_dvdnav_iso_callback(GtkMenuItem * menuitem, void *data)
{
    GtkWidget *dialog;
    gchar *last_dir;
    GtkFileFilter *filter;

    dialog = gtk_file_chooser_dialog_new(_("Choose Disk Image"),
                                         GTK_WINDOW(window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "gtk-cancel", GTK_RESPONSE_CANCEL,
                                         "gtk-open",   GTK_RESPONSE_ACCEPT, NULL);
    gtk_widget_show(dialog);
    gm_store = gm_pref_store_new("g-mplayer");
    last_dir = gm_pref_store_get_string(gm_store, LAST_DIR);
    if (last_dir != NULL && is_uri_dir(last_dir)) {
        gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), last_dir);
        g_free(last_dir);
    }
    gm_pref_store_free(gm_store);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Disk Image (*.iso)"));
    gtk_file_filter_add_pattern(filter, "*.iso");
    gtk_file_filter_add_pattern(filter, "*.ISO");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {

        gtk_list_store_clear(playliststore);
        idledata->device = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));

        add_item_to_playlist("dvdnav://", FALSE);
        gtk_widget_show(menu_event_box);

        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter)) {
            gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
            gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MEDIA_DEVICE, idledata->device);
            gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_DVD);
            g_idle_add(async_play_iter, &iter);
        }
    }
    if (GTK_IS_WIDGET(dialog))
        gtk_widget_destroy(dialog);

}

void menuitem_open_acd_callback(GtkMenuItem * menuitem, void *data)
{
    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
        dontplaynext = TRUE;
    gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
    gtk_list_store_clear(playliststore);
    parse_playlist("cdda://");

    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter)) {
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
        gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MEDIA_DEVICE, mplayer_dvd_device);
        gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_CD);
        g_idle_add(async_play_iter, &iter);
    }

}

void menuitem_open_vcd_callback(GtkMenuItem * menuitem, void *data)
{
    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
        dontplaynext = TRUE;
    gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
    gtk_list_store_clear(playliststore);
    parse_playlist("vcd://");

    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter)) {
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
        gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MEDIA_DEVICE, mplayer_dvd_device);
        gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_VCD);
        g_idle_add(async_play_iter, &iter);
    }

}

void menuitem_open_atv_callback(GtkMenuItem * menuitem, void *data)
{
    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
        dontplaynext = TRUE;
    gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
    gtk_list_store_clear(playliststore);
    add_item_to_playlist("tv://", FALSE);

    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter)) {
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
        gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_TV);
        g_idle_add(async_play_iter, &iter);
    }
}

void menuitem_open_recent_callback(GtkRecentChooser * chooser, gpointer data)
{
    gboolean playlist = FALSE;
    gchar *uri;
    gint count;
    GtkTreeViewColumn *column;
    gchar *coltitle;

    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
        dontplaynext = TRUE;
    gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
    gtk_list_store_clear(playliststore);

    uri = gtk_recent_chooser_get_current_uri(chooser);
    if (uri != NULL) {
        if (playlist == FALSE)
            playlist = detect_playlist(uri);

        if (!playlist) {
            add_item_to_playlist(uri, playlist);
        } else {
            if (!parse_playlist(uri)) {
                add_item_to_playlist(uri, playlist);
            }
        }
    }

    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter);

    if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
        if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
            dontplaynext = TRUE;
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
        gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_FILE);
        g_idle_add(async_play_iter, &iter);
    }

    if (GTK_IS_WIDGET(list)) {
        column = gtk_tree_view_get_column(GTK_TREE_VIEW(list), 0);
        count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL);
        coltitle = g_strdup_printf(ngettext("Item to Play", "Items to Play", count));
        gtk_tree_view_column_set_title(column, coltitle);
        g_free(coltitle);
    }
}

void parseChannels(FILE * f)
{
    gint parsing = 0, i = 0, firstW = 0, firstP = 0;
    gchar ch, s[256];
    gchar *strout;

    while (parsing == 0) {
        ch = (char) fgetc(f);   // Read in the next character
        if ((int) ch != EOF) {
            // If the line is empty or commented, we want to skip it.
            if (((ch == '\n') && (i == 0)) || ((ch == '#') && (i == 0))) {
                firstW++;
                firstP++;
            }
            if ((ch != ':') && (firstW == 0) && i < 255) {
                s[i] = ch;
                i++;
            } else {
                if ((ch == ':') && (firstP == 0)) {
                    s[i] = '\0';
                    strout = g_strdup_printf("dvb://%s", s);
                    add_item_to_playlist(strout, FALSE);        //add to playlist
                    g_free(strout);
                    i = 0;
                    firstW++;
                    firstP++;
                }
                if (ch == '\n') {
                    firstW = 0;
                    firstP = 0;
                }
            }
        } else
            parsing++;
    }                           //END while
}                               //END parseChannels

void menuitem_open_dtv_callback(GtkMenuItem * menuitem, void *data)
{
    FILE *fi;                   // FILE pointer to use to open the conf file
    gchar *mpconf;

    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_UNKNOWN)
        dontplaynext = TRUE;
    gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
    gtk_list_store_clear(playliststore);

    mpconf = g_strdup_printf("%s/.mplayer/channels.conf", g_getenv("HOME"));
    fi = fopen(mpconf, "r");    // Make sure this is pointing to
    // the appropriate file
    if (fi != NULL) {
        parseChannels(fi);
        fclose(fi);
    } else {
        gm_log(verbose, G_LOG_LEVEL_MESSAGE, "Unable to open the config file"); //can change this to whatever error message system is used
    }
    g_free(mpconf);

    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter);
    if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_QUIT);
        gmtk_media_player_set_media_type(GMTK_MEDIA_PLAYER(media), TYPE_TV);
        g_idle_add(async_play_iter, &iter);
    }
}


void menuitem_quit_callback(GtkMenuItem * menuitem, void *data)
{
    delete_callback(NULL, NULL, NULL);
    gtk_widget_destroy(window);
}

void menuitem_prev_callback(GtkMenuItem * menuitem, void *data)
{
    prev_callback(NULL, NULL, NULL);
}

void menuitem_next_callback(GtkMenuItem * menuitem, void *data)
{
    next_callback(NULL, NULL, NULL);
}

void about_url_hook(GtkAboutDialog * about, const char *link, gpointer data)
{
    GError *error = NULL;
    if (!gtk_show_uri(gtk_widget_get_screen(GTK_WIDGET(about)), link, gtk_get_current_event_time(), &error)) {
        g_error_free(error);
    }
}

void menuitem_about_callback(GtkMenuItem * menuitem, void *data)
{
    gchar *authors[] = {
        "Kevin DeKorte - main author",
        "James Carthew",
        "Diogo Franco",
        "Icons provided by Victor Castillejo",
        NULL
    };
    gtk_show_about_dialog(GTK_WINDOW(window),
                          "authors", authors,
                          "copyright", "Copyright © 2007-2020",
                          "comments", "GTK front-end for MPlayer",
                          "version", VERSION,
                          "license", "This program is free software; you can redistribute it and/or\nmodify it under the terms of the GNU General Public License\nas published by the Free Software Foundation; either version 2\nof the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with this program; if not, write to the Free Software\nFoundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.",
                          "website", "https://github.com/wdlkmpx/gtk-mplayer",
                          "translator-credits",
                          "Bulgarian - Adrian Dimitrov\n"
                          "Czech - Petr Pisar\n"
                          "Chinese (simplified) - Wenzheng Hu\n"
                          "Chinese (Hong Kong) - Hialan Liu\n"
                          "Chinese (Taiwan) - Hailan Liu\n"
                          "Dutch - Mark Huijgen\n"
                          "Finnish - Kristian Polso &amp; Tuomas Lähteenmäki\n"
                          "French - Alexandre Bedot\n"
                          "German - Tim Buening\n"
                          "Greek - Γεώργιος Γεωργάς\n"
                          "Hungarian - Kulcsár Kázmér\n"
                          "Italian - Cesare Tirabassi\n"
                          "Japanese - Munehiro Yamamoto\n"
                          "Korean - ByeongSik Jeon\n"
                          "Lithuanian - Mindaugas B.\n"
                          "Polish - Julian Sikorski\n"
                          "Portugese - LL and Sérgio Marques\n"
                          "Russian - Dmitry Stropaloff and Denis Koryavov\n"
                          "Serbian - Милош Поповић\n"
                          "Spanish - Festor Wailon Dacoba\n"
                          "Swedish - Daniel Nylander\n" "Turkish - Onur Küçük",
                          NULL);
}

void menuitem_play_callback(GtkMenuItem * menuitem, void *data)
{
    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_PLAY)
        play_callback(GTK_WIDGET(menuitem), NULL, NULL);
}

void menuitem_pause_callback(GtkMenuItem * menuitem, void *data)
{
    pause_callback(GTK_WIDGET(menuitem), NULL, NULL);
}

void menuitem_stop_callback(GtkMenuItem * menuitem, void *data)
{
    stop_callback(GTK_WIDGET(menuitem), NULL, NULL);
}

void menuitem_edit_random_callback(GtkMenuItem * menuitem, void *data)
{
    GtkTreePath *path;
    gchar *iterfilename = NULL;
    gchar *localfilename = NULL;

    if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
        gtk_tree_model_get(GTK_TREE_MODEL(playliststore), &iter, ITEM_COLUMN, &iterfilename, -1);
    }

    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(playliststore), -2, GTK_SORT_ASCENDING);

    random_order = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_edit_random));
    if (random_order) {
        randomize_playlist(playliststore);
    } else {
        reset_playlist_order(playliststore);
    }

    if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
        if (GTK_IS_TREE_SELECTION(selection)) {
            gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore), &iter);
            if (iterfilename != NULL) {
                do {
                    gtk_tree_model_get(GTK_TREE_MODEL(playliststore), &iter, ITEM_COLUMN, &localfilename, -1);
                    gm_log(verbose, G_LOG_LEVEL_DEBUG, "iter = %s   local = %s", iterfilename, localfilename);
                    if (g_ascii_strcasecmp(iterfilename, localfilename) == 0) {
                        // we found the current iter
                        g_free(localfilename);
                        break;
                    }
                    g_free(localfilename);
                } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(playliststore), &iter));
                g_free(iterfilename);
            }
            path = gtk_tree_model_get_path(GTK_TREE_MODEL(playliststore), &iter);
            gtk_tree_selection_select_path(selection, path);
            if (GTK_IS_WIDGET(list))
                gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(list), path, NULL, FALSE, 0, 0);
            gtk_tree_path_free(path);
        }
    }
}

void menuitem_edit_loop_callback(GtkMenuItem * menuitem, void *data)
{
    loop = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_edit_loop));
}

void menuitem_view_info_callback(GtkMenuItem * menuitem, void *data)
{

    if (GTK_IS_WIDGET(media_hbox)) {
        if (!gtk_widget_get_realized(media_hbox))
            gtk_widget_realize(media_hbox);
        g_idle_add(set_adjust_layout, NULL);
        // adjust_layout();
    }

}

void menuitem_view_fullscreen_callback(GtkMenuItem * menuitem, void *data)
{
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);
}

void menuitem_view_onetoone_callback(GtkMenuItem * menuitem, void *data)
{
    IdleData *idle = (IdleData *) data;

    idle->width = (gint) gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_WIDTH);
    idle->height = (gint) gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HEIGHT);
    non_fs_width = 0;
    non_fs_height = 0;
    resize_window(idle);

}

void menuitem_view_twotoone_callback(GtkMenuItem * menuitem, void *data)
{
    IdleData *idle = (IdleData *) data;

    idle->width = (gint) gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_WIDTH) * 2;
    idle->height = (gint) gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HEIGHT) * 2;
    non_fs_width = 0;
    non_fs_height = 0;
    resize_window(idle);
}

void menuitem_view_onetoonepointfive_callback(GtkMenuItem * menuitem, void *data)
{
    IdleData *idle = (IdleData *) data;

    idle->width = (gint) gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_WIDTH) * 1.5;
    idle->height = (gint) gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HEIGHT) * 1.5;
    non_fs_width = 0;
    non_fs_height = 0;
    resize_window(idle);
}

void menuitem_view_onetotwo_callback(GtkMenuItem * menuitem, void *data)
{
    IdleData *idle = (IdleData *) data;

    idle->width = (gint) gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_WIDTH) / 2;
    idle->height = (gint) gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HEIGHT) / 2;
    non_fs_width = 0;
    non_fs_height = 0;
    resize_window(idle);
}

void menuitem_view_controls_callback(GtkMenuItem * menuitem, void *data)
{
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_showcontrols),
                                   !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_showcontrols)));
}

void menuitem_view_subtitles_callback(GtkMenuItem * menuitem, void *data)
{
    gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUB_VISIBLE,
                                            gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM
                                                                           (menuitem_view_subtitles)));
}

//      Switch Audio Streams
void menuitem_edit_switch_audio_callback(GtkMenuItem * menuitem, void *data)
{
    gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SWITCH_AUDIO);
}

void menuitem_edit_set_audiofile_callback(GtkMenuItem * menuitem, void *data)
{
    gchar *audiofile = NULL;
    GtkWidget *dialog;
    gchar *path;
    gchar *item;
    gchar *p;

    if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
        gtk_tree_model_get(GTK_TREE_MODEL(playliststore), &iter, ITEM_COLUMN, &item, -1);

        path = g_strdup(item);
        p = g_strrstr(path, "/");
        if (p != NULL)
            p[1] = '\0';

        dialog = gtk_file_chooser_dialog_new(_("Set AudioFile"),
                                             GTK_WINDOW(window),
                                             GTK_FILE_CHOOSER_ACTION_OPEN,
                                             "gtk-cancel", GTK_RESPONSE_CANCEL,
                                             "gtk-open",   GTK_RESPONSE_ACCEPT, NULL);
        gtk_widget_show(dialog);
        gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), path);
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            audiofile = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            gtk_list_store_set(playliststore, &iter, AUDIOFILE_COLUMN, audiofile, -1);
            gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_TRACK_FILE, audiofile);
        }
        gtk_widget_destroy(dialog);

        if (audiofile != NULL) {
            gmtk_media_player_restart(GMTK_MEDIA_PLAYER(media));
        }
    }
}

void menuitem_edit_set_subtitle_callback(GtkMenuItem * menuitem, void *data)
{
    gchar *subtitle = NULL;
    GtkWidget *dialog;
    gchar *path;
    gchar *item;
    gchar *p;

    if (gtk_list_store_iter_is_valid(playliststore, &iter)) {
        gtk_tree_model_get(GTK_TREE_MODEL(playliststore), &iter, ITEM_COLUMN, &item, -1);

        path = g_strdup(item);
        p = g_strrstr(path, "/");
        if (p != NULL)
            p[1] = '\0';

        dialog = gtk_file_chooser_dialog_new(_("Set Subtitle"),
                                             GTK_WINDOW(window),
                                             GTK_FILE_CHOOSER_ACTION_OPEN,
                                             "gtk-cancel", GTK_RESPONSE_CANCEL,
                                             "gtk-open",   GTK_RESPONSE_ACCEPT, NULL);
        gtk_widget_show(dialog);
        gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), path);
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            subtitle = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            gtk_list_store_set(playliststore, &iter, SUBTITLE_COLUMN, subtitle, -1);
        }
        gtk_widget_destroy(dialog);

        if (subtitle != NULL) {
            gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_FILE, subtitle);
        }
    }
}

//      Take Screenshot
void menuitem_edit_take_screenshot_callback(GtkMenuItem * menuitem, void *data)
{
    gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_TAKE_SCREENSHOT);
}

void menuitem_fs_callback(GtkMenuItem * menuitem, void *data)
{
    static gboolean restore_playlist;
    static gboolean restore_details;
    static gboolean restore_info;

    if (GTK_CHECK_MENU_ITEM(menuitem) == GTK_CHECK_MENU_ITEM(menuitem_view_fullscreen)) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen),
                                       gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_fullscreen)));
        return;
    } else {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_fullscreen),
                                       gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen)));
    }

    if (fullscreen) {
        hide_fs_controls();

        skip_fixed_allocation_on_show = TRUE;
        gtk_window_unfullscreen(GTK_WINDOW(window));
        gtk_widget_show(menubar);
        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_controls))) {
            gtk_widget_show(controls_box);
        }
        if (restore_details) {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_details), TRUE);
        }
        if (restore_info) {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_info), TRUE);
        }
        if (restore_playlist) {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist), TRUE);
        }
        skip_fixed_allocation_on_show = FALSE;
        gtk_window_resize(GTK_WINDOW(window), last_window_width, last_window_height);

    } else {
        gtk_window_get_size(GTK_WINDOW(window), &last_window_width, &last_window_height);
        gtk_widget_hide(menubar);
        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_controls))) {
            gtk_widget_hide(controls_box);
        }
        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_details))) {
            restore_details = TRUE;
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_details), FALSE);
        } else {
            restore_details = FALSE;
        }
        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_info))) {
            restore_info = TRUE;
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_info), FALSE);
        } else {
            restore_info = FALSE;
        }
        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist))) {
            restore_playlist = TRUE;
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist), FALSE);
        } else {
            restore_playlist = FALSE;
        }

        // --fullscreen option doesn' t work without this event flush gm_log(verbose, G_LOG_LEVEL_DEBUG, "waiting for all events to drain");
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
        gtk_window_fullscreen(GTK_WINDOW(window));
    }

    fullscreen = !fullscreen;
    if (!fullscreen) {
        hide_fs_controls();
        gtk_window_present(GTK_WINDOW(window));
    }

}

void menuitem_copyurl_callback(GtkMenuItem * menuitem, void *data)
{
    GtkClipboard *clipboard;
    gchar *url, *s;

    url = g_strdup(gmtk_media_player_get_uri(GMTK_MEDIA_PLAYER(media)));
    s = url;
    if (strncmp(url, "file://", 7) == 0) {
        s = url + 7;
    }
    clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    gtk_clipboard_set_text(clipboard, s, -1);
    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, s, -1);

    g_free(url);
}

void menuitem_showcontrols_callback(GtkCheckMenuItem * menuitem, void *data)
{
    int width, height;
    GtkAllocation alloc;

    if (GTK_CHECK_MENU_ITEM(menuitem) == GTK_CHECK_MENU_ITEM(menuitem_view_controls)) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_showcontrols),
                                       gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_controls)));
        return;
    } else {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_controls),
                                       gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_showcontrols)));
    }

    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_showcontrols))) {
        if (GTK_IS_WIDGET(button_event_box)) {
            gtk_widget_hide(button_event_box);
        }

        if (fullscreen) {
            show_fs_controls();
        } else {
            gtk_widget_set_size_request(controls_box, -1, -1);
            gtk_widget_show(controls_box);
            if (!fullscreen) {
                gtk_window_get_size(GTK_WINDOW(window), &width, &height);
                gtk_widget_get_allocation(controls_box, &alloc);
                gtk_window_resize(GTK_WINDOW(window), width, height + alloc.height);
            }
        }

        showcontrols = TRUE;
    } else {
        if (fullscreen) {
            hide_fs_controls();
        } else {
            gtk_widget_hide(controls_box);
            if (!fullscreen) {
                gtk_window_get_size(GTK_WINDOW(window), &width, &height);
                gtk_widget_get_allocation(controls_box, &alloc);
                gtk_window_resize(GTK_WINDOW(window), width, height - alloc.height);
            }
        }
        showcontrols = FALSE;
    }
}

void config_apply(GtkWidget * widget, void *data)
{
    gint oldosd;
    gboolean old_disable_framedrop;
    GdkColor sub_color;

    if (vo != NULL) {
        g_free(vo);
        vo = NULL;
    }
    vo = g_strdup(gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(config_vo)))));

    audio_device_name = g_strdup(gmtk_output_combo_box_get_active_description(GMTK_OUTPUT_COMBO_BOX(config_ao)));

    if (audio_device.description != NULL) {
        g_free(audio_device.description);
        audio_device.description = NULL;
    }
    audio_device.description = g_strdup(audio_device_name);
    gm_audio_update_device(&audio_device);
    if (softvol || audio_device.type == AUDIO_TYPE_SOFTVOL) {
        gm_audio_set_server_volume_update_callback(&audio_device, NULL);
        gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SOFTVOL, TRUE);
    } else {
        gm_audio_get_volume(&audio_device);
        gm_audio_set_server_volume_update_callback(&audio_device, set_volume);
        gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SOFTVOL, FALSE);
    }

    gmtk_media_player_set_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AO, audio_device.mplayer_ao);

#ifdef HAVE_ASOUNDLIB
    if (audio_device.alsa_mixer != NULL) {
        g_free(audio_device.alsa_mixer);
        audio_device.alsa_mixer = NULL;
    }
    audio_device.alsa_mixer = g_strdup(gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(config_mixer)))));
#endif

    if (alang != NULL) {
        g_free(alang);
        alang = NULL;
    }
    alang = g_strdup(gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(config_alang)))));

    if (slang != NULL) {
        g_free(slang);
        slang = NULL;
    }
    slang = g_strdup(gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(config_slang)))));

    if (metadata_codepage != NULL) {
        g_free(metadata_codepage);
        metadata_codepage = NULL;
    }
    metadata_codepage = g_strdup(gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(config_metadata_codepage)))));

    if (subtitle_codepage != NULL) {
        g_free(subtitle_codepage);
        subtitle_codepage = NULL;
    }
    subtitle_codepage = g_strdup(gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(config_subtitle_codepage)))));

    if (mplayer_dvd_device != NULL) {
        g_free(mplayer_dvd_device);
        mplayer_dvd_device = NULL;
    }
    mplayer_dvd_device = g_strdup(gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(config_mplayer_dvd_device)))));

    audio_channels = gtk_combo_box_get_active(GTK_COMBO_BOX(config_audio_channels));
    use_hw_audio = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_use_hw_audio));
    use_hardware_codecs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_hardware_codecs));
    use_crystalhd_codecs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_crystalhd_codecs));
    cache_size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(config_cachesize));
    old_disable_framedrop = disable_framedrop;
    disable_deinterlace = !(gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_deinterlace));
    disable_framedrop = !(gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_framedrop));
    disable_ass = !(gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_ass));
    disable_embeddedfonts = !(gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_embeddedfonts));
    disable_pause_on_click = !(gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_pause_on_click));
    disable_animation = (gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_disable_animation));
    disable_cover_art_fetch =
        (gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_disable_cover_art_fetch));
    oldosd = osdlevel;
    osdlevel = (gint) gtk_range_get_value(GTK_RANGE(config_osdlevel));
    pplevel = (gint) gtk_range_get_value(GTK_RANGE(config_pplevel));
    softvol = (gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_softvol));
    remember_softvol = (gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_remember_softvol));
    volume_gain = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(config_volume_gain));
    verbose = (gint) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_verbose));
    mouse_wheel_changes_volume = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_mouse_wheel));
    resume_mode = gtk_combo_box_get_active(GTK_COMBO_BOX(config_resume_mode));
    playlist_visible = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_playlist_visible));
    details_visible = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_details_visible));
    use_mediakeys = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_use_mediakeys));
    use_defaultpl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_use_defaultpl));
    vertical_layout = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_vertical_layout));
    single_instance = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_single_instance));
    replace_and_play = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_replace_and_play));
    bring_to_front = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_bring_to_front));
    show_status_icon = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_show_status_icon));
    if (GTK_IS_STATUS_ICON(status_icon)) {
        gtk_status_icon_set_visible(status_icon, show_status_icon);
    } else {
        if (show_status_icon) {
            GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
            if (gtk_icon_theme_has_icon(icon_theme, "g-mplayer")) {
                status_icon = gtk_status_icon_new_from_icon_name("g-mplayer");
            } else {
                status_icon = gtk_status_icon_new_from_pixbuf(pb_icon);
            }
            gtk_status_icon_set_visible(status_icon, show_status_icon);
            g_signal_connect(status_icon, "activate", G_CALLBACK(status_icon_callback), NULL);
            g_signal_connect(status_icon, "popup_menu", G_CALLBACK(status_icon_context_callback), NULL);

        } else {
            if (GTK_IS_STATUS_ICON(status_icon)) {
                gtk_status_icon_set_visible(status_icon, show_status_icon);
            }
        }
    }

    forcecache = (gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_forcecache));
    remember_loc = (gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_remember_loc));
    disable_cover_art_fetch =
        (gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_disable_cover_art_fetch));
    resize_on_new_media = (gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_resize_on_new_media));
    keep_on_top = (gboolean) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_keep_on_top));
    gtk_window_set_keep_above(GTK_WINDOW(window), keep_on_top);

    if (subtitlefont != NULL) {
        g_free(subtitlefont);
        subtitlefont = NULL;
    }
    subtitlefont = g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(config_subtitle_font)));
    subtitle_scale = gtk_spin_button_get_value(GTK_SPIN_BUTTON(config_subtitle_scale));
    gtk_color_button_get_color(GTK_COLOR_BUTTON(config_subtitle_color), &sub_color);
    if (subtitle_color != NULL) {
        g_free(subtitle_color);
        subtitle_color = NULL;
    }
    subtitle_color = g_strdup_printf("%02x%02x%02x00", sub_color.red >> 8, sub_color.green >> 8, sub_color.blue >> 8);
    subtitle_outline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_subtitle_outline));
    subtitle_shadow = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_subtitle_shadow));
    subtitle_margin = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(config_subtitle_margin));
    subtitle_fuzziness = (gint) gtk_range_get_value(GTK_RANGE(config_subtitle_fuzziness));
    showsubtitles = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_show_subtitles));

    if (old_disable_framedrop != disable_framedrop) {
        gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ENABLE_FRAME_DROP,
                                                !disable_framedrop);
    }

    if (oldosd != osdlevel) {
        gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_OSDLEVEL, osdlevel);
    }

    mplayer_bin = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(config_mplayer_bin)));
    if (!g_file_test(mplayer_bin, G_FILE_TEST_EXISTS)) {
        g_free(mplayer_bin);
        mplayer_bin = NULL;
    }
    extraopts = g_strdup(gtk_entry_get_text(GTK_ENTRY(config_extraopts)));

    set_media_player_attributes(media);

    gm_store = gm_pref_store_new("g-mplayer");

    gm_pref_store_set_string(gm_store, AUDIO_DEVICE_NAME, audio_device_name);

#ifndef HAVE_ASOUNDLIB
    gm_pref_store_set_int(gm_store, VOLUME, gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(config_volume)));
#endif
    gm_pref_store_set_string(gm_store, VO, vo);
    gm_pref_store_set_int(gm_store, AUDIO_CHANNELS, audio_channels);
    gm_pref_store_set_boolean(gm_store, USE_HW_AUDIO, use_hw_audio);
    gm_pref_store_set_boolean(gm_store, USE_HARDWARE_CODECS, use_hardware_codecs);
    gm_pref_store_set_boolean(gm_store, USE_CRYSTALHD_CODECS, use_crystalhd_codecs);
    gm_pref_store_set_int(gm_store, CACHE_SIZE, cache_size);
    gm_pref_store_set_string(gm_store, ALSA_MIXER, audio_device.alsa_mixer);
    gm_pref_store_set_int(gm_store, OSDLEVEL, osdlevel);
    gm_pref_store_set_int(gm_store, PPLEVEL, pplevel);
    gm_pref_store_set_boolean(gm_store, SOFTVOL, softvol);
    gm_pref_store_set_boolean(gm_store, REMEMBER_SOFTVOL, remember_softvol);
    gm_pref_store_set_int(gm_store, VOLUME_GAIN, volume_gain);
    gm_pref_store_set_boolean(gm_store, FORCECACHE, forcecache);
    gm_pref_store_set_boolean(gm_store, DISABLEASS, disable_ass);
    gm_pref_store_set_boolean(gm_store, DISABLEEMBEDDEDFONTS, disable_embeddedfonts);
    gm_pref_store_set_boolean(gm_store, DISABLEDEINTERLACE, disable_deinterlace);
    gm_pref_store_set_boolean(gm_store, DISABLEFRAMEDROP, disable_framedrop);
    gm_pref_store_set_boolean(gm_store, DISABLEPAUSEONCLICK, disable_pause_on_click);
    gm_pref_store_set_boolean(gm_store, DISABLEANIMATION, disable_animation);
    gm_pref_store_set_boolean(gm_store, SHOWPLAYLIST, playlist_visible);
    gm_pref_store_set_boolean(gm_store, SHOWDETAILS, details_visible);
    gm_pref_store_set_boolean(gm_store, USE_MEDIAKEYS, use_mediakeys);
    gm_pref_store_set_boolean(gm_store, USE_DEFAULTPL, use_defaultpl);
    gm_pref_store_set_boolean(gm_store, MOUSE_WHEEL_CHANGES_VOLUME, mouse_wheel_changes_volume);
    gm_pref_store_set_boolean(gm_store, SHOW_NOTIFICATION, show_notification);
    gm_pref_store_set_boolean(gm_store, SHOW_STATUS_ICON, show_status_icon);
    gm_pref_store_set_boolean(gm_store, VERTICAL, vertical_layout);
    gm_pref_store_set_boolean(gm_store, SINGLE_INSTANCE, single_instance);
    gm_pref_store_set_boolean(gm_store, REPLACE_AND_PLAY, replace_and_play);
    gm_pref_store_set_boolean(gm_store, BRING_TO_FRONT, bring_to_front);
    gm_pref_store_set_boolean(gm_store, REMEMBER_LOC, remember_loc);
    gm_pref_store_set_boolean(gm_store, DISABLE_COVER_ART_FETCH, disable_cover_art_fetch);
    gm_pref_store_set_boolean(gm_store, KEEP_ON_TOP, keep_on_top);
    gm_pref_store_set_boolean(gm_store, RESIZE_ON_NEW_MEDIA, resize_on_new_media);
    gm_pref_store_set_int(gm_store, RESUME_MODE, resume_mode);
    gm_pref_store_set_int(gm_store, VERBOSE, verbose);
    gm_pref_store_set_string(gm_store, METADATACODEPAGE, metadata_codepage);
    gm_pref_store_set_string(gm_store, SUBTITLEFONT, subtitlefont);
    gm_pref_store_set_float(gm_store, SUBTITLESCALE, subtitle_scale);
    gm_pref_store_set_string(gm_store, SUBTITLECODEPAGE, subtitle_codepage);
    gm_pref_store_set_string(gm_store, SUBTITLECOLOR, subtitle_color);
    gm_pref_store_set_boolean(gm_store, SUBTITLEOUTLINE, subtitle_outline);
    gm_pref_store_set_boolean(gm_store, SUBTITLESHADOW, subtitle_shadow);
    gm_pref_store_set_int(gm_store, SUBTITLE_MARGIN, subtitle_margin);
    gm_pref_store_set_boolean(gm_store, SHOW_SUBTITLES, showsubtitles);

    gm_pref_store_set_string(gm_store, AUDIO_LANG, alang);
    gm_pref_store_set_string(gm_store, SUBTITLE_LANG, slang);

    gm_pref_store_set_string(gm_store, MPLAYER_BIN, mplayer_bin);
    gm_pref_store_set_string(gm_store, MPLAYER_DVD_DEVICE, mplayer_dvd_device);
    gm_pref_store_set_string(gm_store, EXTRAOPTS, extraopts);
    gm_pref_store_set_string(gm_store, ACCELERATOR_KEYS, g_strjoinv(" ", accel_keys));
    gm_pref_store_free(gm_store);

    gmtk_media_player_restart(GMTK_MEDIA_PLAYER(media));
    gtk_widget_destroy(widget);
}

void vpa_reset_values(GtkWidget * widget, void *data)
{
    gtk_range_set_value (GTK_RANGE (vpa_scale_brightness), 0);
    gtk_range_set_value (GTK_RANGE (vpa_scale_contrast),   0);
    gtk_range_set_value (GTK_RANGE (vpa_scale_hue),        0);
    gtk_range_set_value (GTK_RANGE (vpa_scale_gamma),      0);
    gtk_range_set_value (GTK_RANGE (vpa_scale_saturation), 0);
}


void config_close(GtkWidget * widget, void *data)
{
    selection = NULL;
    if (GTK_IS_WIDGET(widget))
        gtk_widget_destroy(widget);
}

void on_gtk_scale_ValueChanged_brightness_cb (GtkRange * range, gpointer data)
{
    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_BRIGHTNESS,
                                            (gint) gtk_range_get_value(range));
}

void on_gtk_scale_ValueChanged_contrast_cb (GtkRange * range, gpointer data)
{
    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_CONTRAST,
                                            (gint) gtk_range_get_value(range));
}

void on_gtk_scale_ValueChanged_gamma_cb (GtkRange * range, gpointer data)
{
    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_GAMMA,
                                            (gint) gtk_range_get_value(range));
}

void on_gtk_scale_ValueChanged_hue_cb (GtkRange * range, gpointer data)
{
    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HUE, (gint) gtk_range_get_value(range));
}

void on_gtk_scale_ValueChanged_saturation_cb (GtkRange * range, gpointer data)
{
    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SATURATION,
                                            (gint) gtk_range_get_value(range));
}

void menuitem_details_callback(GtkMenuItem * menuitem, void *data)
{
    g_idle_add(set_adjust_layout, NULL);

}

void create_details_table()
{
    GtkWidget *label;
    gchar *buf;
    gint i = 0;
    IdleData *idle = idledata;

    label = gtk_label_new(_("<span weight=\"bold\">Video Details</span>"));
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 2, i, i + 1);
    i++;

    if (idle != NULL) {
        label = gtk_label_new(_("Video Size:"));
        gtk_widget_set_halign (label, GTK_ALIGN_END);
        gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
        buf = g_strdup_printf("%i x %i", idle->width, idle->height);
        details_video_size = gtk_label_new(buf);
        gtk_widget_set_halign (details_video_size, GTK_ALIGN_START);
        gtk_table_attach_defaults(GTK_TABLE(details_table), details_video_size, 1, 2, i, i + 1);
        g_free(buf);
        i++;
    }

    label = gtk_label_new(_("Video Format:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    buf = g_strdup_printf(_("Unknown"));
    details_video_format = gtk_label_new(buf);
    g_free(buf);
    gtk_widget_set_halign (details_video_format, GTK_ALIGN_START);
    gtk_table_attach_defaults(GTK_TABLE(details_table), details_video_format, 1, 2, i, i + 1);
    i++;

    label = gtk_label_new(_("Video Codec:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    buf = g_strdup_printf(_("Unknown"));
    details_video_codec = gtk_label_new(buf);
    g_free(buf);
    gtk_widget_set_halign (details_video_codec, GTK_ALIGN_START);
    gtk_table_attach_defaults(GTK_TABLE(details_table), details_video_codec, 1, 2, i, i + 1);
    i++;

    label = gtk_label_new(_("Video FPS:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    details_video_fps = gtk_label_new("");
    gtk_widget_set_halign (details_video_fps, GTK_ALIGN_START);
    gtk_table_attach_defaults(GTK_TABLE(details_table), details_video_fps, 1, 2, i, i + 1);
    i++;

    label = gtk_label_new(_("Video Bitrate:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    buf = g_strdup_printf("0 Kb/s");
    details_video_bitrate = gtk_label_new(buf);
    g_free(buf);
    gtk_widget_set_halign (details_video_bitrate, GTK_ALIGN_START);
    gtk_table_attach_defaults(GTK_TABLE(details_table), details_video_bitrate, 1, 2, i, i + 1);
    i++;

    label = gtk_label_new(_("Video Chapters:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_widget_set_margin_start (label, 12);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    buf = g_strdup_printf(_("Unknown"));
    details_video_chapters = gtk_label_new(buf);
    g_free(buf);
    gtk_widget_set_halign (details_video_chapters, GTK_ALIGN_START);
    gtk_table_attach_defaults(GTK_TABLE(details_table), details_video_chapters, 1, 2, i, i + 1);
    i++;

    label = gtk_label_new("");
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    i++;

    label = gtk_label_new(_("<span weight=\"bold\">Audio Details</span>"));
    gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 2, i, i + 1);
    i++;

    label = gtk_label_new(_("Audio Format:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    buf = g_strdup_printf(_("Unknown"));
    details_audio_format = gtk_label_new(buf);
    g_free(buf);
    gtk_widget_set_halign (details_audio_format, GTK_ALIGN_START);
    gtk_table_attach_defaults(GTK_TABLE(details_table), details_audio_format, 1, 2, i, i + 1);
    i++;

    label = gtk_label_new(_("Audio Codec:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    buf = g_strdup_printf(_("Unknown"));
    details_audio_codec = gtk_label_new(buf);
    g_free(buf);
    gtk_widget_set_halign (details_audio_codec, GTK_ALIGN_START);
    gtk_table_attach_defaults(GTK_TABLE(details_table), details_audio_codec, 1, 2, i, i + 1);
    i++;

    label = gtk_label_new(_("Audio Channels:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    if (idle != NULL) {
        buf = g_strdup_printf(_("Unknown"));
        details_audio_channels = gtk_label_new(buf);
        g_free(buf);
        gtk_widget_set_halign (details_audio_channels, GTK_ALIGN_START);
        gtk_table_attach_defaults(GTK_TABLE(details_table), details_audio_channels, 1, 2, i, i + 1);
    }
    i++;

    label = gtk_label_new(_("Audio Bitrate:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    if (idle != NULL) {
        buf = g_strdup_printf("0 Kb/s");
        details_audio_bitrate = gtk_label_new(buf);
        g_free(buf);
        gtk_widget_set_halign (details_audio_bitrate, GTK_ALIGN_START);
        gtk_table_attach_defaults(GTK_TABLE(details_table), details_audio_bitrate, 1, 2, i, i + 1);
    }
    i++;

    label = gtk_label_new(_("Audio Sample Rate:"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_table_attach_defaults(GTK_TABLE(details_table), label, 0, 1, i, i + 1);
    if (idle != NULL) {
        buf = g_strdup_printf("0 KHz");
        details_audio_samplerate = gtk_label_new(buf);
        g_free(buf);
        gtk_widget_set_halign (details_audio_samplerate, GTK_ALIGN_START);
        gtk_table_attach_defaults(GTK_TABLE(details_table), details_audio_samplerate, 1, 2, i, i + 1);
    }
    i++;

}


void menuitem_video_adjustments_dlg (GtkMenuItem * menuitem, void *data)
{
    GtkWidget * dlg_window;
    GtkWidget * main_vbox;
    GtkWidget * frame1, * vbox_table, * hbox_row[5], * label_row[5], * scale_row[5];
    GtkWidget * hbutton_box;
    GtkWidget * button_reset, * button_close;
    GtkWidget * label;

    dlg_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint (GTK_WINDOW (dlg_window), GDK_WINDOW_TYPE_HINT_UTILITY);
    gtk_window_set_resizable (GTK_WINDOW (dlg_window), FALSE);
    gtk_window_set_icon (GTK_WINDOW (dlg_window), pb_icon);
    gtk_window_set_transient_for (GTK_WINDOW (dlg_window), GTK_WINDOW(window));
    gtk_window_set_position (GTK_WINDOW (dlg_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_keep_above (GTK_WINDOW (dlg_window), keep_on_top);

    gtk_window_set_title(GTK_WINDOW(dlg_window), _("Video Picture Adjustments"));
    gtk_container_set_border_width (GTK_CONTAINER (dlg_window), 4);

    // vbox
    main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add (GTK_CONTAINER (dlg_window), main_vbox);
    gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 4);

    // vbox -> frame1
    frame1 = gtk_frame_new (_("<span weight=\"bold\">Video Picture Adjustments</span>"));
    label = gtk_frame_get_label_widget (GTK_FRAME (frame1));
    gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
    gtk_box_pack_start (GTK_BOX (main_vbox), frame1, TRUE, TRUE, 0);

    // vbox -> frame1 -> vbox
    vbox_table = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (frame1), vbox_table);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_table), 5); // padding
    gtk_box_set_spacing (GTK_BOX (vbox_table), 4);

    int i;
    for (i = 0 ; i < 5 ; i++)
    {
       hbox_row[i]  = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
       gtk_box_pack_start (GTK_BOX (vbox_table), hbox_row[i], FALSE, FALSE, 0);

       label_row[i] = gtk_label_new ("empty label");
       scale_row[i] = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -100.0, 100.0, 1.0);
       gtk_widget_set_size_request (scale_row[i], 200, -1);
       gtk_scale_set_value_pos (GTK_SCALE (scale_row[i]), GTK_POS_LEFT);
       gtk_range_set_value (GTK_RANGE (scale_row[i]), 0.0);

       gtk_box_pack_end (GTK_BOX (hbox_row[i]), scale_row[i], FALSE, FALSE, 0);
       gtk_box_pack_end (GTK_BOX (hbox_row[i]), label_row[i], FALSE, FALSE, 0);
    }

    gtk_label_set_text (GTK_LABEL (label_row[0]), _("Brightness"));
    vpa_scale_brightness = scale_row[0];
    gtk_range_set_value(GTK_RANGE(vpa_scale_brightness),
                        gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_BRIGHTNESS));
    gtk_label_set_text (GTK_LABEL (label_row[1]), _("Contrast"));
    vpa_scale_contrast = scale_row[1];
    gtk_range_set_value(GTK_RANGE(vpa_scale_contrast),
                        gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_CONTRAST));
    gtk_label_set_text (GTK_LABEL (label_row[2]), _("Gamma"));
    vpa_scale_gamma = scale_row[2];
    gtk_range_set_value(GTK_RANGE(vpa_scale_gamma),
                        gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_GAMMA));
    gtk_label_set_text (GTK_LABEL (label_row[3]), _("Hue"));
    vpa_scale_hue = scale_row[3];
    gtk_range_set_value(GTK_RANGE(vpa_scale_hue),
                        gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HUE));
    gtk_label_set_text (GTK_LABEL (label_row[4]), _("Saturation"));
    vpa_scale_saturation = scale_row[4];
    gtk_range_set_value(GTK_RANGE(vpa_scale_saturation),
                        gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SATURATION));

    g_signal_connect (G_OBJECT (vpa_scale_brightness), "value_changed",
                      G_CALLBACK (on_gtk_scale_ValueChanged_brightness_cb), idledata);
    g_signal_connect (G_OBJECT (vpa_scale_contrast), "value_changed",
                      G_CALLBACK (on_gtk_scale_ValueChanged_contrast_cb), idledata);
    g_signal_connect (G_OBJECT (vpa_scale_gamma), "value_changed",
                      G_CALLBACK (on_gtk_scale_ValueChanged_gamma_cb), idledata);
    g_signal_connect (G_OBJECT (vpa_scale_hue), "value_changed",
                      G_CALLBACK (on_gtk_scale_ValueChanged_hue_cb), idledata);
    g_signal_connect (G_OBJECT (vpa_scale_saturation), "value_changed",
                      G_CALLBACK (on_gtk_scale_ValueChanged_saturation_cb), idledata);

    // vbox -> hbox
    hbutton_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbutton_box), GTK_BUTTONBOX_END);
    gtk_box_set_spacing (GTK_BOX (hbutton_box), 10);
    gtk_container_add (GTK_CONTAINER (main_vbox), hbutton_box);

    button_reset = gtk_button_new_with_mnemonic(_("_Reset"));
    g_signal_connect (G_OBJECT (button_reset), "clicked",
                      G_CALLBACK (vpa_reset_values), NULL);
    gtk_container_add (GTK_CONTAINER (hbutton_box), button_reset);

    GtkWidget * img = gtk_image_new_from_icon_name ("gtk-close", GTK_ICON_SIZE_BUTTON);
    button_close = gtk_button_new_with_mnemonic (_("_Close"));
    gtk_button_set_image (GTK_BUTTON (button_close), img);
    g_signal_connect_swapped (G_OBJECT (button_close), "clicked",
                              G_CALLBACK (gtk_widget_destroy), dlg_window);
    gtk_container_add (GTK_CONTAINER (hbutton_box), button_close);

    gtk_widget_show_all (dlg_window);
    gtk_window_present (GTK_WINDOW(dlg_window));
}

void menuitem_view_angle_callback(GtkMenuItem * menuitem, gpointer data)
{
    gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SWITCH_ANGLE);
    return;
}

void menuitem_view_smaller_subtitle_callback(GtkMenuItem * menuitem, void *data)
{

    subtitle_scale -= 0.2;
    gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_SCALE, subtitle_scale);

    return;
}

void menuitem_view_larger_subtitle_callback(GtkMenuItem * menuitem, void *data)
{
    subtitle_scale += 0.2;
    gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_SCALE, subtitle_scale);

    return;
}

void menuitem_view_decrease_subtitle_delay_callback(GtkMenuItem * menuitem, void *data)
{
    gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_DELAY,
                                           gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media),
                                                                                  ATTRIBUTE_SUBTITLE_DELAY) - 0.1);
    return;

}

void menuitem_view_increase_subtitle_delay_callback(GtkMenuItem * menuitem, void *data)
{
    gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_DELAY,
                                           gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media),
                                                                                  ATTRIBUTE_SUBTITLE_DELAY) + 0.1);
    return;

}

void menuitem_view_aspect_callback(GtkMenuItem * menuitem, void *data)
{
    static gint i = 0;
    GmtkMediaPlayerAspectRatio aspect;

    if ((gpointer) menuitem == (gpointer) menuitem_view_aspect_default) {
        i++;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_four_three), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_nine), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_ten), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_anamorphic), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_follow_window), FALSE);
        aspect = ASPECT_DEFAULT;
        i--;
    }

    if ((gpointer) menuitem == (gpointer) menuitem_view_aspect_four_three) {
        i++;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_default), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_nine), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_ten), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_anamorphic), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_follow_window), FALSE);
        aspect = ASPECT_4X3;
        i--;
    }

    if ((gpointer) menuitem == (gpointer) menuitem_view_aspect_sixteen_nine) {
        i++;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_default), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_four_three), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_ten), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_anamorphic), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_follow_window), FALSE);
        aspect = ASPECT_16X9;
        i--;
    }

    if ((gpointer) menuitem == (gpointer) menuitem_view_aspect_sixteen_ten) {
        i++;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_default), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_four_three), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_nine), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_anamorphic), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_follow_window), FALSE);
        aspect = ASPECT_16X10;
        i--;
    }

    if ((gpointer) menuitem == (gpointer) menuitem_view_aspect_anamorphic) {
        i++;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_default), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_four_three), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_nine), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_ten), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_follow_window), FALSE);
        aspect = ASPECT_ANAMORPHIC;
        i--;
    }

    if ((gpointer) menuitem == (gpointer) menuitem_view_aspect_follow_window) {
        i++;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_default), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_four_three), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_nine), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_sixteen_ten), FALSE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_anamorphic), FALSE);
        aspect = ASPECT_WINDOW;
        i--;
    }

    if (i == 0) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), TRUE);
        gmtk_media_player_set_aspect(GMTK_MEDIA_PLAYER(media), aspect);
    }
}

gchar *osdlevel_format_callback(GtkScale * scale, gdouble value)
{
    gchar *text;

    switch ((gint) value) {

    case 0:
        text = g_strdup(_("No Display"));
        break;
    case 1:
        text = g_strdup(_("Minimal"));
        break;
    case 2:
        text = g_strdup(_("Timer"));
        break;
    case 3:
        text = g_strdup(_("Timer/Total"));
        break;
    default:
        text = g_strdup("How did we get here?");
    }

    return text;
}

gchar *pplevel_format_callback(GtkScale * scale, gdouble value)
{
    gchar *text;

    switch ((gint) value) {

    case 0:
        text = g_strdup(_("No Postprocessing"));
        break;
    case 1:
    case 2:
        text = g_strdup(_("Minimal Postprocessing"));
        break;
    case 3:
    case 4:
        text = g_strdup(_("More Postprocessing"));
        break;
    case 5:
    case 6:
        text = g_strdup(_("Maximum Postprocessing"));
        break;
    default:
        text = g_strdup("How did we get here?");
    }

    return text;
}

gchar *subtitle_fuzziness_format_callback(GtkScale * scale, gdouble value)
{
    gchar *text;

    switch ((gint) value) {

    case 0:
        text = g_strdup(_("Exact Match"));
        break;
    case 1:
        text = g_strdup(_("Load all subtitles containing movie name"));
        break;
    case 2:
        text = g_strdup(_("Load all subtitles in the same folder"));
        break;
    default:
        text = g_strdup("How did we get here?");
    }

    return text;
}

void osdlevel_change_callback(GtkRange * range, gpointer data)
{
    gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_OSDLEVEL,
                                            (gint) gtk_range_get_value(range));

    return;
}

void config_single_instance_callback(GtkWidget * button, gpointer data)
{
    gtk_widget_set_sensitive(config_replace_and_play,
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_single_instance)));
    gtk_widget_set_sensitive(config_bring_to_front,
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_single_instance)));
}

void config_softvol_callback(GtkWidget * button, gpointer data)
{
    gtk_widget_set_sensitive(config_remember_softvol, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_softvol)));
    gtk_widget_set_sensitive(config_volume_gain, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_softvol)));

}

void config_forcecache_callback(GtkWidget * button, gpointer data)
{
    gtk_widget_set_sensitive(config_cachesize, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_forcecache)));
}

void ass_toggle_callback(GtkToggleButton * source, gpointer user_data)
{
    gtk_widget_set_sensitive(config_subtitle_color, gtk_toggle_button_get_active(source));
    gtk_widget_set_sensitive(config_embeddedfonts, gtk_toggle_button_get_active(source));
}

void hw_audio_toggle_callback(GtkToggleButton * source, gpointer user_data)
{
    if (gtk_toggle_button_get_active(source)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_softvol), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(config_softvol), FALSE);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(config_softvol), TRUE);
    }
}

void output_combobox_changed_callback(GtkComboBox * config_ao, gpointer data)
{
    GtkComboBox *config_mixer = GTK_COMBO_BOX(data);
    gchar *device;
    gint card;

#ifdef HAVE_ASOUNDLIB
    snd_mixer_t *mhandle;
    snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;
    gint err;
    gchar *mix;
    gint i, j, master, pcm;
#endif

    if (gmtk_output_combo_box_get_active_type(GMTK_OUTPUT_COMBO_BOX(config_ao)) == OUTPUT_TYPE_ALSA) {
        gtk_widget_set_sensitive(GTK_WIDGET(config_mixer), TRUE);
        card = gmtk_output_combo_box_get_active_card(GMTK_OUTPUT_COMBO_BOX(config_ao));
        if (card == -1) {
            device = g_strdup_printf("default");
        } else {
            device = g_strdup_printf("hw:%i", card);
        }

        gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(config_mixer)));
        gtk_combo_box_set_active(config_mixer, -1);

#ifdef HAVE_ASOUNDLIB
        if (config_mixer != NULL) {
            if ((err = snd_mixer_open(&mhandle, 0)) < 0) {
                gm_log(verbose, G_LOG_LEVEL_INFO, "Mixer open error %s", snd_strerror(err));
            }

            if ((err = snd_mixer_attach(mhandle, device)) < 0) {
                gm_log(verbose, G_LOG_LEVEL_INFO, "Mixer attach error %s", snd_strerror(err));
            }

            if ((err = snd_mixer_selem_register(mhandle, NULL, NULL)) < 0) {
                gm_log(verbose, G_LOG_LEVEL_INFO, "Mixer register error %s", snd_strerror(err));
            }

            if ((err = snd_mixer_load(mhandle)) < 0) {
                gm_log(verbose, G_LOG_LEVEL_INFO, "Mixer load error %s", snd_strerror(err));
            }
            i = 0;
            j = -1;
            master = -1;
            pcm = -1;
            snd_mixer_selem_id_alloca(&sid);
            for (elem = snd_mixer_first_elem(mhandle); elem; elem = snd_mixer_elem_next(elem)) {
                snd_mixer_selem_get_id(elem, sid);
                if (!snd_mixer_selem_is_active(elem))
                    continue;
                if (snd_mixer_selem_has_capture_volume(elem)
                    || snd_mixer_selem_has_capture_switch(elem))
                    continue;
                if (!snd_mixer_selem_has_playback_volume(elem))
                    continue;
                mix = g_strdup_printf("%s,%i", snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_mixer), mix);
                if (audio_device.alsa_mixer != NULL && g_ascii_strcasecmp(mix, audio_device.alsa_mixer) == 0)
                    j = i;
                if (g_ascii_strcasecmp(snd_mixer_selem_id_get_name(sid), "Master") == 0)
                    master = i;
                if (g_ascii_strcasecmp(snd_mixer_selem_id_get_name(sid), "PCM") == 0)
                    pcm = i;

                i++;
            }
            if (j != -1)
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_mixer), j);
            if (j == -1 && pcm != -1)
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_mixer), pcm);
            if (j == -1 && master != -1)
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_mixer), master);

            snd_mixer_close(mhandle);

        }
#endif
        g_free(device);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(config_mixer), FALSE);
    }

    if (gmtk_output_combo_box_get_active_type(GMTK_OUTPUT_COMBO_BOX(config_ao)) == OUTPUT_TYPE_ALSA && softvol == FALSE) {
        gtk_label_set_text(GTK_LABEL(conf_volume_label), _("Direct ALSA Control"));
    } else if (gmtk_output_combo_box_get_active_type(GMTK_OUTPUT_COMBO_BOX(config_ao)) == OUTPUT_TYPE_PULSE
               && softvol == FALSE) {
        gtk_label_set_text(GTK_LABEL(conf_volume_label), _("Direct PulseAudio Control"));
    } else {
        gtk_label_set_text(GTK_LABEL(conf_volume_label), _("Software Volume Control"));
    }

}

void menuitem_config_dialog_cb(GtkMenuItem * menuitem, void *data)
{
    GtkWidget *config_window;
    GtkWidget *conf_vbox;
    GtkWidget *conf_hbutton_box;
    GtkWidget *conf_ok;
    GtkWidget *conf_cancel;
    GtkWidget *conf_table;
    GtkWidget *conf_label, * entry;
    GtkWidget *conf_page_vbox[5], * page_label[5], * page_table[6];
    GtkWidget *notebook;
    GdkColor sub_color;
    gint i = 0;
    gint j = -1;
    gint k = -1;
    GtkTreeIter ao_iter;
    gchar *desc;
    guint key;
    GdkModifierType modifier;

    config_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(config_window), GDK_WINDOW_TYPE_HINT_UTILITY);
    gtk_window_set_resizable(GTK_WINDOW(config_window), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(config_window), GTK_WINDOW(window));
    gtk_window_set_position(GTK_WINDOW(config_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_icon(GTK_WINDOW(config_window), pb_icon);
    gtk_window_set_resizable(GTK_WINDOW(config_window), FALSE);

    conf_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    conf_hbutton_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(conf_hbutton_box), GTK_BUTTONBOX_END);

    notebook = gtk_notebook_new();
    for (i = 0; i < 5 ; i++)
    {
       /* create and append page to notebook */
       conf_page_vbox[i] = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
       page_label[i]     = gtk_label_new ("lbl");
       gtk_notebook_append_page (GTK_NOTEBOOK (notebook), conf_page_vbox[i], page_label[i]);
       gtk_container_set_border_width (GTK_CONTAINER (conf_page_vbox[i]), 6);
       /* page table: 20 rows, 2 columns - add to conf_page_vbox[i] */
       page_table[i] = gtk_table_new(20, 2, FALSE);
       gtk_table_set_col_spacings (GTK_TABLE (page_table[i]), 5);
       gtk_container_add (GTK_CONTAINER (conf_page_vbox[i]), page_table[i]);
    }
    gtk_label_set_text (GTK_LABEL (page_label[0]), _("Player"));
    gtk_label_set_text (GTK_LABEL (page_label[1]), _("Subtitles"));
    gtk_label_set_text (GTK_LABEL (page_label[2]), _("Interface"));
    gtk_label_set_text (GTK_LABEL (page_label[3]), _("Keyboard Shortcuts"));
    gtk_label_set_text (GTK_LABEL (page_label[4]), _("MPlayer"));

    gtk_container_add(GTK_CONTAINER(conf_vbox), notebook);
    gtk_container_add(GTK_CONTAINER(config_window), conf_vbox);

    gtk_window_set_title(GTK_WINDOW(config_window), _("G-MPlayer Configuration"));
    gtk_container_set_border_width(GTK_CONTAINER(config_window), 5);
    //gtk_window_set_default_size(GTK_WINDOW(config_window), 300, 300);
    conf_ok = gtk_button_new_from_stock("gtk-ok");
    g_signal_connect_swapped(G_OBJECT(conf_ok), "clicked", G_CALLBACK(config_apply), config_window);

    conf_cancel = gtk_button_new_from_stock("gtk-close");
    g_signal_connect_swapped(G_OBJECT(conf_cancel), "clicked", G_CALLBACK(config_apply), config_window);

    config_vo = gtk_combo_box_text_new_with_entry();

    gtk_widget_set_tooltip_text(config_vo,
                                _("mplayer video output device\nx11 should always work, try xv, gl or vdpau for better performance and enhanced features"));

    if (config_vo != NULL) {
        static const char * vo_str[] = { "gl", "gl2", "gl3", "x11", "xv", "xvmc", "vaapi", "vdpau", NULL, };
        gboolean add_vo_to_combo = TRUE;
        for (i = 0 ; vo_str[i] ; i++) {
           gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (config_vo), vo_str[i]);
           if (vo && strcmp (vo, vo_str[i]) == 0)
               add_vo_to_combo = FALSE;
        }
        if (vo) {
           entry = gtk_bin_get_child (GTK_BIN (config_vo));
           gtk_entry_set_text (GTK_ENTRY (entry), vo);
           if (add_vo_to_combo == TRUE) {
              gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (config_vo), vo);
           }
        }
    }

    config_use_hw_audio = gtk_check_button_new_with_mnemonic(_("Enable AC3/DTS pass-through to S/PDIF"));
    g_signal_connect(GTK_WIDGET(config_use_hw_audio), "toggled", G_CALLBACK(hw_audio_toggle_callback), NULL);

    config_mixer = gtk_combo_box_text_new_with_entry();

    config_softvol = gtk_check_button_new_with_label(_("Mplayer Software Volume Control Enabled"));
    conf_volume_label = gtk_label_new("");

    config_ao = gmtk_output_combo_box_new();
    g_signal_connect(GTK_WIDGET(config_ao), "changed", G_CALLBACK(output_combobox_changed_callback), config_mixer);

    if (gtk_tree_model_get_iter_first(gmtk_output_combo_box_get_tree_model(GMTK_OUTPUT_COMBO_BOX(config_ao)), &ao_iter)) {
        do {
            if (gtk_list_store_iter_is_valid
                (GTK_LIST_STORE(gmtk_output_combo_box_get_tree_model(GMTK_OUTPUT_COMBO_BOX(config_ao))), &ao_iter)) {
                gtk_tree_model_get(gmtk_output_combo_box_get_tree_model
                                   (GMTK_OUTPUT_COMBO_BOX(config_ao)), &ao_iter, OUTPUT_DESCRIPTION_COLUMN, &desc, -1);

                gm_log(verbose, G_LOG_LEVEL_DEBUG, "audio_device_name = %s, desc = %s", audio_device_name, desc);
                if (audio_device_name != NULL && strcmp(audio_device_name, desc) == 0) {
                    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(config_ao), &ao_iter);
                    g_free(desc);
                    break;
                }

                if ((audio_device_name == NULL || strcmp(audio_device_name, "") == 0)
                    && strcmp(desc, _("Default")) == 0) {
                    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(config_ao), &ao_iter);
                    g_free(desc);
                    break;
                }
                g_free(desc);

            }
        } while (gtk_tree_model_iter_next
                 (gmtk_output_combo_box_get_tree_model(GMTK_OUTPUT_COMBO_BOX(config_ao)), &ao_iter));
    }
    config_alang = gtk_combo_box_text_new_with_entry();
    if (config_alang != NULL) {
        i = 0;
        j = -1;
        k = -1;
        while (langlist[i] != NULL) {
            if (alang != NULL && strlen(alang) > 0 && g_ascii_strncasecmp(alang, langlist[i], strlen(alang)) == 0)
                j = i;
            if (alang != NULL && strlen(alang) == 0
                && g_ascii_strncasecmp("English,eng,en", langlist[i], strlen("English,eng,en")) == 0)
                k = i;
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_alang), langlist[i++]);
            if (j != -1) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_alang), j);
            }
            if (k != -1) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_alang), k);
            }
        }
        if (alang != NULL && strlen(alang) > 0 && j == -1) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_alang), alang);
            gtk_combo_box_set_active(GTK_COMBO_BOX(config_alang), i);
        }
        gtk_widget_set_tooltip_text(config_alang,
                                    _("Choose one of the languages or type in your own comma-separated selection"));
    }
    config_slang = gtk_combo_box_text_new_with_entry();
    if (config_slang != NULL) {
        i = 0;
        j = -1;
        k = -1;
        while (langlist[i] != NULL) {
            if (slang != NULL && strlen(slang) > 0 && g_ascii_strncasecmp(slang, langlist[i], strlen(slang)) == 0)
                j = i;
            if (slang != NULL && strlen(slang) == 0
                && g_ascii_strncasecmp("English,eng,en", langlist[i], strlen("English,eng,en")) == 0)
                k = i;
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_slang), langlist[i++]);
            if (j != -1) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_slang), j);
            }
            if (k != -1) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_slang), k);
            }
        }
        if (slang != NULL && strlen(slang) > 0 && j == -1) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_slang), slang);
            gtk_combo_box_set_active(GTK_COMBO_BOX(config_slang), i);
        }
        gtk_widget_set_tooltip_text(config_slang,
                                    _("Choose one of the languages or type in your own comma-separated selection"));
    }
    config_metadata_codepage = gtk_combo_box_text_new_with_entry();
    if (config_metadata_codepage != NULL) {
        i = 0;
        j = -1;
        while (codepagelist[i] != NULL) {
            if (metadata_codepage != NULL && strlen(metadata_codepage) > 1
                && g_ascii_strncasecmp(metadata_codepage, codepagelist[i], strlen(metadata_codepage)) == 0)
                j = i;
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_metadata_codepage), codepagelist[i++]);
            if (j != -1)
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_metadata_codepage), j);
        }
        if (metadata_codepage != NULL && j == -1) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_metadata_codepage), metadata_codepage);
            gtk_combo_box_set_active(GTK_COMBO_BOX(config_metadata_codepage), i);
        }
    }
    config_subtitle_codepage = gtk_combo_box_text_new_with_entry();
    if (config_subtitle_codepage != NULL) {
        i = 0;
        j = -1;
        while (codepagelist[i] != NULL) {
            if (subtitle_codepage != NULL && strlen(subtitle_codepage) > 1
                && g_ascii_strncasecmp(subtitle_codepage, codepagelist[i], strlen(subtitle_codepage)) == 0)
                j = i;
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_subtitle_codepage), codepagelist[i++]);

            if (j != -1)
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_subtitle_codepage), j);
        }
        if (subtitle_codepage != NULL && j == -1) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_subtitle_codepage), subtitle_codepage);
            gtk_combo_box_set_active(GTK_COMBO_BOX(config_subtitle_codepage), i);
        }
    }
    config_audio_channels = gtk_combo_box_text_new_with_entry();
    if (config_audio_channels != NULL) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_audio_channels), "Stereo");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_audio_channels), "Surround");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_audio_channels), "5.1 Surround");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_audio_channels), "7.1 Surround");

        gtk_combo_box_set_active(GTK_COMBO_BOX(config_audio_channels), audio_channels);
    }
    config_resume_mode = gtk_combo_box_text_new_with_entry();
    if (config_resume_mode != NULL) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_resume_mode), _("Always ask"));
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_resume_mode), _("Always resume without asking"));
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_resume_mode), _("Never resume"));

        gtk_combo_box_set_active(GTK_COMBO_BOX(config_resume_mode), resume_mode);
    }

    i = 0;
    j = -1;

    config_mplayer_dvd_device = gtk_combo_box_text_new_with_entry();

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_mplayer_dvd_device), "/dev/dvd");
    if (mplayer_dvd_device == NULL || g_ascii_strcasecmp("/dev/dvd", mplayer_dvd_device) == 0) {
        j = i;
    }

    i++;

    GVolumeMonitor *volumemonitor;
    GList *d, *drives;
    GDrive *drive;
    gchar *unix_device;
    volumemonitor = g_volume_monitor_get();
    if (volumemonitor != NULL) {
        drives = g_volume_monitor_get_connected_drives(volumemonitor);

        for (d = drives; d != NULL; d = d->next) {
            drive = G_DRIVE(d->data);
            if (g_drive_can_poll_for_media(drive)) {
                unix_device = g_drive_get_identifier(drive, "unix-device");
                if (unix_device != NULL) {
                    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_mplayer_dvd_device), unix_device);

                    if (mplayer_dvd_device != NULL && g_ascii_strcasecmp(unix_device, mplayer_dvd_device) == 0) {
                        j = i;
                    }
                    g_free(unix_device);
                    i++;
                }
            }
        }

    }
    if (j != -1) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(config_mplayer_dvd_device), j);
    } else {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(config_mplayer_dvd_device), mplayer_dvd_device);
        gtk_combo_box_set_active(GTK_COMBO_BOX(config_mplayer_dvd_device), i);
    }


    /// page 0
    conf_table = page_table[0];
    i = 0;

    conf_label = gtk_label_new(_("<span weight=\"bold\">Language Settings</span>"));
    gtk_label_set_use_markup(GTK_LABEL(conf_label), TRUE);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_START);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Default Audio Language"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    gtk_widget_set_size_request(GTK_WIDGET(config_alang), 200, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_alang, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    gtk_widget_set_tooltip_text(conf_label,
                                _("Choose one of the languages or type in your own comma-separated selection"));
    i++;

    conf_label = gtk_label_new(_("Default Subtitle Language:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);;
    gtk_widget_show(conf_label);
    gtk_widget_set_size_request(GTK_WIDGET(config_slang), 200, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_slang, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    gtk_widget_set_tooltip_text(conf_label,
                                _("Choose one of the languages or type in your own comma-separated selection"));
    i++;

    conf_label = gtk_label_new(_("File Metadata Encoding:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    gtk_widget_set_size_request(GTK_WIDGET(config_metadata_codepage), 200, -1);

    gtk_table_attach(GTK_TABLE(conf_table), config_metadata_codepage, 1, 2, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new("");
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    i++;

    conf_label = gtk_label_new(_("<span weight=\"bold\">Output Settings</span>"));
    gtk_label_set_use_markup(GTK_LABEL(conf_label), TRUE);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_START);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Video Output:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    gtk_widget_set_size_request(GTK_WIDGET(config_vo), 200, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_vo, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    config_hardware_codecs = gtk_check_button_new_with_label(_("Enable Video Hardware Support"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_hardware_codecs), use_hardware_codecs);
    gtk_widget_set_tooltip_text(config_hardware_codecs,
                                _
                                ("When this option is enabled, codecs or options will be enabled to accelerate video processing. These options may cause playback to fail in some cases."));
    gtk_widget_set_size_request(GTK_WIDGET(config_hardware_codecs), 200, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_hardware_codecs, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK,
                     0, 0);
    i++;

    config_crystalhd_codecs = gtk_check_button_new_with_label(_("Enable CrystalHD Hardware Support"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_crystalhd_codecs), use_crystalhd_codecs);
    gtk_widget_set_size_request(GTK_WIDGET(config_crystalhd_codecs), 200, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_crystalhd_codecs, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK,
                     0, 0);
    i++;

    conf_label = gtk_label_new(_("Audio Output:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    gtk_widget_set_size_request(GTK_WIDGET(config_ao), 200, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_ao, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Audio Volume Type:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    gtk_widget_set_halign (conf_volume_label, GTK_ALIGN_START);
    gtk_table_attach(GTK_TABLE(conf_table), conf_volume_label, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

#ifdef HAVE_ASOUNDLIB
    conf_label = gtk_label_new(_("Default Mixer:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    gtk_widget_set_size_request(GTK_WIDGET(config_mixer), 200, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_mixer, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;
#endif
    conf_label = gtk_label_new(_("Audio Channels to Output:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    gtk_widget_set_size_request(GTK_WIDGET(config_audio_channels), 200, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_audio_channels, 1, 2, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_use_hw_audio), use_hw_audio);
    gtk_table_attach(GTK_TABLE(conf_table), config_use_hw_audio, 1, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(config_use_hw_audio);
    i++;

    conf_label = gtk_label_new("");
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_EXPAND, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    i++;

    conf_label = gtk_label_new(_("<span weight=\"bold\">Configuration Settings</span>"));
    gtk_label_set_use_markup(GTK_LABEL(conf_label), TRUE);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_START);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

#ifndef HAVE_ASOUNDLIB
    conf_label = gtk_label_new(_("Default Volume Level:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    config_volume = gtk_spin_button_new_with_range(0, 100, 1);
    gtk_widget_set_tooltip_text(config_volume, _("Default volume for playback"));
    gtk_widget_set_size_request(config_volume, 100, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_volume, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    gm_store = gm_pref_store_new("g-mplayer");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_volume), gm_pref_store_get_int(gm_store, VOLUME));
    gm_pref_store_free(gm_store);
    gtk_entry_set_width_chars(GTK_ENTRY(config_volume), 6);
    gtk_editable_set_editable(GTK_EDITABLE(config_volume), TRUE);
    gtk_entry_set_alignment(GTK_ENTRY(config_volume), 1);
    gtk_widget_show(config_volume);
    i++;
#endif

    conf_label = gtk_label_new(_("On Screen Display Level:"));
    config_osdlevel = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, 3.0, 1.0);
    gtk_range_set_value(GTK_RANGE(config_osdlevel), osdlevel);
    g_signal_connect(G_OBJECT(config_osdlevel), "format-value", G_CALLBACK(osdlevel_format_callback), NULL);
    g_signal_connect(G_OBJECT(config_osdlevel), "value-changed", G_CALLBACK(osdlevel_change_callback), NULL);
    gtk_widget_set_size_request(config_osdlevel, 150, -1);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(conf_table), config_osdlevel, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Post-processing level:"));
    config_pplevel = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, 6.0, 1.0);
    g_signal_connect(G_OBJECT(config_pplevel), "format-value", G_CALLBACK(pplevel_format_callback), NULL);
    gtk_widget_set_size_request(config_pplevel, 150, -1);
    gtk_range_set_value(GTK_RANGE(config_pplevel), pplevel);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(conf_table), config_pplevel, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;


    /// Page 1
    conf_table = page_table[1];
    i = 0;

    conf_label = gtk_label_new(_("<span weight=\"bold\">Subtitle Settings</span>"));
    gtk_label_set_use_markup(GTK_LABEL(conf_label), TRUE);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_START);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    config_ass = gtk_check_button_new_with_mnemonic(_("Enable _Advanced Substation Alpha (ASS) Subtitle Support"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_ass), !disable_ass);
    g_signal_connect(G_OBJECT(config_ass), "toggled", G_CALLBACK(ass_toggle_callback), NULL);
    gtk_table_attach(GTK_TABLE(conf_table), config_ass, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(config_ass);
    i++;

    config_embeddedfonts = gtk_check_button_new_with_mnemonic(_("Use _Embedded Fonts (MKV only)"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_embeddedfonts), !disable_embeddedfonts);
    gtk_widget_set_sensitive(config_embeddedfonts, !disable_ass);
    gtk_table_attach(GTK_TABLE(conf_table), config_embeddedfonts, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(config_embeddedfonts);
    i++;

    config_subtitle_outline = gtk_check_button_new_with_label(_("Outline Subtitle Font"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_subtitle_outline), subtitle_outline);
    gtk_table_attach(GTK_TABLE(conf_table), config_subtitle_outline, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_subtitle_shadow = gtk_check_button_new_with_label(_("Shadow Subtitle Font"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_subtitle_shadow), subtitle_shadow);
    gtk_table_attach(GTK_TABLE(conf_table), config_subtitle_shadow, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_show_subtitles = gtk_check_button_new_with_label(_("Show Subtitles by Default"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_show_subtitles), showsubtitles);
    gtk_table_attach(GTK_TABLE(conf_table), config_show_subtitles, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Subtitle Font:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);

    config_subtitle_font = gtk_font_button_new();
    if (subtitlefont != NULL) {
        gtk_font_button_set_font_name(GTK_FONT_BUTTON(config_subtitle_font), subtitlefont);
    }
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(config_subtitle_font), TRUE);
    gtk_font_button_set_show_style(GTK_FONT_BUTTON(config_subtitle_font), TRUE);

    gtk_font_button_set_use_size(GTK_FONT_BUTTON(config_subtitle_font), FALSE);
    gtk_font_button_set_use_font(GTK_FONT_BUTTON(config_subtitle_font), TRUE);

    gtk_font_button_set_title(GTK_FONT_BUTTON(config_subtitle_font), _("Subtitle Font Selection"));
    gtk_table_attach(GTK_TABLE(conf_table), config_subtitle_font, 1, 2, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Subtitle Color:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    config_subtitle_color = gtk_color_button_new();
    if (subtitle_color != NULL && strlen(subtitle_color) > 5) {
        sub_color.red = g_ascii_xdigit_value(subtitle_color[0]) << 4;
        sub_color.red += g_ascii_xdigit_value(subtitle_color[1]);
        sub_color.red = sub_color.red << 8;
        sub_color.green = g_ascii_xdigit_value(subtitle_color[2]) << 4;
        sub_color.green += g_ascii_xdigit_value(subtitle_color[3]);
        sub_color.green = sub_color.green << 8;
        sub_color.blue = g_ascii_xdigit_value(subtitle_color[4]) << 4;
        sub_color.blue += g_ascii_xdigit_value(subtitle_color[5]);
        sub_color.blue = sub_color.blue << 8;
        gtk_color_button_set_color(GTK_COLOR_BUTTON(config_subtitle_color), &sub_color);
    } else {
        sub_color.red = 0xFF << 8;
        sub_color.green = 0xFF << 8;
        sub_color.blue = 0xFF << 8;
        gtk_color_button_set_color(GTK_COLOR_BUTTON(config_subtitle_color), &sub_color);
    }
    gtk_color_button_set_title(GTK_COLOR_BUTTON(config_subtitle_color), _("Subtitle Color Selection"));
    gtk_widget_set_sensitive(config_subtitle_color, !disable_ass);
    gtk_table_attach(GTK_TABLE(conf_table), config_subtitle_color, 1, 2, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Subtitle Font Scaling:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    config_subtitle_scale = gtk_spin_button_new_with_range(0.25, 10, 0.05);
    gtk_widget_set_size_request(config_subtitle_scale, -1, -1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_subtitle_scale), subtitle_scale);
    gtk_table_attach(GTK_TABLE(conf_table), config_subtitle_scale, 1, 2, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Subtitle File Encoding:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    gtk_widget_set_size_request(GTK_WIDGET(config_subtitle_codepage), 200, -1);

    gtk_table_attach(GTK_TABLE(conf_table), config_subtitle_codepage, 1, 2, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Subtitle Lower Margin (X11/XV Only):"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    config_subtitle_margin = gtk_spin_button_new_with_range(0, 200, 1);
    gtk_widget_set_size_request(config_subtitle_scale, -1, -1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_subtitle_margin), subtitle_margin);
    gtk_table_attach(GTK_TABLE(conf_table), config_subtitle_margin, 1, 2, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Subtitle Load Fuzziness:"));
    config_subtitle_fuzziness = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, 2.0, 1.0);
    g_signal_connect(G_OBJECT(config_subtitle_fuzziness), "format-value",
                     G_CALLBACK(subtitle_fuzziness_format_callback), NULL);
    gtk_widget_set_size_request(config_subtitle_fuzziness, 150, -1);
    gtk_range_set_value(GTK_RANGE(config_subtitle_fuzziness), subtitle_fuzziness);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(conf_table), config_subtitle_fuzziness, 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND,
                     GTK_SHRINK, 0, 0);
    i++;


    /// Page 2
    conf_table = page_table[2];
    i = 0;

    conf_label = gtk_label_new(_("<span weight=\"bold\">Application Preferences</span>"));
    gtk_label_set_use_markup(GTK_LABEL(conf_label), TRUE);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_START);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Media Resume:"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_START);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    gtk_widget_set_size_request(GTK_WIDGET(config_resume_mode), 200, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_resume_mode, 1, 2, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    config_playlist_visible = gtk_check_button_new_with_label(_("Start with playlist visible"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_playlist_visible), playlist_visible);
    gtk_table_attach(GTK_TABLE(conf_table), config_playlist_visible, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_details_visible = gtk_check_button_new_with_label(_("Start with details visible"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_details_visible), details_visible);
    gtk_table_attach(GTK_TABLE(conf_table), config_details_visible, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_use_mediakeys = gtk_check_button_new_with_label(_("Respond to Keyboard Media Keys"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_use_mediakeys), use_mediakeys);
    gtk_table_attach(GTK_TABLE(conf_table), config_use_mediakeys, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_use_defaultpl = gtk_check_button_new_with_label(_("Use default playlist"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_use_defaultpl), use_defaultpl);
    gtk_table_attach(GTK_TABLE(conf_table), config_use_defaultpl, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_show_status_icon = gtk_check_button_new_with_label(_("Show status icon"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_show_status_icon), show_status_icon);
    gtk_table_attach(GTK_TABLE(conf_table), config_show_status_icon, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_vertical_layout = gtk_check_button_new_with_label(_("Place playlist below media (requires application restart)"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_vertical_layout), vertical_layout);
    gtk_table_attach(GTK_TABLE(conf_table), config_vertical_layout, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_single_instance = gtk_check_button_new_with_label(_("Only allow one instance of G-MPlayer"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_single_instance), single_instance);
    gtk_table_attach(GTK_TABLE(conf_table), config_single_instance, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    g_signal_connect(G_OBJECT(config_single_instance), "toggled", G_CALLBACK(config_single_instance_callback), NULL);
    i++;

    conf_label = gtk_label_new("");
    gtk_label_set_width_chars(GTK_LABEL(conf_label), 3);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
    config_replace_and_play =
        gtk_check_button_new_with_label(_("When opening in single instance mode, replace existing file"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_replace_and_play), replace_and_play);
    gtk_table_attach(GTK_TABLE(conf_table), config_replace_and_play, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_set_sensitive(config_replace_and_play,
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_single_instance)));
    i++;
    conf_label = gtk_label_new("");
    gtk_label_set_width_chars(GTK_LABEL(conf_label), 3);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
    config_bring_to_front = gtk_check_button_new_with_label(_("When opening file, bring main window to front"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_bring_to_front), bring_to_front);
    gtk_table_attach(GTK_TABLE(conf_table), config_bring_to_front, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_set_sensitive(config_bring_to_front,
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_single_instance)));

    i++;
    config_remember_loc = gtk_check_button_new_with_label(_("Remember Window Location and Size"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_remember_loc), remember_loc);
    gtk_table_attach(GTK_TABLE(conf_table), config_remember_loc, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_resize_on_new_media = gtk_check_button_new_with_label(_("Resize window when new video is loaded"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_resize_on_new_media), resize_on_new_media);
    gtk_table_attach(GTK_TABLE(conf_table), config_resize_on_new_media, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_keep_on_top = gtk_check_button_new_with_label(_("Keep window above other windows"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_keep_on_top), keep_on_top);
    gtk_table_attach(GTK_TABLE(conf_table), config_keep_on_top, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_pause_on_click = gtk_check_button_new_with_label(_("Pause playback on mouse click"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_pause_on_click), !disable_pause_on_click);
    gtk_table_attach(GTK_TABLE(conf_table), config_pause_on_click, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_disable_animation = gtk_check_button_new_with_label(_("Disable Fullscreen Control Bar Animation"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_disable_animation), disable_animation);
    gtk_table_attach(GTK_TABLE(conf_table), config_disable_animation, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_disable_cover_art_fetch = gtk_check_button_new_with_label(_("Disable Cover Art Fetch"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_disable_cover_art_fetch), disable_cover_art_fetch);
    gtk_table_attach(GTK_TABLE(conf_table), config_disable_cover_art_fetch, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_mouse_wheel = gtk_check_button_new_with_label(_("Use Mouse Wheel to change volume, instead of seeking"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_mouse_wheel), mouse_wheel_changes_volume);
    gtk_table_attach(GTK_TABLE(conf_table), config_mouse_wheel, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_verbose = gtk_check_button_new_with_label(_("Verbose Debug Enabled"));
    gtk_widget_set_tooltip_text(config_verbose,
                                _
                                ("When this option is set, extra debug information is sent to the terminal or into ~/.xsession-errors"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_verbose), verbose);
    gtk_table_attach(GTK_TABLE(conf_table), config_verbose, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    /// page 3
    conf_table = page_table[3];
    i = 0;

    conf_label = gtk_label_new(_("<span weight=\"bold\">Keyboard Shortcuts</span>\n\n"
                                 "Place the cursor in the box next to the shortcut you want to modify\n"
                                 "Then press the keys you would like for the shortcut"));
    gtk_label_set_use_markup(GTK_LABEL(conf_label), TRUE);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_START);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 3, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    for (j = 0; j < KEY_COUNT; j++) {
        conf_label = gtk_label_new(accel_keys_description[j]);
        gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
        gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
        gtk_widget_show(conf_label);

        config_accel_keys[j] = gtk_entry_new();

        if (get_key_and_modifier(accel_keys[j], &key, &modifier)) {
            gtk_entry_set_text(GTK_ENTRY(config_accel_keys[j]),
                               g_strdup_printf("%s%s%s%s", (modifier & GDK_CONTROL_MASK) ? "Control-" : "",
                                               (modifier & GDK_MOD1_MASK) ? "Alt-" : "",
                                               (modifier & GDK_SHIFT_MASK) ? "Shift-" : "", gdk_keyval_name(key)));
        }
        g_signal_connect(G_OBJECT(config_accel_keys[j]), "key_press_event", G_CALLBACK(accel_key_key_press_event),
                         GINT_TO_POINTER(j));
        gtk_widget_set_size_request(GTK_WIDGET(config_accel_keys[j]), 150, -1);
        gtk_table_attach(GTK_TABLE(conf_table), config_accel_keys[j], 1, 2, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK,
                         0, 0);

        i++;
    }
    conf_label = gtk_button_new_with_label(_("Reset Keys"));
    g_signal_connect(G_OBJECT(conf_label), "clicked", G_CALLBACK(reset_keys_callback), NULL);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;


    /// page 4
    conf_table = page_table[4];
    i = 0;

    conf_label = gtk_label_new(_("<span weight=\"bold\">Advanced Settings for MPlayer</span>"));
    gtk_label_set_use_markup(GTK_LABEL(conf_label), TRUE);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_START);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 3, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    gtk_widget_set_tooltip_text(config_softvol,
                                _("Set this option if changing the volume in G-MPlayer changes the master volume"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_softvol), softvol);
    gtk_table_attach(GTK_TABLE(conf_table), config_softvol, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    g_signal_connect(G_OBJECT(config_softvol), "toggled", G_CALLBACK(config_softvol_callback), NULL);
    i++;

    conf_label = gtk_label_new("");
    gtk_label_set_width_chars(GTK_LABEL(conf_label), 3);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 1, 2, i, i + 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
    config_remember_softvol = gtk_check_button_new_with_label(_("Remember last software volume level"));
    gtk_widget_set_tooltip_text(config_remember_softvol,
                                _("Set this option if you want the software volume level to be remembered"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_remember_softvol), remember_softvol);
    gtk_table_attach(GTK_TABLE(conf_table), config_remember_softvol, 1, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_set_sensitive(config_remember_softvol, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_softvol)));
    i++;

    conf_label = gtk_label_new(_("Volume Gain (-200dB to +60dB)"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 1, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    config_volume_gain = gtk_spin_button_new_with_range(-200, 60, 1);
    gtk_widget_set_size_request(config_volume_gain, -1, -1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_volume_gain), volume_gain);
    gtk_table_attach(GTK_TABLE(conf_table), config_volume_gain, 2, 3, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    gtk_widget_set_sensitive(config_volume_gain, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_softvol)));
    i++;

    config_deinterlace = gtk_check_button_new_with_mnemonic(_("De_interlace Video"));
    gtk_widget_set_tooltip_text(config_deinterlace, _("Set this option if video looks striped"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_deinterlace), !disable_deinterlace);
    gtk_table_attach(GTK_TABLE(conf_table), config_deinterlace, 0, 3, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_framedrop = gtk_check_button_new_with_mnemonic(_("_Drop frames"));
    gtk_widget_set_tooltip_text(config_framedrop, _("Set this option if video is well behind the audio"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_framedrop), !disable_framedrop);
    gtk_table_attach(GTK_TABLE(conf_table), config_framedrop, 0, 3, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    config_forcecache = gtk_check_button_new_with_label(_("Enable mplayer cache"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_forcecache), forcecache);
    gtk_table_attach(GTK_TABLE(conf_table), config_forcecache, 0, 3, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    g_signal_connect(G_OBJECT(config_forcecache), "toggled", G_CALLBACK(config_forcecache_callback), NULL);
    i++;

    conf_label = gtk_label_new(_("Cache Size (KB):"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    config_cachesize = gtk_spin_button_new_with_range(32, 256 * 1024, 512);
    gtk_widget_set_tooltip_text(config_cachesize, _
                                ("Amount of data to cache when playing media, use higher values for slow devices and sites."));
    gtk_widget_set_size_request(config_cachesize, 100, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_cachesize, 2, 3, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_cachesize), cache_size);
    gtk_entry_set_width_chars(GTK_ENTRY(config_cachesize), 6);
    gtk_entry_set_alignment(GTK_ENTRY(config_cachesize), 1);
    gtk_widget_show(config_cachesize);
    gtk_widget_set_sensitive(config_cachesize, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_forcecache)));
    i++;

    conf_label = gtk_label_new("");
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("MPlayer Executable:"));
    config_mplayer_bin = gtk_file_chooser_button_new("Select", GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_widget_set_tooltip_text(config_mplayer_bin, _
                                ("Use this option to specify a mplayer application that is not in the path"));
    if (mplayer_bin != NULL)
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(config_mplayer_bin), mplayer_bin);
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(conf_table), config_mplayer_bin, 2, 3, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new("");
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("MPlayer Default Optical Device"));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_END);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_widget_show(conf_label);
    gtk_widget_set_size_request(GTK_WIDGET(config_mplayer_dvd_device), 200, -1);
    gtk_table_attach(GTK_TABLE(conf_table), config_mplayer_dvd_device, 2, 3, i, i + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new("");
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;

    conf_label = gtk_label_new(_("Extra Options to MPlayer:"));
    config_extraopts = gtk_entry_new();
    gtk_widget_set_tooltip_text(config_extraopts, _("Add any extra mplayer options here (filters etc)"));
    gtk_entry_set_text(GTK_ENTRY(config_extraopts), ((extraopts) ? extraopts : ""));
    gtk_widget_set_halign (conf_label, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(config_extraopts), 40);
    gtk_table_attach(GTK_TABLE(conf_table), conf_label, 0, 2, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
    i++;
    gtk_table_attach(GTK_TABLE(conf_table), config_extraopts, 0, 3, i, i + 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    i++;

    // --
    gtk_container_add(GTK_CONTAINER(conf_hbutton_box), conf_cancel);
    gtk_container_add(GTK_CONTAINER(conf_vbox), conf_hbutton_box);

    gtk_widget_show_all(config_window);
    gtk_window_set_transient_for(GTK_WINDOW(config_window), GTK_WINDOW(window));
    gtk_window_set_keep_above(GTK_WINDOW(config_window), keep_on_top);
    gtk_window_present(GTK_WINDOW(config_window));
}

void reset_keys_callback(GtkButton * button, gpointer data)
{
    gint j;
    guint key;
    GdkModifierType modifier;

    for (j = 0; j < KEY_COUNT; j++) {
        g_free(accel_keys[j]);
        accel_keys[j] = NULL;
    }

    assign_default_keys();

    for (j = 0; j < KEY_COUNT; j++) {
        if (get_key_and_modifier(accel_keys[j], &key, &modifier)) {
            gtk_entry_set_text(GTK_ENTRY(config_accel_keys[j]),
                               g_strdup_printf("%s%s%s%s", (modifier & GDK_CONTROL_MASK) ? "Control-" : "",
                                               (modifier & GDK_MOD1_MASK) ? "Alt-" : "",
                                               (modifier & GDK_SHIFT_MASK) ? "Shift-" : "", gdk_keyval_name(key)));
        }
    }
}

gboolean accel_key_key_press_event(GtkWidget * widget, GdkEventKey * event, gpointer data)
{

    gint index = GPOINTER_TO_INT(data);

    gm_log(verbose, G_LOG_LEVEL_DEBUG, "key press %s shift: %s, ctrl: %s, alt: %s modifier: %s",
           gdk_keyval_name(event->keyval), gm_bool_to_string(event->state & GDK_SHIFT_MASK),
           gm_bool_to_string(event->state & GDK_CONTROL_MASK), gm_bool_to_string(event->state & GDK_MOD1_MASK),
           gm_bool_to_string(event->is_modifier));


    if (event->is_modifier)
        return TRUE;

    event->state = event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_SHIFT_MASK);
    gtk_entry_set_text(GTK_ENTRY(widget),
                       g_strdup_printf("%s%s%s%s", (event->state & GDK_CONTROL_MASK) ? "Control-" : "",
                                       (event->state & GDK_MOD1_MASK) ? "Alt-" : "",
                                       (event->state & GDK_SHIFT_MASK) ? "Shift-" : "",
                                       gdk_keyval_name(event->keyval)));

    g_free(accel_keys[index]);
    accel_keys[index] = g_strdup_printf("%i+%s", event->state, gdk_keyval_name(event->keyval));
    setup_accelerators();


    return TRUE;
}


gboolean tracker_callback(GtkWidget * widget, gint percent, void *data)
{

    if (!idledata->streaming) {
        if (!autopause) {
            if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP) {
                gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), (gdouble) percent / 100.0, SEEK_PERCENT);
            }
        }
    }

    return FALSE;
}

void subtitle_select_callback(GtkMenuItem * menu, gpointer data)
{
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu)))
        gmtk_media_player_select_subtitle_by_id(GMTK_MEDIA_PLAYER(media), GPOINTER_TO_INT(data));
}

void player_subtitle_callback(GmtkMediaPlayer * player, int count, gpointer data)
{
    GtkWidget *menuitem_select_subtitles = GTK_WIDGET(data);
    GtkWidget *sub;
    GList *iter;
    GmtkMediaPlayerSubtitle *subtitle;
    GSList *group = NULL;

    subtitles = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem_select_subtitles), NULL);

    if (g_list_length(player->subtitles)) {
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem_select_subtitles), subtitles);

        iter = player->subtitles;
        while (iter) {
            subtitle = (GmtkMediaPlayerSubtitle *) iter->data;
            sub = gtk_radio_menu_item_new_with_label(group, subtitle->label);
            group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(sub));
            g_signal_connect(sub, "activate", G_CALLBACK(subtitle_select_callback), GINT_TO_POINTER(subtitle->id));
            gtk_menu_shell_append(GTK_MENU_SHELL(subtitles), sub);
            iter = iter->next;
        }

        gtk_widget_show_all(subtitles);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_select_sub_lang), TRUE);
    }

}

void audio_track_select_callback(GtkMenuItem * menu, gpointer data)
{
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu))) {
        gmtk_media_player_select_audio_track(GMTK_MEDIA_PLAYER(media), gtk_menu_item_get_label(menu));
    }
}

void player_audio_track_callback(GmtkMediaPlayer * player, int count, gpointer data)
{
    GtkWidget *menuitem_select_audio_track = GTK_WIDGET(data);
    GtkWidget *menuitem;
    GList *iter;
    GmtkMediaPlayerAudioTrack *audio_track;
    GSList *group = NULL;
    tracks = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem_select_audio_track), NULL);
    if (g_list_length(player->audio_tracks)) {

        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem_select_audio_track), tracks);
        iter = player->audio_tracks;
        while (iter) {
            audio_track = (GmtkMediaPlayerAudioTrack *) iter->data;
            menuitem = gtk_radio_menu_item_new_with_label(group, audio_track->label);
            group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitem));
            g_signal_connect(menuitem, "activate",
                             G_CALLBACK(audio_track_select_callback), GINT_TO_POINTER(audio_track->id));
            gtk_menu_shell_append(GTK_MENU_SHELL(tracks), menuitem);
            iter = iter->next;
        }

        gtk_widget_show_all(tracks);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_select_audio_lang), TRUE);
    }

}

void player_attribute_changed_callback(GmtkMediaTracker * tracker, GmtkMediaPlayerMediaAttributes attribute)
{
    const gchar *name;
    GList *list;
    GtkMenuItem *item;
    gchar *text;
    gchar *title;
    gchar *buffer;
    MetaData *metadata;
    GtkTreeIter *citer = NULL;


    switch (attribute) {
    case ATTRIBUTE_LENGTH:
        if (GTK_IS_WIDGET(tracker)) {
            gmtk_media_tracker_set_length(GMTK_MEDIA_TRACKER(tracker),
                                          gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), attribute));
        }
        metadata = g_new0(MetaData, 1);
        metadata->uri = g_strdup(gmtk_media_player_get_uri(GMTK_MEDIA_PLAYER(media)));

        metadata->length_value = gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), attribute);
        if (metadata->length)
            g_free(metadata->length);
        metadata->length =
            seconds_to_string(gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), attribute));

        free_metadata(metadata);

        break;
    case ATTRIBUTE_SIZE:
        if (remember_loc) {
            idledata->width = loc_window_width;
            idledata->height = loc_window_height;
        } else {
            idledata->width = (gint) gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_WIDTH);
            idledata->height =
                (gint) gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HEIGHT);
        }
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "video present = %s new size %i x %i",
               gm_bool_to_string(idledata->videopresent), idledata->width, idledata->height);
        text = g_strdup_printf("%i x %i", idledata->width, idledata->height);
        gtk_label_set_text(GTK_LABEL(details_video_size), text);
        g_free(text);
        if (resize_on_new_media || idledata->videopresent == FALSE) {
            if (idledata->width > 0 && idledata->height > 0)
                idledata->videopresent = TRUE;
            g_idle_add(resize_window, idledata);
        }
        break;
    case ATTRIBUTE_VIDEO_PRESENT:
        idledata->videopresent =
            gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_VIDEO_PRESENT);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_fullscreen), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(fs_event_box), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_set_subtitle), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_set_audiofile), idledata->videopresent);
#ifdef GTK3_ENABLED
        gtk_window_set_has_resize_grip(GTK_WINDOW(window), idledata->videopresent);
#endif
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_take_screenshot), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_fullscreen), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_onetoone), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_onetotwo), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_twotoone), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_onetoonepointfive), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_advanced), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_default), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_four_three), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_sixteen_nine), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_sixteen_ten), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_anamorphic), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_follow_window), idledata->videopresent);
        if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBS_EXIST)) {
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_subtitles), idledata->videopresent);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_smaller_subtitle), idledata->videopresent);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_larger_subtitle), idledata->videopresent);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_decrease_subtitle_delay), idledata->videopresent);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_increase_subtitle_delay), idledata->videopresent);
        } else {
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_subtitles), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_smaller_subtitle), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_larger_subtitle), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_decrease_subtitle_delay), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_increase_subtitle_delay), FALSE);
        }
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_angle), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_advanced), idledata->videopresent);
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_details), TRUE);
        if (resize_on_new_media || idledata->videopresent == FALSE) {
            if (idledata->width > 0 && idledata->height > 0)
                idledata->videopresent = TRUE;
            g_idle_add(resize_window, idledata);
        }
        break;
    case ATTRIBUTE_AUDIO_TRACK:
        name = gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_TRACK);
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "track name = %s", name);
        if (name != NULL && GTK_IS_WIDGET(tracks)) {
            list = gtk_container_get_children(GTK_CONTAINER(tracks));
            while (list) {
                item = GTK_MENU_ITEM(list->data);
                if (g_ascii_strcasecmp(name, gtk_menu_item_get_label(item)) == 0) {
                    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
                }
                list = list->next;
            }
        }
        break;
    case ATTRIBUTE_AF_EXPORT_FILENAME:
        unmap_af_export_file(idledata);
        map_af_export_file(idledata);
        break;
    case ATTRIBUTE_SUBTITLE:
        name = gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE);
        if (name != NULL && GTK_IS_WIDGET(subtitles)) {
            list = gtk_container_get_children(GTK_CONTAINER(subtitles));
            while (list) {
                item = GTK_MENU_ITEM(list->data);
                if (g_ascii_strcasecmp(name, gtk_menu_item_get_label(item)) == 0) {
                    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
                }
                list = list->next;
            }
        }
        gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_subtitles),
                                 gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media),
                                                                         ATTRIBUTE_SUBS_EXIST));

        break;
    case ATTRIBUTE_SUB_VISIBLE:
        if (showsubtitles) {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_subtitles),
                                           gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media),
                                                                                   ATTRIBUTE_SUB_VISIBLE));
        }
        break;
    case ATTRIBUTE_HAS_CHAPTERS:
        if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HAS_CHAPTERS)
            || gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore), NULL) > 1) {
            gtk_widget_show_all(prev_event_box);
            gtk_widget_show_all(next_event_box);
        } else {
            gtk_widget_hide(prev_event_box);
            gtk_widget_hide(next_event_box);
        }
        break;
    case ATTRIBUTE_MESSAGE:
        gmtk_media_tracker_set_text(GMTK_MEDIA_TRACKER(tracker),
                                    gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media),
                                                                           ATTRIBUTE_MESSAGE));
        break;
    case ATTRIBUTE_VIDEO_FORMAT:
        text =
            g_ascii_strup(gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_VIDEO_FORMAT), -1);
        gtk_label_set_text(GTK_LABEL(details_video_format), text);
        g_free(text);
        break;
    case ATTRIBUTE_VIDEO_CODEC:
        text =
            g_ascii_strup(gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_VIDEO_CODEC), -1);
        gtk_label_set_text(GTK_LABEL(details_video_codec), text);
        g_free(text);
        break;
    case ATTRIBUTE_VIDEO_FPS:
        text =
            g_strdup_printf("%2.1lf",
                            gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_VIDEO_FPS));
        gtk_label_set_text(GTK_LABEL(details_video_fps), text);
        g_free(text);
        break;
    case ATTRIBUTE_AUDIO_FORMAT:
        text =
            g_ascii_strup(gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_FORMAT), -1);
        gtk_label_set_text(GTK_LABEL(details_audio_format), text);
        g_free(text);
        break;
    case ATTRIBUTE_AUDIO_CODEC:
        text =
            g_ascii_strup(gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_CODEC), -1);
        gtk_label_set_text(GTK_LABEL(details_audio_codec), text);
        g_free(text);
        break;
    case ATTRIBUTE_VIDEO_BITRATE:
        text =
            g_strdup_printf("%i Kb/s",
                            gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media),
                                                                    ATTRIBUTE_VIDEO_BITRATE) / 1000);
        gtk_label_set_text(GTK_LABEL(details_video_bitrate), text);
        g_free(text);
        break;
    case ATTRIBUTE_AUDIO_BITRATE:
        text =
            g_strdup_printf("%i Kb/s",
                            gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media),
                                                                    ATTRIBUTE_AUDIO_BITRATE) / 1000);
        gtk_label_set_text(GTK_LABEL(details_audio_bitrate), text);
        g_free(text);
        break;
    case ATTRIBUTE_AUDIO_RATE:
        text =
            g_strdup_printf("%i KHz",
                            gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media),
                                                                    ATTRIBUTE_AUDIO_RATE) / 1000);
        gtk_label_set_text(GTK_LABEL(details_audio_samplerate), text);
        g_free(text);
        break;
    case ATTRIBUTE_AUDIO_NCH:
        text =
            g_strdup_printf("%i",
                            gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_NCH));
        gtk_label_set_text(GTK_LABEL(details_audio_channels), text);
        g_free(text);
        break;
    case ATTRIBUTE_CHAPTERS:
        text =
            g_strdup_printf("%i",
                            gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_CHAPTERS));
        gtk_label_set_text(GTK_LABEL(details_video_chapters), text);
        g_free(text);
        break;
    case ATTRIBUTE_SEEKABLE:
        gtk_widget_set_sensitive(GTK_WIDGET(tracker),
                                 gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SEEKABLE));
        break;
    case ATTRIBUTE_START_TIME:
        break;
    case ATTRIBUTE_TITLE:
    case ATTRIBUTE_ARTIST:
    case ATTRIBUTE_ALBUM:
        metadata = g_new0(MetaData, 1);
        metadata->uri = g_strdup(gmtk_media_player_get_uri(GMTK_MEDIA_PLAYER(media)));

        text = g_strdup_printf("<small>\n");
        if (gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_TITLE)) {
            buffer =
                g_markup_printf_escaped("\t<big><b>%s</b></big>\n",
                                        gmtk_media_player_get_attribute_string
                                        (GMTK_MEDIA_PLAYER(media), ATTRIBUTE_TITLE));
            text = g_strconcat(text, buffer, NULL);
            g_free(buffer);
            if (metadata->title)
                g_free(metadata->title);
            metadata->title =
                g_strstrip(g_strdup(gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_TITLE)));
            if (metadata->title != NULL && strlen(metadata->title) > 0) {
                title = g_strdup_printf(_("%s - Media Player"), metadata->title);
                gtk_window_set_title(GTK_WINDOW(window), title);
                g_free(title);
            }
        }

        if (gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ARTIST)) {
            buffer =
                g_markup_printf_escaped("\t<i>%s</i>\n",
                                        gmtk_media_player_get_attribute_string
                                        (GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ARTIST));
            text = g_strconcat(text, buffer, NULL);
            g_free(buffer);
            if (metadata->artist)
                g_free(metadata->artist);
            metadata->artist =
                g_strstrip(g_strdup
                           (gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ARTIST)));
        }

        if (gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ALBUM)) {
            buffer =
                g_markup_printf_escaped("\t%s\n",
                                        gmtk_media_player_get_attribute_string
                                        (GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ALBUM));
            text = g_strconcat(text, buffer, NULL);
            g_free(buffer);
            if (metadata->album)
                g_free(metadata->album);
            metadata->album =
                g_strstrip(g_strdup(gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ALBUM)));
        }

        free_metadata(metadata);

        text = g_strconcat(text, "</small>", NULL);
        gtk_label_set_markup(GTK_LABEL(media_label), text);
        g_strlcpy(idledata->media_info, text, sizeof(idledata->media_info));

        citer = find_iter_by_uri(gmtk_media_player_get_uri(GMTK_MEDIA_PLAYER(media)));
        if (gtk_list_store_iter_is_valid(playliststore, citer) && !g_thread_pool_unprocessed(retrieve_metadata_pool)) {
            switch (attribute) {
            case ATTRIBUTE_TITLE:
                gtk_list_store_set(playliststore, citer, DESCRIPTION_COLUMN, gmtk_media_player_get_attribute_string
                                   (GMTK_MEDIA_PLAYER(media), ATTRIBUTE_TITLE), -1);
                break;
            case ATTRIBUTE_ARTIST:
                gtk_list_store_set(playliststore, citer, ARTIST_COLUMN, gmtk_media_player_get_attribute_string
                                   (GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ARTIST), -1);
                break;

            case ATTRIBUTE_ALBUM:
                gtk_list_store_set(playliststore, citer, ALBUM_COLUMN, gmtk_media_player_get_attribute_string
                                   (GMTK_MEDIA_PLAYER(media), ATTRIBUTE_ALBUM), -1);
                break;
            default:
                break;
            }
        }
        g_free(citer);
        break;

    case ATTRIBUTE_RETRY_ON_FULL_CACHE:
        idledata->retry_on_full_cache =
            gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_RETRY_ON_FULL_CACHE);
        break;

    case ATTRIBUTE_SPEED_SET:
        break;

    default:
        gm_log(verbose, G_LOG_LEVEL_INFO, "Unhandled attribute change %i", attribute);
    }

}

void player_media_state_changed_callback(GtkButton * button, GmtkMediaPlayerMediaState media_state, gpointer data)
{
    gchar *tip_text = NULL;
    gchar *short_filename = NULL;
#ifdef GTK3_ENABLED
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    const gchar *icon_start;

    if (gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL)
        icon_start = "media-playback-start-rtl-symbolic";
    else
        icon_start = "media-playback-start-symbolic";
#endif

    gm_log(verbose, G_LOG_LEVEL_DEBUG, "in media state change with state = %s dontplaynext = %i",
           gmtk_media_state_to_string(media_state), dontplaynext);
    switch (media_state) {
        // mplayer is dead, need the next item off the playlist
    case MEDIA_STATE_QUIT:
    case MEDIA_STATE_UNKNOWN:
        if (idledata->mapped_af_export != NULL)
            unmap_af_export_file(idledata);
        if (dontplaynext == FALSE
            && gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_edit_play_single)) == FALSE) {
            if (next_item_in_playlist(&iter)) {
                g_idle_add(async_play_iter, &iter);
            } else {
                gm_log(verbose, G_LOG_LEVEL_DEBUG, "end of thread playlist is empty");
                if (loop) {
                    if (is_first_item_in_playlist(&iter)) {
                        g_idle_add(async_play_iter, &iter);
                    }
                } else {
                    idledata->fullscreen = 0;
                    g_idle_add(set_fullscreen, idledata);
                    g_idle_add(set_stop, idledata);
                }

                if (quit_on_complete) {
                    g_idle_add(set_quit, idledata);
                }
            }
        } else {
            if (next_iter != NULL) {
                g_idle_add(async_play_iter, next_iter);
                next_iter = NULL;
            }
        }
        dontplaynext = FALSE;
        // break purposely not put here, so gui is properly updated
    case MEDIA_STATE_STOP:
#ifdef GTK3_ENABLED
        if (gtk_icon_theme_has_icon(icon_theme, icon_start)) {
            gtk_image_set_from_icon_name(GTK_IMAGE(image_play), icon_start, button_size);
        } else {
            gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
        }
#else
        gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
#endif
        gmtk_media_tracker_set_position(GMTK_MEDIA_TRACKER(tracker), 0.0);
        tip_text = gtk_widget_get_tooltip_text(play_event_box);
        if (tip_text == NULL || g_ascii_strcasecmp(tip_text, _("Play")) != 0)
            gtk_widget_set_tooltip_text(play_event_box, _("Play"));
        g_free(tip_text);
        gtk_widget_set_sensitive(ff_event_box, FALSE);
        gtk_widget_set_sensitive(rew_event_box, FALSE);
        gtk_widget_set_sensitive(prev_event_box, FALSE);
        gtk_widget_set_sensitive(next_event_box, FALSE);
        gtk_widget_set_sensitive(stop_event_box, FALSE);

        if (GTK_WIDGET(gtk_widget_get_parent(GTK_WIDGET(menuitem_play))) == GTK_WIDGET(popup_menu))
            gtk_container_remove(GTK_CONTAINER(popup_menu), GTK_WIDGET(menuitem_play));
        menuitem_play = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-media-play", NULL));
        g_signal_connect(G_OBJECT(menuitem_play), "activate", G_CALLBACK(menuitem_play_callback), NULL);
        gtk_menu_shell_insert(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_play), 0);
        gtk_widget_show(GTK_WIDGET(menuitem_play));
        g_strlcpy(idledata->media_info, "", sizeof(idledata->media_info));
        gtk_widget_hide(media_hbox);
        g_strlcpy(idledata->display_name, "", sizeof(idledata->display_name));
        g_idle_add(set_title_bar, idledata);
        break;
    case MEDIA_STATE_PLAY:
        if (idledata->mapped_af_export == NULL)
            map_af_export_file(idledata);
#ifdef GTK3_ENABLED
        if (gtk_icon_theme_has_icon(icon_theme, "media-playback-pause-symbolic")) {
            gtk_image_set_from_icon_name(GTK_IMAGE(image_play), "media-playback-pause-symbolic", button_size);
        } else {
            gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-pause", button_size);
        }
#else
        gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-pause", button_size);
#endif
        tip_text = gtk_widget_get_tooltip_text(play_event_box);
        if (tip_text == NULL || g_ascii_strcasecmp(tip_text, _("Pause")) != 0)
            gtk_widget_set_tooltip_text(play_event_box, _("Pause"));
        g_free(tip_text);
        gtk_widget_set_sensitive(ff_event_box, TRUE);
        gtk_widget_set_sensitive(rew_event_box, TRUE);
        gtk_widget_set_sensitive(prev_event_box, TRUE);
        gtk_widget_set_sensitive(next_event_box, TRUE);
        gtk_widget_set_sensitive(stop_event_box, TRUE);

        if (GTK_WIDGET(gtk_widget_get_parent(GTK_WIDGET(menuitem_play))) == GTK_WIDGET(popup_menu))
            gtk_container_remove(GTK_CONTAINER(popup_menu), GTK_WIDGET(menuitem_play));
        menuitem_play = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-media-pause", NULL));
        g_signal_connect(G_OBJECT(menuitem_play), "activate", G_CALLBACK(menuitem_pause_callback), NULL);
        gtk_menu_shell_insert(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_play), 0);
        gtk_widget_show(GTK_WIDGET(menuitem_play));
        gmtk_media_tracker_set_text(GMTK_MEDIA_TRACKER(tracker), _("Playing"));
        gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUB_VISIBLE,
                                                gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_subtitles)));
        g_idle_add(set_media_label, idledata);
        if (gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_TITLE) != NULL) {
            g_strlcpy(idledata->display_name,
                      gmtk_media_player_get_attribute_string(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_TITLE),
                      sizeof(idledata->display_name));
        } else {
            short_filename = g_strrstr(gmtk_media_player_get_uri(GMTK_MEDIA_PLAYER(media)), "/");
            if (short_filename != NULL) {
                g_strlcpy(idledata->display_name, short_filename + sizeof(gchar), sizeof(idledata->display_name));
            } else {
                g_strlcpy(idledata->display_name, "", sizeof(idledata->display_name));
            }
        }
        g_idle_add(set_title_bar, idledata);
        break;
    case MEDIA_STATE_PAUSE:
#ifdef GTK3_ENABLED
        if (gtk_icon_theme_has_icon(icon_theme, icon_start)) {
            gtk_image_set_from_icon_name(GTK_IMAGE(image_play), icon_start, button_size);
        } else {
            gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
        }
#else
        gtk_image_set_from_stock(GTK_IMAGE(image_play), "gtk-media-play", button_size);
#endif
        tip_text = gtk_widget_get_tooltip_text(play_event_box);
        if (tip_text == NULL || g_ascii_strcasecmp(tip_text, _("Play")) != 0)
            gtk_widget_set_tooltip_text(play_event_box, _("Play"));
        g_free(tip_text);
        gtk_widget_set_sensitive(ff_event_box, FALSE);
        gtk_widget_set_sensitive(rew_event_box, FALSE);
        gtk_widget_set_sensitive(prev_event_box, FALSE);
        gtk_widget_set_sensitive(next_event_box, FALSE);
        gtk_widget_set_sensitive(stop_event_box, FALSE);

        if (GTK_WIDGET(gtk_widget_get_parent(GTK_WIDGET(menuitem_play))) == GTK_WIDGET(popup_menu))
            gtk_container_remove(GTK_CONTAINER(popup_menu), GTK_WIDGET(menuitem_play));
        menuitem_play = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-media-play", NULL));
        g_signal_connect(G_OBJECT(menuitem_play), "activate", G_CALLBACK(menuitem_play_callback), NULL);
        gtk_menu_shell_insert(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_play), 0);
        gtk_widget_show(GTK_WIDGET(menuitem_play));
        gmtk_media_tracker_set_text(GMTK_MEDIA_TRACKER(tracker), _("Paused"));
        break;
    default:
        break;
    }

}

void player_error_message_callback(GmtkMediaPlayer * media, gchar * message)
{
    GtkWidget *dialog;

    // log messages to the screen
    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE, "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), g_dgettext(GETTEXT_PACKAGE, "G-MPlayer Error"));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void player_cache_percent_changed_callback(GmtkMediaTracker * tracker, gdouble percentage)
{
    gchar *text;
    if (GTK_IS_WIDGET(tracker)) {
        if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_PLAY) {
            gmtk_media_tracker_set_cache_percentage(tracker, percentage);
            text = g_strdup_printf("Caching %2i%% \342\226\274", (int) (percentage * 100));
            gmtk_media_tracker_set_text(GMTK_MEDIA_TRACKER(tracker), text);
            g_free(text);
        } else {
            gmtk_media_tracker_set_cache_percentage(tracker, 0.0);
        }
    }
}

void player_position_changed_callback(GmtkMediaTracker * tracker, gdouble position)
{
    gchar *text;
    if (GTK_IS_WIDGET(tracker)) {
        gmtk_media_tracker_set_percentage(tracker,
                                          (position -
                                           gmtk_media_player_get_attribute_double
                                           (GMTK_MEDIA_PLAYER(media),
                                            ATTRIBUTE_START_TIME)) /
                                          gmtk_media_player_get_attribute_double
                                          (GMTK_MEDIA_PLAYER(media), ATTRIBUTE_LENGTH));
        gmtk_media_tracker_set_position(tracker,
                                        position -
                                        gmtk_media_player_get_attribute_double
                                        (GMTK_MEDIA_PLAYER(media), ATTRIBUTE_START_TIME));
        if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PLAY) {
            text = g_strdup_printf(_("Playing"));
        } else if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
            text = g_strdup_printf(_("Paused"));
        } else {
            text = g_strdup_printf(_("Idle"));
        }
        gmtk_media_tracker_set_text(GMTK_MEDIA_TRACKER(tracker), text);
        g_free(text);
        update_status_icon();
        g_idle_add(make_panel_and_mouse_invisible, NULL);
    }
}

gboolean tracker_value_changed_callback(GtkWidget * widget, gint value, gpointer data)
{
    if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SEEKABLE)) {
        if (!autopause) {
            if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP) {
                gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), value * 1.0, SEEK_PERCENT);
            }
        }
    }
    return FALSE;
}

gboolean tracker_difference_callback(GtkWidget * widget, gdouble difference, void *data)
{

    if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SEEKABLE)) {
        if (!autopause) {
            if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP) {
                gmtk_media_player_seek(GMTK_MEDIA_PLAYER(media), difference, SEEK_RELATIVE);
            }
        }
    }

    return FALSE;
}

gboolean progress_callback(GtkWidget * widget, GdkEventButton * event, void *data)
{
    GdkEventButton *event_button;
    if (event->type == GDK_BUTTON_PRESS) {
        event_button = (GdkEventButton *) event;
        if (event_button->button == 3) {
            gtk_menu_popup(popup_menu, NULL, NULL, NULL, NULL, event_button->button, event_button->time);
            return TRUE;
        }
    }

    return TRUE;
}

gboolean load_href_callback(GtkWidget * widget, GdkEventExpose * event, gchar * hrefid)
{
    GdkEventButton *event_button;
    if (event->type == GDK_BUTTON_PRESS) {
        event_button = (GdkEventButton *) event;
        if (event_button->button == 3) {
            gtk_menu_popup(popup_menu, NULL, NULL, NULL, NULL, event_button->button, event_button->time);
            return TRUE;
        }

        if (event_button->button != 3) {
            // do this in the plugin when we should
            // gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_showcontrols), TRUE);
            return TRUE;
        }
    }

    return FALSE;
}

gboolean idle_make_button(gpointer data)
{
    ButtonDef *b = (ButtonDef *) data;
    if (b != NULL) {
        make_button(b->uri, b->hrefid);
        g_free(b->uri);
        g_free(b->hrefid);
        g_free(b);
    }

    return FALSE;
}

void make_button(gchar * src, gchar * hrefid)
{
    GError *error;
    gchar *dirname = NULL;
    gchar *filename = NULL;
    gchar *basepath = NULL;
    gint exit_status;
    gchar *out = NULL;
    gchar *err = NULL;
    gchar *av[255];
    gint ac = 0;
    idledata->showcontrols = FALSE;
    showcontrols = FALSE;
    set_show_controls(idledata);
    error = NULL;
    // only try if src ne NULL
    if (src != NULL) {
        pb_button = gdk_pixbuf_new_from_file(src, &error);
    } else {
        return;
    }

    // if we can't directly load the file into a pixbuf it might be a media file
    if (error != NULL) {
        g_error_free(error);
        error = NULL;
        basepath = g_strdup_printf("%s/g-mplayer/plugin", g_get_user_cache_dir());
        dirname = gm_tempname(basepath, "g-mplayerXXXXXX");
        filename = g_strdup_printf("%s/00000001.jpg", dirname);
        g_free(basepath);
        // run mplayer and try to get the first frame and convert it to a jpeg
        av[ac++] = g_strdup_printf("mplayer");
        av[ac++] = g_strdup_printf("-vo");
        av[ac++] = g_strdup_printf("jpeg:outdir=%s", dirname);
        av[ac++] = g_strdup_printf("-ao");
        av[ac++] = g_strdup_printf("null");
        av[ac++] = g_strdup_printf("-x");
        av[ac++] = g_strdup_printf("%i", window_x);
        av[ac++] = g_strdup_printf("-y");
        av[ac++] = g_strdup_printf("%i", window_y);
        av[ac++] = g_strdup_printf("-frames");
        av[ac++] = g_strdup_printf("1");
        av[ac++] = g_strdup_printf("%s", src);
        av[ac] = NULL;
        g_spawn_sync(NULL, av, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &out, &err, &exit_status, &error);
        if (error != NULL) {
            gm_log(verbose, G_LOG_LEVEL_MESSAGE, "Error when running: %s", error->message);
            g_error_free(error);
            error = NULL;
        }

        if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
            pb_button = gdk_pixbuf_new_from_file(filename, &error);
            if (error != NULL) {
                gm_log(verbose, G_LOG_LEVEL_MESSAGE, "make_button error: %s", error->message);
                g_error_free(error);
                error = NULL;
            }
        }

        if (err != NULL)
            g_free(err);
        if (out != NULL)
            g_free(out);
    }

    if (pb_button != NULL && GDK_IS_PIXBUF(pb_button)) {
        button_event_box = gtk_event_box_new();
        image_button = gtk_image_new_from_pixbuf(pb_button);
        gtk_container_add(GTK_CONTAINER(button_event_box), image_button);
        gtk_box_pack_start(GTK_BOX(vbox), button_event_box, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(button_event_box), "button_press_event",
                         G_CALLBACK(load_href_callback), g_strdup(hrefid));
        gtk_widget_set_size_request(GTK_WIDGET(button_event_box), window_x, window_y);
        gtk_widget_show_all(button_event_box);
        gtk_widget_set_size_request(controls_box, 0, 0);
        gtk_widget_hide(controls_box);
        gtk_widget_show(vbox);
    } else {
        gm_log(verbose, G_LOG_LEVEL_INFO, "unable to make button from media, using default");
        button_event_box = gtk_event_box_new();
        image_button = gtk_image_new_from_pixbuf(pb_icon);
        gtk_container_add(GTK_CONTAINER(button_event_box), image_button);
        gtk_box_pack_start(GTK_BOX(vbox), button_event_box, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(button_event_box), "button_press_event",
                         G_CALLBACK(load_href_callback), g_strdup(hrefid));
        gtk_widget_show_all(button_event_box);
        gtk_widget_set_size_request(controls_box, 0, 0);
        gtk_widget_hide(controls_box);
        gtk_widget_show(vbox);
    }

    if (filename != NULL) {
        if (g_file_test(filename, G_FILE_TEST_EXISTS))
            g_remove(filename);
        g_free(filename);
    }

    if (dirname != NULL) {
        g_remove(dirname);
        g_free(dirname);
    }

}

gboolean get_key_and_modifier(gchar * keyval, guint * key, GdkModifierType * modifier)
{
    gchar **parse;

    if (keyval == NULL) {
        *key = 0;
        *modifier = 0;
        return FALSE;
    }
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "keyval = %s", keyval);
    parse = g_strsplit(keyval, "+", -1);
    if (g_strv_length(parse) == 1) {
        *modifier = 0;
        *key = gdk_keyval_from_name(parse[0]);
    } else {
        *modifier = (guint) g_strtod(parse[0], NULL);
        *key = gdk_keyval_from_name(parse[1]);
    }
    g_strfreev(parse);
    gm_log(verbose, G_LOG_LEVEL_DEBUG, "key = %i, modifier = %i", *key, *modifier);
    return TRUE;
}

gint get_index_from_key_and_modifier(guint key, GdkModifierType modifier)
{
    gint i;
    gint index = -1;
    gchar *keyval = NULL;

    modifier = modifier & (GDK_SHIFT_MASK | GDK_MOD1_MASK | GDK_CONTROL_MASK);
    if (modifier == 0) {
        keyval = g_strdup_printf("%s", gdk_keyval_name(key));
    } else {
        keyval = g_strdup_printf("%i+%s", modifier, gdk_keyval_name(key));
    }

    for (i = 0; i < KEY_COUNT; i++) {
        gm_log(verbose, G_LOG_LEVEL_DEBUG, "%s ?= %s", accel_keys[i], keyval);
        if (g_ascii_strcasecmp(accel_keys[i], keyval) == 0) {
            index = i;
            break;
        }
    }

    g_free(keyval);

    return index;
}

void setup_accelerators()
{
    guint key;
    GdkModifierType modifier;

    if (get_key_and_modifier(accel_keys[FILE_OPEN_LOCATION], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_OPEN_LOCATION, key, modifier, TRUE);

    if (get_key_and_modifier(accel_keys[EDIT_SCREENSHOT], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_EDIT_SCREENSHOT, key, modifier, TRUE);

    if (get_key_and_modifier(accel_keys[EDIT_PREFERENCES], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_EDIT_PREFERENCES, key, modifier, TRUE);

    if (get_key_and_modifier(accel_keys[VIEW_PLAYLIST], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_VIEW_PLAYLIST, key, modifier, TRUE);
    if (get_key_and_modifier(accel_keys[VIEW_INFO], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_VIEW_INFO, key, modifier, TRUE);
    if (get_key_and_modifier(accel_keys[VIEW_DETAILS], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_VIEW_DETAILS, key, modifier, TRUE);
    if (get_key_and_modifier(accel_keys[VIEW_FULLSCREEN], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_VIEW_FULLSCREEN, key, modifier, TRUE);
    if (get_key_and_modifier(accel_keys[VIEW_ASPECT], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_VIEW_ASPECT, key, modifier, TRUE);
    if (get_key_and_modifier(accel_keys[VIEW_SUBTITLES], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_VIEW_SUBTITLES, key, modifier, TRUE);
    if (get_key_and_modifier(accel_keys[VIEW_DECREASE_SIZE], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_VIEW_DECREASE_SIZE, key, modifier, TRUE);
    if (get_key_and_modifier(accel_keys[VIEW_INCREASE_SIZE], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_VIEW_INCREASE_SIZE, key, modifier, TRUE);
    if (get_key_and_modifier(accel_keys[VIEW_ANGLE], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_VIEW_ANGLE, key, modifier, TRUE);
    if (get_key_and_modifier(accel_keys[VIEW_CONTROLS], &key, &modifier))
        gtk_accel_map_change_entry(ACCEL_PATH_VIEW_CONTROLS, key, modifier, TRUE);

    gtk_accel_map_change_entry(ACCEL_PATH_VIEW_NORMAL, GDK_1, GDK_CONTROL_MASK, TRUE);
    gtk_accel_map_change_entry(ACCEL_PATH_VIEW_DOUBLE, GDK_2, GDK_CONTROL_MASK, TRUE);
}

GtkWidget *create_window()
{
    GtkIconTheme *icon_theme;
    GtkTargetEntry target_entry[3];
    gint i = 0;
    GtkRecentFilter *recent_filter;
    GtkAdjustment *adj;
#ifdef GTK3_ENABLED
    const char *icon_start, *icon_seek_forward, *icon_seek_backward, *icon_skip_forward, *icon_skip_backward;

    if (gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL) {
        icon_start = "media-playback-start-rtl-symbolic";
        icon_seek_forward = "media-seek-forward-rtl-symbolic";
        icon_seek_backward = "media-seek-backward-rtl-symbolic";
        icon_skip_forward = "media-skip-forward-rtl-symbolic";
        icon_skip_backward = "media-skip-backward-rtl-symbolic";
    } else {
        icon_start = "media-playback-start-symbolic";
        icon_seek_forward = "media-seek-forward-symbolic";
        icon_seek_backward = "media-seek-backward-symbolic";
        icon_skip_forward = "media-skip-forward-symbolic";
        icon_skip_backward = "media-skip-backward-symbolic";
    }
#endif
    in_button = FALSE;
    last_movement_time = -1;
    fs_window = NULL;

    Wg_mutex_init(&fs_controls_lock);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), _("Media Player"));
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events(window, GDK_BUTTON_RELEASE_MASK);
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
    gtk_widget_add_events(window, GDK_KEY_RELEASE_MASK);
    gtk_widget_add_events(window, GDK_ENTER_NOTIFY_MASK);
    gtk_widget_add_events(window, GDK_LEAVE_NOTIFY_MASK);
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
    gtk_widget_add_events(window, GDK_VISIBILITY_NOTIFY_MASK);
    gtk_widget_add_events(window, GDK_STRUCTURE_MASK);
    gtk_widget_add_events(window, GDK_POINTER_MOTION_MASK);
    delete_signal_id = g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_callback), NULL);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_callback), NULL);
    g_signal_connect(G_OBJECT(window), "motion_notify_event", G_CALLBACK(motion_notify_callback), NULL);
    g_signal_connect(G_OBJECT(window), "window_state_event", G_CALLBACK(window_state_callback), NULL);
    g_signal_connect(G_OBJECT(window), "configure_event", G_CALLBACK(configure_callback), NULL);
    accel_group = gtk_accel_group_new();

    // right click menu
    popup_menu = GTK_MENU(gtk_menu_new());
    menubar = gtk_menu_bar_new();
    menuitem_play = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-media-play", NULL));
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_play));
    gtk_widget_show(GTK_WIDGET(menuitem_play));
    menuitem_stop = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-media-stop", NULL));
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_stop));
    gtk_widget_show(GTK_WIDGET(menuitem_stop));
    menuitem_prev = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-media-previous", NULL));
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_prev));
    menuitem_next = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-media-next", NULL));
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_next));
    menuitem_sep1 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_sep1));
    gtk_widget_show(GTK_WIDGET(menuitem_sep1));
    menuitem_open = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-open", accel_group));
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_open));
    gtk_widget_show(GTK_WIDGET(menuitem_open));
    menuitem_sep3 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_sep3));
    gtk_widget_show(GTK_WIDGET(menuitem_sep3));
    menuitem_showcontrols = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("S_how Controls")));
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_showcontrols));
    gtk_widget_show(GTK_WIDGET(menuitem_showcontrols));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_showcontrols), TRUE);
    menuitem_fullscreen = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("_Full Screen")));
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_fullscreen));
    gtk_widget_show(GTK_WIDGET(menuitem_fullscreen));

    menuitem_copyurl = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("_Copy Location")));
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_copyurl));
    gtk_widget_show(GTK_WIDGET(menuitem_copyurl));
    menuitem_sep2 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_sep2));
    gtk_widget_show(GTK_WIDGET(menuitem_sep2)); // control_id
    menuitem_config = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-preferences", NULL));
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_config));
    gtk_widget_show(GTK_WIDGET(menuitem_config)); // control_id

    menuitem_sep3 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_sep3));
    gtk_widget_show(GTK_WIDGET(menuitem_sep3)); // control_id
    menuitem_quit = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-quit", accel_group));
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), GTK_WIDGET(menuitem_quit));
    gtk_widget_show(GTK_WIDGET(menuitem_quit)); // control_id

    g_signal_connect(G_OBJECT(menuitem_open), "activate", G_CALLBACK(menuitem_open_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_play), "activate", G_CALLBACK(menuitem_pause_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_stop), "activate", G_CALLBACK(menuitem_stop_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_prev), "activate", G_CALLBACK(menuitem_prev_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_next), "activate", G_CALLBACK(menuitem_next_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_showcontrols), "toggled", G_CALLBACK(menuitem_showcontrols_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_fullscreen), "toggled", G_CALLBACK(menuitem_fs_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_copyurl), "activate", G_CALLBACK(menuitem_copyurl_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_config), "activate", G_CALLBACK(menuitem_config_dialog_cb), NULL);
    g_signal_connect(G_OBJECT(menuitem_quit), "activate", G_CALLBACK(menuitem_quit_callback), NULL);
    g_signal_connect_swapped(G_OBJECT(window), "button_release_event", G_CALLBACK(popup_handler), G_OBJECT(popup_menu));
    g_signal_connect_swapped(G_OBJECT(window), "enter_notify_event", G_CALLBACK(notification_handler), NULL);
    g_signal_connect_swapped(G_OBJECT(window), "leave_notify_event", G_CALLBACK(notification_handler), NULL);
    // File Menu
    menuitem_file = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("_File")));
    menu_file = GTK_MENU(gtk_menu_new());
    gtk_menu_set_accel_group(GTK_MENU(menu_file), accel_group);
    gtk_widget_show(GTK_WIDGET(menuitem_file));
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), GTK_WIDGET(menuitem_file));
    gtk_menu_item_set_submenu(menuitem_file, GTK_WIDGET(menu_file));
    menuitem_file_open = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-open", accel_group));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file), GTK_WIDGET(menuitem_file_open));
    menuitem_file_open_folder = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Open _Folder")));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM
                                  (menuitem_file_open_folder),
                                  gtk_image_new_from_icon_name("folder", GTK_ICON_SIZE_MENU));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file), GTK_WIDGET(menuitem_file_open_folder));
    menuitem_file_open_location = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Open _Location")));
    gtk_menu_item_set_accel_path(menuitem_file_open_location, ACCEL_PATH_OPEN_LOCATION);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM
                                  (menuitem_file_open_location),
                                  gtk_image_new_from_icon_name("network-server", GTK_ICON_SIZE_MENU));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file), GTK_WIDGET(menuitem_file_open_location));
    menuitem_file_disc = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("_Disc")));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM
                                  (menuitem_file_disc),
                                  gtk_image_new_from_icon_name("media-optical", GTK_ICON_SIZE_MENU));
    menu_file_disc = GTK_MENU(gtk_menu_new());
    gtk_widget_show(GTK_WIDGET(menuitem_file_disc));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file), GTK_WIDGET(menuitem_file_disc));
    gtk_menu_item_set_submenu(menuitem_file_disc, GTK_WIDGET(menu_file_disc));
    menuitem_file_open_acd = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Open _Audio CD")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file_disc), GTK_WIDGET(menuitem_file_open_acd));
    menuitem_file_open_sep1 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file_disc), GTK_WIDGET(menuitem_file_open_sep1));
    menuitem_file_open_dvdnav = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Open DVD with _Menus")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file_disc), GTK_WIDGET(menuitem_file_open_dvdnav));
    menuitem_file_open_dvdnav_folder =
        GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Open DVD from Folder with M_enus")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file_disc), GTK_WIDGET(menuitem_file_open_dvdnav_folder));
    menuitem_file_open_dvdnav_iso =
        GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Open DVD from ISO with Me_nus")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file_disc), GTK_WIDGET(menuitem_file_open_dvdnav_iso));
    menuitem_file_open_sep2 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file_disc), GTK_WIDGET(menuitem_file_open_sep2));
    menuitem_file_open_vcd = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Open _VCD")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file_disc), GTK_WIDGET(menuitem_file_open_vcd));
    menuitem_file_tv = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("_TV")));
    menu_file_tv = GTK_MENU(gtk_menu_new());
    gtk_widget_show(GTK_WIDGET(menuitem_file_tv));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file), GTK_WIDGET(menuitem_file_tv));
    gtk_menu_item_set_submenu(menuitem_file_tv, GTK_WIDGET(menu_file_tv));
    menuitem_file_open_atv = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Open _Analog TV")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file_tv), GTK_WIDGET(menuitem_file_open_atv));
    menuitem_file_open_dtv = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Open _Digital TV")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file_tv), GTK_WIDGET(menuitem_file_open_dtv));

    recent_manager = gtk_recent_manager_get_default();
    menuitem_file_recent = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Open _Recent")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file), GTK_WIDGET(menuitem_file_recent));
    menuitem_file_recent_items = gtk_recent_chooser_menu_new();
    recent_filter = gtk_recent_filter_new();
    gtk_recent_filter_add_application(recent_filter, "g-mplayer");
    gtk_recent_chooser_add_filter(GTK_RECENT_CHOOSER(menuitem_file_recent_items), recent_filter);
    gtk_recent_chooser_set_show_tips(GTK_RECENT_CHOOSER(menuitem_file_recent_items), TRUE);
    gtk_recent_chooser_set_sort_type(GTK_RECENT_CHOOSER(menuitem_file_recent_items), GTK_RECENT_SORT_MRU);
    gtk_menu_item_set_submenu(menuitem_file_recent, menuitem_file_recent_items);
    gtk_recent_chooser_set_local_only(GTK_RECENT_CHOOSER(menuitem_file_recent_items), FALSE);

    menuitem_file_sep2 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file), GTK_WIDGET(menuitem_file_sep2));
    menuitem_file_quit = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-quit", accel_group));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_file), GTK_WIDGET(menuitem_file_quit));
    g_signal_connect(G_OBJECT(menuitem_file_open), "activate", G_CALLBACK(menuitem_open_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_file_open_folder), "activate",
                     G_CALLBACK(add_folder_to_playlist), menuitem_file_open_folder);
    g_signal_connect(G_OBJECT(menuitem_file_open_location), "activate",
                     G_CALLBACK(menuitem_open_location_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_file_open_dvdnav), "activate", G_CALLBACK(menuitem_open_dvdnav_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_file_open_dvdnav_folder),
                     "activate", G_CALLBACK(menuitem_open_dvdnav_folder_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_file_open_dvdnav_iso),
                     "activate", G_CALLBACK(menuitem_open_dvdnav_iso_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_file_open_acd), "activate", G_CALLBACK(menuitem_open_acd_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_file_open_vcd), "activate", G_CALLBACK(menuitem_open_vcd_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_file_open_atv), "activate", G_CALLBACK(menuitem_open_atv_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_file_open_dtv), "activate", G_CALLBACK(menuitem_open_dtv_callback), NULL);

    g_signal_connect(G_OBJECT(menuitem_file_recent_items),
                     "item-activated", G_CALLBACK(menuitem_open_recent_callback), NULL);

    g_signal_connect(G_OBJECT(menuitem_file_quit), "activate", G_CALLBACK(menuitem_quit_callback), NULL);
    // Edit Menu
    menuitem_edit = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("_Edit")));
    menu_edit = GTK_MENU(gtk_menu_new());
    gtk_menu_set_accel_group(GTK_MENU(menu_edit), accel_group);
    gtk_widget_show(GTK_WIDGET(menuitem_edit));
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), GTK_WIDGET(menuitem_edit));
    gtk_menu_item_set_submenu(menuitem_edit, GTK_WIDGET(menu_edit));
    menuitem_edit_random = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("_Shuffle Playlist")));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_edit_random), random_order);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_random));
    menuitem_edit_loop = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("_Loop Playlist")));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_edit_loop), loop);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_loop));
    menuitem_edit_play_single =
        GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("P_lay Single Track from Playlist")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_play_single));
    menuitem_view_sep0 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_view_sep0));
    menuitem_edit_switch_audio = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("S_witch Audio Track")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_switch_audio));
    menuitem_edit_set_audiofile = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Set Audi_o")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_set_audiofile));
    menuitem_edit_select_audio_lang = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Select _Audio Language")));
    gtk_widget_show(GTK_WIDGET(menuitem_edit_select_audio_lang));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_select_audio_lang));
    menuitem_edit_set_subtitle = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Set S_ubtitle")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_set_subtitle));
    menuitem_edit_select_sub_lang = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("S_elect Subtitle Language")));
    gtk_widget_show(GTK_WIDGET(menuitem_edit_select_sub_lang));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_select_sub_lang));
    menuitem_view_sep0 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_view_sep0));
    menuitem_edit_take_screenshot = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("_Take Screenshot")));
    gtk_menu_item_set_accel_path(menuitem_edit_take_screenshot, ACCEL_PATH_EDIT_SCREENSHOT);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_take_screenshot));

    gtk_widget_set_tooltip_text(GTK_WIDGET
                                (menuitem_edit_take_screenshot),
                                _("Files named ’shotNNNN.png’ will be saved in the working directory"));
    menuitem_edit_sep1 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_sep1));
    menuitem_edit_config = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-preferences", NULL));
    gtk_menu_item_set_accel_path(menuitem_edit_config, ACCEL_PATH_EDIT_PREFERENCES);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_edit), GTK_WIDGET(menuitem_edit_config));
    g_signal_connect(G_OBJECT(menuitem_edit_random), "activate", G_CALLBACK(menuitem_edit_random_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_edit_loop), "activate", G_CALLBACK(menuitem_edit_loop_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_edit_set_audiofile), "activate",
                     G_CALLBACK(menuitem_edit_set_audiofile_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_edit_switch_audio), "activate",
                     G_CALLBACK(menuitem_edit_switch_audio_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_edit_set_subtitle), "activate",
                     G_CALLBACK(menuitem_edit_set_subtitle_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_edit_take_screenshot),
                     "activate", G_CALLBACK(menuitem_edit_take_screenshot_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_edit_config), "activate", G_CALLBACK(menuitem_config_dialog_cb), NULL);

    // View Menu
    menuitem_view = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("_View")));
    menu_view = GTK_MENU(gtk_menu_new());
    gtk_menu_set_accel_group(GTK_MENU(menu_view), accel_group);
    gtk_widget_show(GTK_WIDGET(menuitem_view));
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), GTK_WIDGET(menuitem_view));
    gtk_menu_item_set_submenu(menuitem_view, GTK_WIDGET(menu_view));
    menuitem_view_playlist = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("_Playlist")));
    gtk_menu_item_set_accel_path(menuitem_view_playlist, ACCEL_PATH_VIEW_PLAYLIST);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_playlist));
    menuitem_view_info = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("Media _Info")));
    gtk_menu_item_set_accel_path(menuitem_view_info, ACCEL_PATH_VIEW_INFO);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_info));
    menuitem_view_details = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("D_etails")));
    gtk_menu_item_set_accel_path(menuitem_view_details, ACCEL_PATH_VIEW_DETAILS);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_details));
    menuitem_view_fullscreen = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("_Full Screen")));
    gtk_menu_item_set_accel_path(menuitem_view_fullscreen, ACCEL_PATH_VIEW_FULLSCREEN);
    menuitem_view_sep1 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_fullscreen));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_sep1));

    menuitem_view_onetoone = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("_Normal (1:1)")));
    gtk_menu_item_set_accel_path(menuitem_view_onetoone, ACCEL_PATH_VIEW_NORMAL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_onetoone));
    menuitem_view_twotoone = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("_Double Size (2:1)")));
    gtk_menu_item_set_accel_path(menuitem_view_twotoone, ACCEL_PATH_VIEW_DOUBLE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_twotoone));
    menuitem_view_onetotwo = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("_Half Size (1:2)")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_onetotwo));
    menuitem_view_onetoonepointfive = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Half _Larger (1.5:1)")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_onetoonepointfive));
    menuitem_view_aspect = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("_Aspect")));
    gtk_menu_item_set_accel_path(menuitem_view_aspect, ACCEL_PATH_VIEW_ASPECT);
    menu_view_aspect = GTK_MENU(gtk_menu_new());
    gtk_widget_show(GTK_WIDGET(menuitem_view_aspect));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_aspect));
    gtk_menu_item_set_submenu(menuitem_view_aspect, GTK_WIDGET(menu_view_aspect));
    menuitem_view_aspect_default = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("D_efault Aspect")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view_aspect), GTK_WIDGET(menuitem_view_aspect_default));
    menuitem_view_aspect_four_three = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("_4:3 Aspect")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view_aspect), GTK_WIDGET(menuitem_view_aspect_four_three));
    menuitem_view_aspect_sixteen_nine = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("_16:9 Aspect")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view_aspect), GTK_WIDGET(menuitem_view_aspect_sixteen_nine));
    menuitem_view_aspect_sixteen_ten = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("1_6:10 Aspect")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view_aspect), GTK_WIDGET(menuitem_view_aspect_sixteen_ten));
    menuitem_view_aspect_anamorphic =
        GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("_2.39:1 Aspect (Anamorphic)")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view_aspect), GTK_WIDGET(menuitem_view_aspect_anamorphic));
    menuitem_view_aspect_follow_window = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("_Follow Window")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view_aspect), GTK_WIDGET(menuitem_view_aspect_follow_window));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_aspect_default), TRUE);
    menuitem_view_sep2 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    menuitem_view_sep5 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    menuitem_view_subtitles = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("Show _Subtitles")));
    gtk_menu_item_set_accel_path(menuitem_view_subtitles, ACCEL_PATH_VIEW_SUBTITLES);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_subtitles), showsubtitles);
    menuitem_view_smaller_subtitle = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Decrease Subtitle Size")));
    gtk_menu_item_set_accel_path(menuitem_view_smaller_subtitle, ACCEL_PATH_VIEW_DECREASE_SIZE);
    menuitem_view_larger_subtitle = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Increase Subtitle Size")));
    gtk_menu_item_set_accel_path(menuitem_view_larger_subtitle, ACCEL_PATH_VIEW_INCREASE_SIZE);
    menuitem_view_decrease_subtitle_delay =
        GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Decrease Subtitle Delay")));
    menuitem_view_increase_subtitle_delay =
        GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Increase Subtitle Delay")));
    menuitem_view_angle = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("Switch An_gle")));
    gtk_menu_item_set_accel_path(menuitem_view_angle, ACCEL_PATH_VIEW_ANGLE);
    menuitem_view_controls = GTK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(_("_Controls")));
    gtk_menu_item_set_accel_path(menuitem_view_controls, ACCEL_PATH_VIEW_CONTROLS);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_controls), TRUE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_sep2));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_subtitles));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_smaller_subtitle));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_larger_subtitle));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_decrease_subtitle_delay));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_increase_subtitle_delay));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_sep5));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_angle));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_controls));
    menuitem_view_sep3 = GTK_MENU_ITEM(gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_sep3));
    menuitem_view_advanced = GTK_MENU_ITEM(gtk_image_menu_item_new_with_mnemonic(_("_Video Picture Adjustments")));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_view), GTK_WIDGET(menuitem_view_advanced));
    g_signal_connect(G_OBJECT(menuitem_view_playlist), "toggled", G_CALLBACK(menuitem_view_playlist_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_info), "activate", G_CALLBACK(menuitem_view_info_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_details), "activate", G_CALLBACK(menuitem_details_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_fullscreen), "toggled", G_CALLBACK(menuitem_fs_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_onetoone), "activate",
                     G_CALLBACK(menuitem_view_onetoone_callback), idledata);
    g_signal_connect(G_OBJECT(menuitem_view_twotoone), "activate",
                     G_CALLBACK(menuitem_view_twotoone_callback), idledata);
    g_signal_connect(G_OBJECT(menuitem_view_onetotwo), "activate",
                     G_CALLBACK(menuitem_view_onetotwo_callback), idledata);
    g_signal_connect(G_OBJECT(menuitem_view_onetoonepointfive), "activate",
                     G_CALLBACK(menuitem_view_onetoonepointfive_callback), idledata);
    g_signal_connect(G_OBJECT(menuitem_view_aspect_default),
                     "activate", G_CALLBACK(menuitem_view_aspect_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_aspect_four_three),
                     "activate", G_CALLBACK(menuitem_view_aspect_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_aspect_sixteen_nine),
                     "activate", G_CALLBACK(menuitem_view_aspect_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_aspect_sixteen_ten),
                     "activate", G_CALLBACK(menuitem_view_aspect_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_aspect_anamorphic),
                     "activate", G_CALLBACK(menuitem_view_aspect_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_aspect_follow_window),
                     "activate", G_CALLBACK(menuitem_view_aspect_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_subtitles), "toggled", G_CALLBACK(menuitem_view_subtitles_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_smaller_subtitle),
                     "activate", G_CALLBACK(menuitem_view_smaller_subtitle_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_larger_subtitle),
                     "activate", G_CALLBACK(menuitem_view_larger_subtitle_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_decrease_subtitle_delay),
                     "activate", G_CALLBACK(menuitem_view_decrease_subtitle_delay_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_increase_subtitle_delay),
                     "activate", G_CALLBACK(menuitem_view_increase_subtitle_delay_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_angle), "activate", G_CALLBACK(menuitem_view_angle_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_controls), "toggled", G_CALLBACK(menuitem_showcontrols_callback), NULL);
    g_signal_connect(G_OBJECT(menuitem_view_advanced), "activate", G_CALLBACK(menuitem_video_adjustments_dlg), idledata);
    // Help Menu
    menuitem_help = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("_Help")));
    menu_help = GTK_MENU(gtk_menu_new());
    gtk_widget_show(GTK_WIDGET(menuitem_help));
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), GTK_WIDGET(menuitem_help));
    gtk_menu_item_set_submenu(menuitem_help, GTK_WIDGET(menu_help));
    menuitem_help_about = GTK_MENU_ITEM(gtk_image_menu_item_new_from_stock("gtk-about", NULL));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_help), GTK_WIDGET(menuitem_help_about));
    g_signal_connect(G_OBJECT(menuitem_help_about), "activate", G_CALLBACK(menuitem_about_callback), NULL);
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
    setup_accelerators();
    g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(window_key_callback), NULL);
    // Give the window the property to accept DnD
    target_entry[i].target = DRAG_NAME_0;
    target_entry[i].flags = 0;
    target_entry[i++].info = DRAG_INFO_0;
    target_entry[i].target = DRAG_NAME_1;
    target_entry[i].flags = 0;
    target_entry[i++].info = DRAG_INFO_1;
    target_entry[i].target = DRAG_NAME_2;
    target_entry[i].flags = 0;
    target_entry[i++].info = DRAG_INFO_2;
    gtk_drag_dest_set(window,
                      GTK_DEST_DEFAULT_MOTION |
                      GTK_DEST_DEFAULT_HIGHLIGHT | GTK_DEST_DEFAULT_DROP, target_entry, i, GDK_ACTION_LINK);
    gtk_drag_dest_add_uri_targets(window);
    //Connect the signal for DnD
    g_signal_connect(G_OBJECT(window), "drag_data_received", G_CALLBACK(drop_callback), NULL);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    controls_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    media = gmtk_media_player_new();
    g_signal_connect_swapped(G_OBJECT(media), "media_state_changed",
                             G_CALLBACK(player_media_state_changed_callback), NULL);
    g_signal_connect_swapped(G_OBJECT(media), "button_press_event", G_CALLBACK(popup_handler), G_OBJECT(popup_menu));
    g_signal_connect(G_OBJECT(media), "audio-tracks-changed",
                     G_CALLBACK(player_audio_track_callback), menuitem_edit_select_audio_lang);
    g_signal_connect(G_OBJECT(media), "subtitles-changed",
                     G_CALLBACK(player_subtitle_callback), menuitem_edit_select_sub_lang);
    g_signal_connect(G_OBJECT(media), "size_allocate", G_CALLBACK(player_size_allocate_callback), NULL);
    g_signal_connect(G_OBJECT(media), "scroll_event", G_CALLBACK(media_scroll_event_callback), NULL);
    g_signal_connect(G_OBJECT(media), "error-message", G_CALLBACK(player_error_message_callback), NULL);
    cover_art = gtk_image_new();
    media_label = gtk_label_new("");
    gtk_widget_set_size_request(media_label, 300, 100);
    gtk_label_set_ellipsize(GTK_LABEL(media_label), PANGO_ELLIPSIZE_END);
    media_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    g_signal_connect(media_hbox, "show", G_CALLBACK(view_option_show_callback), NULL);
    g_signal_connect(media_hbox, "size_allocate", G_CALLBACK(view_option_size_allocate_callback), NULL);
    details_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    details_table = gtk_table_new(20, 2, FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (details_table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (details_table), 6);
    g_signal_connect(details_vbox, "show", G_CALLBACK(view_option_show_callback), NULL);
    g_signal_connect(details_vbox, "size_allocate", G_CALLBACK(view_option_size_allocate_callback), NULL);
    create_details_table();
    gtk_container_add(GTK_CONTAINER(details_vbox), details_table);

    gtk_widget_set_halign (media_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), media, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(media_hbox), cover_art, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(media_hbox), media_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), media_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), details_vbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(controls_box), hbox, FALSE, FALSE, 1);
    if (vertical_layout) {
        pane = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
    } else {
        pane = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    }
    gtk_paned_pack1(GTK_PANED(pane), vbox, FALSE, TRUE);
    create_playlist_widget();
    g_signal_connect(plvbox, "show", G_CALLBACK(view_option_show_callback), NULL);
    g_signal_connect(plvbox, "hide", G_CALLBACK(view_option_hide_callback), NULL);
    g_signal_connect(plvbox, "size_allocate", G_CALLBACK(view_option_size_allocate_callback), NULL);

    vbox_master = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox_master), menubar, FALSE, FALSE, 0);
    gtk_widget_show(menubar);
    gtk_box_pack_start(GTK_BOX(vbox_master), pane, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_master), controls_box, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox_master);
    icon_theme = gtk_icon_theme_get_default();
#ifdef GTK3_ENABLED
    if (gtk_icon_theme_has_icon(icon_theme, icon_start) &&
        gtk_icon_theme_has_icon(icon_theme, "media-playback-stop-symbolic") &&
        gtk_icon_theme_has_icon(icon_theme, "media-playback-pause-symbolic") &&
        gtk_icon_theme_has_icon(icon_theme, icon_seek_forward) &&
        gtk_icon_theme_has_icon(icon_theme, icon_seek_backward) &&
        gtk_icon_theme_has_icon(icon_theme, "view-sidebar-symbolic") &&
        gtk_icon_theme_has_icon(icon_theme, "view-fullscreen-symbolic")) {
        image_play = gtk_image_new_from_icon_name(icon_start, button_size);
        image_stop = gtk_image_new_from_icon_name("media-playback-stop-symbolic", button_size);
        image_pause = gtk_image_new_from_icon_name("media-playback-pause-symbolic", button_size);
        image_ff = gtk_image_new_from_icon_name(icon_seek_forward, button_size);
        image_rew = gtk_image_new_from_icon_name(icon_seek_backward, button_size);
        image_prev = gtk_image_new_from_icon_name(icon_skip_backward, button_size);
        image_next = gtk_image_new_from_icon_name(icon_skip_forward, button_size);
        image_menu = gtk_image_new_from_icon_name("view-sidebar-symbolic", button_size);
        image_fs = gtk_image_new_from_icon_name("view-fullscreen-symbolic", button_size);
    } else {
        image_play = gtk_image_new_from_stock("gtk-media-play", button_size);
        image_stop = gtk_image_new_from_stock("gtk-media-stop", button_size);
        image_pause = gtk_image_new_from_stock("gtk-media-pause", button_size);
        image_ff = gtk_image_new_from_stock("gtk-media-forward", button_size);
        image_rew = gtk_image_new_from_stock("gtk-media-rewind", button_size);
        image_prev = gtk_image_new_from_stock("gtk-media-previous", button_size);
        image_next = gtk_image_new_from_stock("gtk-media-next", button_size);
        image_menu = gtk_image_new_from_stock("gtk-index", button_size);
        image_fs = gtk_image_new_from_stock("gtk-fullscreen", button_size);
    }
#else
    image_play = gtk_image_new_from_stock("gtk-media-play", button_size);
    image_stop = gtk_image_new_from_stock("gtk-media-stop", button_size);
    image_pause = gtk_image_new_from_stock("gtk-media-pause", button_size);
    image_ff = gtk_image_new_from_stock("gtk-media-forward", button_size);
    image_rew = gtk_image_new_from_stock("gtk-media-rewind", button_size);
    image_prev = gtk_image_new_from_stock("gtk-media-previous", button_size);
    image_next = gtk_image_new_from_stock("gtk-media-next", button_size);
    image_menu = gtk_image_new_from_stock("gtk-index", button_size);
    image_fs = gtk_image_new_from_stock("gtk-fullscreen", button_size);
#endif

    icon_list = NULL;
    if (gtk_icon_theme_has_icon(icon_theme, "g-mplayer")) {
        pb_icon = gtk_icon_theme_load_icon(icon_theme, "g-mplayer", 128, 0, NULL);
        if (pb_icon)
            icon_list = g_list_prepend(icon_list, pb_icon);
        pb_icon = gtk_icon_theme_load_icon(icon_theme, "g-mplayer", 64, 0, NULL);
        if (pb_icon)
            icon_list = g_list_prepend(icon_list, pb_icon);
        pb_icon = gtk_icon_theme_load_icon(icon_theme, "g-mplayer", 48, 0, NULL);
        if (pb_icon)
            icon_list = g_list_prepend(icon_list, pb_icon);
        pb_icon = gtk_icon_theme_load_icon(icon_theme, "g-mplayer", 24, 0, NULL);
        if (pb_icon)
            icon_list = g_list_prepend(icon_list, pb_icon);
        pb_icon = gtk_icon_theme_load_icon(icon_theme, "g-mplayer", 22, 0, NULL);
        if (pb_icon)
            icon_list = g_list_prepend(icon_list, pb_icon);
        pb_icon = gtk_icon_theme_load_icon(icon_theme, "g-mplayer", 16, 0, NULL);
        if (pb_icon)
            icon_list = g_list_prepend(icon_list, pb_icon);
    } else {
        pb_icon = gdk_pixbuf_new_from_xpm_data((const char **) g_mplayer_xpm);
        if (pb_icon)
            icon_list = g_list_prepend(icon_list, pb_icon);
    }
    gtk_window_set_default_icon_list(icon_list);
    gtk_window_set_icon_list(GTK_WINDOW(window), icon_list);

    if (show_status_icon) {
        if (gtk_icon_theme_has_icon(icon_theme, "g-mplayer-panel")) {
            status_icon = gtk_status_icon_new_from_icon_name("g-mplayer-panel");
        } else if (gtk_icon_theme_has_icon(icon_theme, "g-mplayer")) {
            status_icon = gtk_status_icon_new_from_icon_name("g-mplayer");
        } else {
            status_icon = gtk_status_icon_new_from_pixbuf(pb_icon);
        }
        gtk_status_icon_set_visible(status_icon, show_status_icon);
        g_signal_connect(status_icon, "activate", G_CALLBACK(status_icon_callback), NULL);
        g_signal_connect(status_icon, "popup_menu", G_CALLBACK(status_icon_context_callback), NULL);
    }

    menu_event_box = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(menu_event_box), image_menu);
    gtk_button_set_relief(GTK_BUTTON(menu_event_box), GTK_RELIEF_NONE);

    gtk_widget_set_tooltip_text(menu_event_box, _("Menu"));
    gtk_widget_set_can_focus(menu_event_box, FALSE);
    gtk_widget_set_events(menu_event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(menu_event_box), "button_press_event", G_CALLBACK(menu_callback), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), menu_event_box, FALSE, FALSE, 0);
    gtk_widget_show(image_menu);
    gtk_widget_show(menu_event_box);

    prev_event_box = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(prev_event_box), image_prev);
    gtk_button_set_relief(GTK_BUTTON(prev_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(prev_event_box, _("Previous"));
    gtk_widget_set_can_focus(prev_event_box, FALSE);
    gtk_widget_set_events(prev_event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(prev_event_box), "button_press_event", G_CALLBACK(prev_callback), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), prev_event_box, FALSE, FALSE, 0);
    gtk_widget_show(image_prev);
    gtk_widget_show(prev_event_box);
    gtk_widget_set_sensitive(prev_event_box, FALSE);

    rew_event_box = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(rew_event_box), image_rew);
    gtk_button_set_relief(GTK_BUTTON(rew_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(rew_event_box, _("Rewind"));
    gtk_widget_set_can_focus(rew_event_box, FALSE);
    gtk_widget_set_events(rew_event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(rew_event_box), "button_press_event", G_CALLBACK(rew_callback), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), rew_event_box, FALSE, FALSE, 0);
    gtk_widget_show(image_rew);
    gtk_widget_show(rew_event_box);
    gtk_widget_set_sensitive(rew_event_box, FALSE);

    play_event_box = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(play_event_box), image_play);
    gtk_button_set_relief(GTK_BUTTON(play_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(play_event_box, _("Play"));
    gtk_widget_set_can_focus(play_event_box, FALSE);
    gtk_widget_set_events(play_event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(play_event_box), "button_press_event", G_CALLBACK(play_callback), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), play_event_box, FALSE, FALSE, 0);
    gtk_widget_show(image_play);
    gtk_widget_show(play_event_box);

    stop_event_box = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(stop_event_box), image_stop);
    gtk_button_set_relief(GTK_BUTTON(stop_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(stop_event_box, _("Stop"));
    gtk_widget_set_can_focus(stop_event_box, FALSE);
    gtk_widget_set_events(stop_event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(stop_event_box), "button_press_event", G_CALLBACK(stop_callback), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), stop_event_box, FALSE, FALSE, 0);
    gtk_widget_show(image_stop);
    gtk_widget_show(stop_event_box);
    gtk_widget_set_sensitive(stop_event_box, FALSE);

    ff_event_box = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(ff_event_box), image_ff);
    gtk_button_set_relief(GTK_BUTTON(ff_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(ff_event_box, _("Fast Forward"));
    gtk_widget_set_can_focus(ff_event_box, FALSE);
    gtk_widget_set_events(ff_event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(ff_event_box), "button_press_event", G_CALLBACK(ff_callback), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), ff_event_box, FALSE, FALSE, 0);
    gtk_widget_show(image_ff);
    gtk_widget_show(ff_event_box);
    gtk_widget_set_sensitive(ff_event_box, FALSE);

    next_event_box = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(next_event_box), image_next);
    gtk_button_set_relief(GTK_BUTTON(next_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(next_event_box, _("Next"));
    gtk_widget_set_can_focus(next_event_box, FALSE);
    gtk_widget_set_events(next_event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(next_event_box), "button_press_event", G_CALLBACK(next_callback), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), next_event_box, FALSE, FALSE, 0);
    gtk_widget_show(image_next);
    gtk_widget_show(next_event_box);
    gtk_widget_set_sensitive(next_event_box, FALSE);

    // progress bar
    tracker = GMTK_MEDIA_TRACKER(gmtk_media_tracker_new());
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(tracker), TRUE, TRUE, 2);
    g_signal_connect(G_OBJECT(tracker), "difference-changed", G_CALLBACK(tracker_difference_callback), NULL);
    g_signal_connect(G_OBJECT(tracker), "value-changed", G_CALLBACK(tracker_value_changed_callback), NULL);
    g_signal_connect(G_OBJECT(tracker), "button_press_event", G_CALLBACK(progress_callback), NULL);
    g_signal_connect_swapped(G_OBJECT(media), "position_changed",
                             G_CALLBACK(player_position_changed_callback), tracker);
    g_signal_connect_swapped(G_OBJECT(media), "cache_percent_changed",
                             G_CALLBACK(player_cache_percent_changed_callback), tracker);
    g_signal_connect_swapped(G_OBJECT(media), "attribute_changed",
                             G_CALLBACK(player_attribute_changed_callback), tracker);
    gtk_widget_show(GTK_WIDGET(tracker));

    // fullscreen button, pack from end for this button and the vol slider
    fs_event_box = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(fs_event_box), image_fs);
    gtk_button_set_relief(GTK_BUTTON(fs_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(fs_event_box, _("Full Screen"));
    gtk_widget_set_can_focus(fs_event_box, FALSE);
    gtk_widget_set_events(fs_event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(fs_event_box), "button_press_event", G_CALLBACK(fs_callback), NULL);
    gtk_box_pack_end(GTK_BOX(hbox), fs_event_box, FALSE, FALSE, 0);
    gtk_widget_show(image_fs);
    gtk_widget_show(fs_event_box);
    // volume control
    if ((window_y > window_x)
        && (rpcontrols != NULL && g_ascii_strcasecmp(rpcontrols, "volumeslider") == 0)) {
        vol_slider = gtk_scale_new_with_range (GTK_ORIENTATION_VERTICAL, 0.0, 1.0, 0.20);
        gtk_widget_set_size_request(vol_slider, -1, window_y);
        gtk_range_set_inverted(GTK_RANGE(vol_slider), TRUE);
    } else {
        vol_slider = gtk_volume_button_new();
#ifdef GTK3_ENABLED
        g_object_set(G_OBJECT(vol_slider), "use-symbolic", TRUE, NULL);
#endif
        adj = gtk_scale_button_get_adjustment(GTK_SCALE_BUTTON(vol_slider));
        gtk_scale_button_set_adjustment(GTK_SCALE_BUTTON(vol_slider), adj);
        gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider), audio_device.volume);
        if (large_buttons)
            g_object_set(G_OBJECT(vol_slider), "size", GTK_ICON_SIZE_BUTTON, NULL);
        else
            g_object_set(G_OBJECT(vol_slider), "size", GTK_ICON_SIZE_MENU, NULL);
        g_signal_connect(G_OBJECT(vol_slider), "value_changed",
                         G_CALLBACK(vol_button_value_changed_callback), idledata);
        gtk_button_set_relief(GTK_BUTTON(vol_slider), GTK_RELIEF_NONE);
    }
    // no tooltip on the volume_button is needed
    // gtk_widget_set_tooltip_text(vol_slider, idledata->vol_tooltip);
    gtk_box_pack_end(GTK_BOX(hbox), vol_slider, FALSE, FALSE, 0);
    gtk_widget_set_can_focus(vol_slider, FALSE);
    gtk_widget_show(vol_slider);
    gtk_widget_realize(window);
    return window;
}

void show_window()
{
    gint i;
    gchar **visuals;

    if (rpcontrols == NULL || (rpcontrols != NULL && g_ascii_strcasecmp(rpcontrols, "all") == 0)) {
        gtk_widget_show_all(window);
        gtk_widget_hide(media_hbox);
        if (gmtk_media_player_get_media_type(GMTK_MEDIA_PLAYER(media)) == TYPE_DVD) {
            gtk_widget_show(menu_event_box);
        } else {
            gtk_widget_hide(menu_event_box);
        }
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_showcontrols), showcontrols);

        gtk_widget_set_size_request(window, -1, -1);
        gtk_widget_set_size_request(GTK_WIDGET(tracker), 200, -1);

    } else {

        gtk_widget_show_all(window);
        gtk_widget_hide(media);
        gtk_widget_hide(menubar);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_info), FALSE);
        gtk_widget_hide(menu_event_box);
        gtk_widget_hide(controls_box);
        gm_log(verbose, G_LOG_LEVEL_MESSAGE, "showing the following controls = %s", rpcontrols);
        visuals = g_strsplit(rpcontrols, ",", 0);
        i = 0;
        while (visuals[i] != NULL) {
            if (g_ascii_strcasecmp(visuals[i], "statusbar") == 0
                || g_ascii_strcasecmp(visuals[i], "statusfield") == 0
                || g_ascii_strcasecmp(visuals[i], "positionfield") == 0
                || g_ascii_strcasecmp(visuals[i], "positionslider") == 0) {
                gtk_widget_show(GTK_WIDGET(tracker));
                gtk_widget_show(controls_box);
                gtk_widget_show(hbox);
                gtk_widget_hide(menu_event_box);
                gtk_widget_hide(media);
                control_instance = FALSE;
            }
            if (g_ascii_strcasecmp(visuals[i], "infovolumepanel") == 0) {
                gtk_widget_show(GTK_WIDGET(vol_slider));
                gtk_widget_show(controls_box);
                gtk_widget_show(hbox);
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_info), TRUE);
                control_instance = FALSE;
            }
            if (g_ascii_strcasecmp(visuals[i], "infopanel") == 0) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_info), TRUE);
                control_instance = FALSE;
            }

            if (g_ascii_strcasecmp(visuals[i], "volumeslider") == 0) {
                gtk_widget_show(GTK_WIDGET(vol_slider));
                gtk_widget_show(controls_box);
                gtk_widget_show(hbox);
            }
            if (g_ascii_strcasecmp(visuals[i], "playbutton") == 0) {
                gtk_widget_show_all(GTK_WIDGET(play_event_box));
                gtk_widget_show(controls_box);
                gtk_widget_show(hbox);
            }
            if (g_ascii_strcasecmp(visuals[i], "stopbutton") == 0) {
                gtk_widget_show_all(GTK_WIDGET(stop_event_box));
                gtk_widget_show(controls_box);
                gtk_widget_show(hbox);
            }
            if (g_ascii_strcasecmp(visuals[i], "pausebutton") == 0) {
                gtk_widget_show_all(GTK_WIDGET(play_event_box));
                gtk_widget_show(controls_box);
                gtk_widget_show(hbox);
            }
            if (g_ascii_strcasecmp(visuals[i], "ffctrl") == 0) {
                gtk_widget_show_all(GTK_WIDGET(ff_event_box));
                gtk_widget_show(controls_box);
                gtk_widget_show(hbox);
            }
            if (g_ascii_strcasecmp(visuals[i], "rwctrl") == 0) {
                gtk_widget_show_all(GTK_WIDGET(rew_event_box));
                gtk_widget_show(controls_box);
                gtk_widget_show(hbox);
            }

            if (g_ascii_strcasecmp(visuals[i], "imagewindow") == 0) {
                gtk_widget_show_all(GTK_WIDGET(media));
                control_instance = FALSE;
            }

            if (g_ascii_strcasecmp(visuals[i], "controlpanel") == 0) {
                gtk_widget_show(controls_box);
                gtk_widget_show_all(hbox);
                gtk_widget_hide(GTK_WIDGET(tracker));
                gtk_widget_hide(menu_event_box);
                gtk_widget_hide(media);
            }

            i++;
        }


    }

    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_fullscreen), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_details), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(fs_event_box), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_loop), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_play_single), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_set_subtitle), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_set_audiofile), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_take_screenshot), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_select_audio_lang), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_select_sub_lang), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_info), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_fullscreen), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_onetoone), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_onetotwo), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_twotoone), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_onetoonepointfive), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_default), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_four_three), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_sixteen_nine), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_sixteen_ten), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_anamorphic), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_aspect_follow_window), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_subtitles), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_smaller_subtitle), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_larger_subtitle), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_decrease_subtitle_delay), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_increase_subtitle_delay), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_angle), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_view_advanced), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(menuitem_edit_random), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_widget_hide(prev_event_box);
    gtk_widget_hide(next_event_box);
    gtk_widget_hide(GTK_WIDGET(menuitem_prev));
    gtk_widget_hide(GTK_WIDGET(menuitem_next));
    gtk_widget_hide(media_hbox);
    gtk_widget_hide(plvbox);
    gtk_widget_hide(details_vbox);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_view_details), details_visible);
    gtk_widget_hide(GTK_WIDGET(menuitem_edit_switch_audio));
    if (keep_on_top)
        gtk_window_set_keep_above(GTK_WINDOW(window), keep_on_top);
    update_status_icon();
}

void present_main_window()
{
    gtk_window_present(GTK_WINDOW(window));
}

static inline double logdb(double v)
{
    if (v > 1)
        return 0;
    if (v <= 1E-8)
        return METER_BARS - 1;
    return log(v) / -0.23025850929940456840179914546843642076;
}

void show_fs_controls()
{
    gint x, y;
    GdkScreen *screen;
    GdkRectangle rect;
    GtkAllocation alloc;
#ifdef GTK3_ENABLED
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
#endif


    Wg_mutex_lock(&fs_controls_lock);
    if (fs_controls == NULL && fullscreen) {
        fs_controls = gtk_window_new(GTK_WINDOW_POPUP);
        gtk_widget_add_events(fs_controls, GDK_ENTER_NOTIFY_MASK);
        gtk_widget_add_events(fs_controls, GDK_LEAVE_NOTIFY_MASK);
        g_signal_connect(G_OBJECT(fs_controls), "enter_notify_event", G_CALLBACK(fs_controls_entered), NULL);
        g_signal_connect(G_OBJECT(fs_controls), "leave_notify_event", G_CALLBACK(fs_controls_left), NULL);
        gtk_window_set_transient_for(GTK_WINDOW(fs_controls), GTK_WINDOW(fs_window));
    }
    if (fs_controls != NULL && fullscreen) {
#ifdef GTK3_ENABLED
        if (gtk_icon_theme_has_icon(icon_theme, "view-restore-symbolic")) {
            gtk_image_set_from_icon_name(GTK_IMAGE(image_fs), "view-restore-symbolic", button_size);
        } else {
            gtk_image_set_from_stock(GTK_IMAGE(image_fs), "gtk-leave-fullscreen", button_size);
        }
#else
        gtk_image_set_from_stock(GTK_IMAGE(image_fs), "gtk-leave-fullscreen", button_size);
#endif
        if (gtk_widget_get_parent(GTK_WIDGET(hbox)) == controls_box) {
            g_object_ref(hbox);
            gtk_container_remove(GTK_CONTAINER(controls_box), hbox);
            gtk_container_add(GTK_CONTAINER(fs_controls), hbox);
            g_object_unref(hbox);
        }
        gtk_widget_show(fs_controls);
#ifdef GTK3_ENABLED
        gtk_widget_set_opacity(GTK_WIDGET(fs_controls), 0.75);
#else
        gtk_window_set_opacity(GTK_WINDOW(fs_controls), 0.75);
#endif

        screen = gtk_window_get_screen(GTK_WINDOW(window));
        gtk_window_set_screen(GTK_WINDOW(fs_controls), screen);
        gdk_screen_get_monitor_geometry(screen, gdk_screen_get_monitor_at_window(screen, gtk_widget_get_window(window)),
                                        &rect);
        gtk_widget_get_allocation(fs_controls, &alloc);
        gtk_widget_set_size_request(fs_controls, rect.width / 2, alloc.height);
        x = rect.x + (rect.width / 4);
        y = rect.y + rect.height - alloc.height;
        gtk_window_move(GTK_WINDOW(fs_controls), x, y);
    }
    Wg_mutex_unlock(&fs_controls_lock);

}

void hide_fs_controls()
{
#ifdef GTK3_ENABLED
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
#endif

    Wg_mutex_lock(&fs_controls_lock);
    if (fs_controls != NULL) {
#ifdef GTK3_ENABLED
        if (gtk_icon_theme_has_icon(icon_theme, "view-fullscreen-symbolic")) {
            gtk_image_set_from_icon_name(GTK_IMAGE(image_fs), "view-fullscreen-symbolic", button_size);
        } else {
            gtk_image_set_from_stock(GTK_IMAGE(image_fs), "gtk-fullscreen", button_size);
        }
#else
        gtk_image_set_from_stock(GTK_IMAGE(image_fs), "gtk-fullscreen", button_size);
#endif
        if (gtk_widget_get_parent(GTK_WIDGET(hbox)) == fs_controls) {
            g_object_ref(hbox);
            gtk_container_remove(GTK_CONTAINER(fs_controls), hbox);
            gtk_container_add(GTK_CONTAINER(controls_box), hbox);
            g_object_unref(hbox);
        }
        if (fullscreen) {
            gtk_widget_hide(GTK_WIDGET(fs_controls));
        } else {
            gtk_widget_destroy(GTK_WIDGET(fs_controls));
            fs_controls = NULL;
        }
    }
    Wg_mutex_unlock(&fs_controls_lock);

}
