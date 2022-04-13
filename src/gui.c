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
#include "gui-menu.c"

static GtkWidget *fs_window;
static GtkWidget *fs_controls;

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

static GtkWidget *button_event_box;
static GtkWidget *image_button;

static GtkWidget *stop_event_box;
static GtkWidget *ff_event_box;
static GtkWidget *rew_event_box;

static gboolean in_button;

static GtkWidget *image_play;
static GtkWidget *image_fs;
static const char *icon_play;
static const char *icon_pause;
static const char *icon_fs;
static const char *icon_nofs;

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
    gm_log(verbose, G_LOG_LEVEL_MESSAGE, "setting gui state to %s", PLAYSTATE_to_string(guistate));

    if (lastguistate != guistate) {
        if (guistate == PLAYING) {
            w_gtk_image_set_from_icon_name (GTK_IMAGE(image_play), icon_pause, GTK_ICON_SIZE_BUTTON);
            w_gtk_widget_change_tooltip (play_event_box, _("Pause"));
            gtk_widget_set_sensitive(ff_event_box, TRUE);
            gtk_widget_set_sensitive(rew_event_box, TRUE);
            gtk_widget_hide (GTK_WIDGET (menuitem_play));
            gtk_widget_show (GTK_WIDGET (menuitem_pause));
        }

        if (guistate == PAUSED) {
            w_gtk_image_set_from_icon_name (GTK_IMAGE(image_play), icon_play, GTK_ICON_SIZE_BUTTON);
            w_gtk_widget_change_tooltip (play_event_box, _("Play"));
            gtk_widget_set_sensitive(ff_event_box, FALSE);
            gtk_widget_set_sensitive(rew_event_box, FALSE);
            gtk_widget_hide (GTK_WIDGET (menuitem_pause));
            gtk_widget_show (GTK_WIDGET (menuitem_play));
        }

        if (guistate == STOPPED) {
            w_gtk_image_set_from_icon_name (GTK_IMAGE(image_play), icon_play, GTK_ICON_SIZE_BUTTON);
            w_gtk_widget_change_tooltip (play_event_box, _("Play"));
            gtk_widget_set_sensitive(ff_event_box, FALSE);
            gtk_widget_set_sensitive(rew_event_box, FALSE);
            gtk_widget_hide (GTK_WIDGET (menuitem_pause));
            gtk_widget_show (GTK_WIDGET (menuitem_play));
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

    cancel = w_gtk_button_new (_("_Cancel"), "gtk-cancel", cancel_clicked, NULL);

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
        case GDK_KEY(ISO_Next_Group):
            setup_accelerators();
            return FALSE;
        case GDK_KEY(Right):
            if (title_is_menu) {
                return gmtk_media_player_send_key_press_event(GMTK_MEDIA_PLAYER(media), event, data);
            } else {
                if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP
                    && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist)))
                    return ff_callback(NULL, NULL, NULL);
            }
            break;
        case GDK_KEY(Left):
            if (title_is_menu) {
                return gmtk_media_player_send_key_press_event(GMTK_MEDIA_PLAYER(media), event, data);
            } else {
                if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) != MEDIA_STATE_STOP
                    && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem_view_playlist)))
                    return rew_callback(NULL, NULL, NULL);
            }
            break;
        case GDK_KEY(Page_Up):
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
        case GDK_KEY(Page_Down):
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
        case GDK_KEY(Up):
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
        case GDK_KEY(Down):
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
        case GDK_KEY(Return):
        case GDK_KEY(KP_Enter):
            if (title_is_menu) {
                return gmtk_media_player_send_key_press_event(GMTK_MEDIA_PLAYER(media), event, data);
            } else if (idledata->videopresent) {
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);
            }
            return FALSE;
        case GDK_KEY(less):
            prev_callback(NULL, NULL, NULL);
            return FALSE;
        case GDK_KEY(greater):
            next_callback(NULL, NULL, NULL);
            return FALSE;
        case GDK_KEY(space):
        case GDK_KEY(p):
            return play_callback(NULL, NULL, NULL);
            break;
        case GDK_KEY(m):
            if (gmtk_media_player_get_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED)) {
                gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED, FALSE);
                gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider), audio_device.volume);
            } else {
                gmtk_media_player_set_attribute_boolean(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_MUTED, TRUE);
                gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider), 0.0);
            }
            return FALSE;

        case GDK_KEY(1):
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_CONTRAST, -5);
            return FALSE;
        case GDK_KEY(2):
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_CONTRAST, 5);
            return FALSE;
        case GDK_KEY(3):
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_BRIGHTNESS, -5);
            return FALSE;
        case GDK_KEY(4):
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_BRIGHTNESS, 5);
            return FALSE;
        case GDK_KEY(5):
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HUE, -5);
            return FALSE;
        case GDK_KEY(6):
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_HUE, 5);
            return FALSE;
        case GDK_KEY(7):
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SATURATION, -5);
            return FALSE;
        case GDK_KEY(8):
            gmtk_media_player_set_attribute_integer_delta(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SATURATION, 5);
            return FALSE;
        case GDK_KEY(bracketleft):
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SPEED_MULTIPLIER, 0.9091);
            return FALSE;
        case GDK_KEY(bracketright):
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SPEED_MULTIPLIER, 1.10);
            return FALSE;
        case GDK_KEY(BackSpace):
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SPEED_SET, 1.0);
            return FALSE;
        case GDK_KEY(9):
            gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider),
                                       gtk_scale_button_get_value(GTK_SCALE_BUTTON(vol_slider)) - 0.01);
            return FALSE;
        case GDK_KEY(0):
            gtk_scale_button_set_value(GTK_SCALE_BUTTON(vol_slider),
                                       gtk_scale_button_get_value(GTK_SCALE_BUTTON(vol_slider)) + 0.01);
            return FALSE;
        case GDK_KEY(period):
            if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE)
                gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_FRAME_STEP);
            return FALSE;
        case GDK_KEY(j):
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SUBTITLE_SELECT);
            return FALSE;
        case GDK_KEY(y):
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SUBTITLE_STEP_FORWARD);
            return FALSE;
        case GDK_KEY(g):
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SUBTITLE_STEP_BACKWARD);
            return FALSE;
        case GDK_KEY(q):
            delete_callback(NULL, NULL, NULL);
            g_idle_add(set_destroy, NULL);
            return FALSE;
        case GDK_KEY(v):
            return FALSE;
        case GDK_KEY(KP_Add):
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_DELAY,
                                                   gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media),
                                                                                          ATTRIBUTE_AUDIO_DELAY) + 0.1);

            return FALSE;
        case GDK_KEY(minus):
        case GDK_KEY(KP_Subtract):
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_DELAY,
                                                   gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media),
                                                                                          ATTRIBUTE_AUDIO_DELAY) - 0.1);
            return FALSE;
        //case GDK_KEY(z):
        //    menuitem_view_decrease_subtitle_delay_callback(NULL, NULL);
        //    return FALSE;
        //case GDK_KEY(x):
        //    menuitem_view_increase_subtitle_delay_callback(NULL, NULL);
        //    return FALSE;
        case GDK_KEY(F11):
            if (idledata->videopresent)
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);
            return FALSE;
        case GDK_KEY(Escape):
            if (fullscreen) {
                if (idledata->videopresent)
                    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);
            } else {
                delete_callback(NULL, NULL, NULL);
                g_idle_add(set_destroy, NULL);
            }
            return FALSE;
        case GDK_KEY(f):
            if (idledata->videopresent)
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem_fullscreen), !fullscreen);
            return FALSE;
        case GDK_KEY(a):
            return FALSE;
        case GDK_KEY(d):
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SWITCH_FRAME_DROP);
            return FALSE;
        case GDK_KEY(i):
            if (fullscreen) {
                //cmd = g_strdup_printf("osd_show_text '%s' 1500 0\n", idledata->display_name);
                //send_command(cmd, TRUE);
                //g_free(cmd);
            }
            return FALSE;
        case GDK_KEY(b):
            gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_POSITION,
                                                    gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media),
                                                                                            ATTRIBUTE_SUBTITLE_POSITION)
                                                    - 1);
            return FALSE;
        case GDK_KEY(s):
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
        case GDK_KEY(B):
            gmtk_media_player_set_attribute_integer(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SUBTITLE_POSITION,
                                                    gmtk_media_player_get_attribute_integer(GMTK_MEDIA_PLAYER(media),
                                                                                            ATTRIBUTE_SUBTITLE_POSITION)
                                                    + 1);
            return FALSE;
        case GDK_KEY(S):
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_TAKE_SCREENSHOT);
            return FALSE;
        case GDK_KEY(J):
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SUBTITLE_SELECT);
            return FALSE;
        case GDK_KEY(numbersign):
            gmtk_media_player_send_command(GMTK_MEDIA_PLAYER(media), COMMAND_SWITCH_AUDIO);
            return FALSE;
        case GDK_KEY(plus):
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_AUDIO_DELAY,
                                                   gmtk_media_player_get_attribute_double(GMTK_MEDIA_PLAYER(media),
                                                                                          ATTRIBUTE_AUDIO_DELAY) + 0.1);
            return FALSE;
        case GDK_KEY(braceleft):
            gmtk_media_player_set_attribute_double(GMTK_MEDIA_PLAYER(media), ATTRIBUTE_SPEED_MULTIPLIER, 0.50);
            return FALSE;
        case GDK_KEY(braceright):
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
        case GDK_KEY(f):
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
    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PLAY ||
        gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_PAUSE) {
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(media), MEDIA_STATE_STOP);
        autopause = FALSE;
        if (gmtk_media_player_get_media_type(GMTK_MEDIA_PLAYER(media)) == TYPE_NETWORK)
            dontplaynext = TRUE;
        gmtk_media_tracker_set_percentage(tracker, 0.0);
        gtk_widget_set_sensitive(play_event_box, TRUE);
        w_gtk_image_set_from_icon_name (GTK_IMAGE(image_play), icon_play, GTK_ICON_SIZE_BUTTON);
        gtk_widget_set_tooltip_text(play_event_box, _("Play"));
    }

    if (gmtk_media_player_get_media_state(GMTK_MEDIA_PLAYER(media)) == MEDIA_STATE_QUIT) {
        gmtk_media_tracker_set_percentage(tracker, 0.0);
        w_gtk_image_set_from_icon_name (GTK_IMAGE(image_play), icon_play, GTK_ICON_SIZE_BUTTON);
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
            w_gtk_image_set_from_icon_name (GTK_IMAGE(image_play), icon_play, GTK_ICON_SIZE_BUTTON);
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
            w_gtk_image_set_from_icon_name (GTK_IMAGE(image_play), icon_play, GTK_ICON_SIZE_BUTTON);
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
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *cancel_button;
    GtkWidget *open_button;

    open_window = gtk_dialog_new ();
    gtk_window_set_type_hint(GTK_WINDOW(open_window), GDK_WINDOW_TYPE_HINT_UTILITY);
    gtk_window_set_resizable(GTK_WINDOW(open_window), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(open_window), GTK_WINDOW(window));
    gtk_window_set_position(GTK_WINDOW(open_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_icon(GTK_WINDOW(open_window), pb_icon);

    gtk_window_set_resizable(GTK_WINDOW(open_window), FALSE);
    gtk_window_set_title(GTK_WINDOW(open_window), _("Open Location"));

    vbox = gtk_dialog_get_content_area (GTK_DIALOG (open_window));
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new(_("Location:"));
    open_location = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(open_location), 50);
    gtk_entry_set_activates_default(GTK_ENTRY(open_location), TRUE);

    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 12);
    gtk_box_pack_start (GTK_BOX(vbox), open_location, TRUE, TRUE, 0);
    gtkcompat_widget_set_halign_left (GTK_WIDGET (label));

    cancel_button = gtk_dialog_add_button (GTK_DIALOG (open_window), "gtk-cancel", GTK_RESPONSE_CANCEL);
    open_button = gtk_dialog_add_button (GTK_DIALOG (open_window), "gtk-open", GTK_RESPONSE_OK);

    g_signal_connect_swapped(G_OBJECT(cancel_button), "clicked", G_CALLBACK(config_close), open_window);
    g_signal_connect_swapped(G_OBJECT(open_button), "clicked", G_CALLBACK(open_location_callback), open_window);

    gtk_widget_show_all(open_window);
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

void menuitem_about_callback(GtkMenuItem * menuitem, void *data)
{
    GtkWidget *w;
    const gchar * authors[] =
    {
        "Kevin DeKorte - main author",
        "James Carthew",
        "Diogo Franco",
        "Icons provided by Victor Castillejo",
        NULL
    };
    /* TRANSLATORS: Replace this string with your names, one name per line. */
    gchar * translators = _("Translated by");

    GdkPixbuf * logo = NULL;
    if (pb_icon)
       logo = pb_icon;

    /* Create and initialize the dialog. */
    w = g_object_new (GTK_TYPE_ABOUT_DIALOG,
                      "version",      VERSION,
                      "program-name", PACKAGE,
                      "copyright",    "Copyright (C) 2007-2022",
                      "comments",     "GTK frontend for mplayer",
                      "license",      "GPL2 - see COPYING file.",
                      "website",      PACKAGE_URL,
                      "authors",      authors,
                      "translator-credits", translators,
                      "logo",         logo,
                      NULL);
    gtk_container_set_border_width (GTK_CONTAINER (w), 2);
    gtk_window_set_transient_for (GTK_WINDOW (w), GTK_WINDOW (window));
    gtk_window_set_modal (GTK_WINDOW (w), TRUE);
    gtk_window_set_position (GTK_WINDOW (w), GTK_WIN_POS_CENTER_ON_PARENT);

    g_signal_connect_swapped (w, "response",
                              G_CALLBACK (gtk_widget_destroy), w);
    gtk_widget_show_all (GTK_WIDGET (w));
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


static void create_details_table (GtkWidget *vbox)
{
    WGtkGridParams grid;
    memset (&grid, 0, sizeof(grid));
    grid.cols = 2;
    grid.c1.align = GTK_ALIGN_END;
    grid.c2.align = GTK_ALIGN_START;

    grid.table = details_table = w_gtk_grid_new (vbox);

    grid.c1.w = gtk_label_new(_("<span weight=\"bold\">Video Details</span>"));
    grid.c1.align = GTK_ALIGN_CENTER;
    gtk_label_set_use_markup(GTK_LABEL(grid.c1.w), TRUE);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Video Size:"));
    grid.c2.w = details_video_size = gtk_label_new (_("Unknown"));
    grid.c1.align = GTK_ALIGN_END;
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Video Format:"));
    grid.c2.w = details_video_format = gtk_label_new (_("Unknown"));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Video Codec:"));
    grid.c2.w = details_video_codec = gtk_label_new(_("Unknown"));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Video FPS:"));
    grid.c2.w = details_video_fps = gtk_label_new("");
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Video Bitrate:"));
    grid.c2.w = details_video_bitrate = gtk_label_new ("0 Kb/s");
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Video Chapters:"));
    grid.c2.w = details_video_chapters = gtk_label_new (_("Unknown"));
    w_gtk_grid_append_row (&grid);

    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("<span weight=\"bold\">Audio Details</span>"));
    grid.c1.align = GTK_ALIGN_CENTER;
    gtk_label_set_use_markup(GTK_LABEL(grid.c1.w), TRUE);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Audio Format:"));
    grid.c1.align = GTK_ALIGN_END;
    grid.c2.w = details_audio_format = gtk_label_new (_("Unknown"));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Audio Codec:"));
    grid.c2.w = details_audio_codec = gtk_label_new (_("Unknown"));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Audio Channels:"));
    grid.c2.w = details_audio_channels = gtk_label_new (_("Unknown"));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Audio Bitrate:"));
    grid.c2.w = details_audio_bitrate = gtk_label_new ("0 Kb/s");
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Audio Sample Rate:"));
    grid.c2.w = details_audio_samplerate = gtk_label_new ("0 KHz");
    w_gtk_grid_append_row (&grid);
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
    // - activate/toggled trigger this cb twice with different menuitems
    // - the menuitem being deactivated and the menuitem being activated
    // - we only want to know activated menuitems
    int i = 0;
    int active;
    //const char *id;
    GmtkMediaPlayerAspectRatio aspect =  ASPECT_DEFAULT;

    active = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menuitem));;
    if (!active) {
        return;
    }
    i = GPOINTER_TO_INT (g_object_get_data (G_OBJECT(menuitem), "index"));
    //id = g_object_get_data (G_OBJECT(menuitem), "id");
    switch (i)
    {
        case 0: aspect = ASPECT_DEFAULT; break;
        case 1: aspect = ASPECT_4X3;     break;
        case 2: aspect = ASPECT_16X9;    break;
        case 3: aspect = ASPECT_16X10;   break;
        case 4: aspect = ASPECT_ANAMORPHIC; break;
        case 5: aspect = ASPECT_WINDOW;  break;
    }
    gmtk_media_player_set_aspect(GMTK_MEDIA_PLAYER(media), aspect);
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
    GtkWidget *entry;
    GtkWidget *page_table[5];
    GtkWidget *notebook;
    GdkColor sub_color;
    gint i = 0;
    gint j = -1;
    gint k = -1;
    GtkTreeIter ao_iter;
    gchar *desc;
    guint key;
    GdkModifierType modifier;

    WGtkGridParams grid;
    memset (&grid, 0, sizeof(grid));
    grid.cols = 2;
    grid.c1.align = GTK_ALIGN_END;
    grid.c2.align = GTK_ALIGN_FILL;

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

    static const char *tab_labels[] = {
        N_("Player"),
        N_("Subtitles"),
        N_("Interface"),
        N_("Keyboard Shortcuts"),
        N_("MPlayer"),
    };

    notebook = gtk_notebook_new();
    for (i = 0; i < 5 ; i++)
    {
        page_table[i] = w_gtk_notebook_add_tab (notebook, gettext(tab_labels[i]));
    }

    gtk_container_add(GTK_CONTAINER(conf_vbox), notebook);
    gtk_container_add(GTK_CONTAINER(config_window), conf_vbox);

    gtk_window_set_title(GTK_WINDOW(config_window), _("G-MPlayer Configuration"));
    gtk_container_set_border_width(GTK_CONTAINER(config_window), 5);
    //gtk_window_set_default_size(GTK_WINDOW(config_window), 300, 300);

    conf_ok = w_gtk_button_new (_("_OK"), "gtk-ok", NULL, NULL);
    g_signal_connect_swapped(G_OBJECT(conf_ok), "clicked", G_CALLBACK(config_apply), config_window);

    conf_cancel = w_gtk_button_new (_("_Close"), "gtk-close", NULL, NULL);
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
    grid.table = page_table[0];
    grid.row = 0;

    grid.c1.w = gtk_label_new(_("<span weight=\"bold\">Language Settings</span>"));
    grid.c1.align = GTK_ALIGN_START;
    gtk_label_set_use_markup(GTK_LABEL(grid.c1.w), TRUE);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Default Audio Language"));
    grid.c2.w = config_alang;
    grid.c1.align = GTK_ALIGN_END;
    gtk_widget_set_size_request(GTK_WIDGET(config_alang), 200, -1);
    gtk_widget_set_tooltip_text(grid.c1.w, _("Choose one of the languages or type in your own comma-separated selection"));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Default Subtitle Language:"));
    grid.c2.w = config_slang;
    gtk_widget_set_size_request(GTK_WIDGET(config_slang), 200, -1);
    gtk_widget_set_tooltip_text (grid.c1.w,_("Choose one of the languages or type in your own comma-separated selection"));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("File Metadata Encoding:"));
    grid.c2.w = config_metadata_codepage;
    gtk_widget_set_size_request(GTK_WIDGET(config_metadata_codepage), 200, -1);
    w_gtk_grid_append_row (&grid);

    w_gtk_grid_append_row (&grid); /* space */

    grid.c1.w = gtk_label_new(_("<span weight=\"bold\">Output Settings</span>"));
    grid.c1.align = GTK_ALIGN_START;
    gtk_label_set_use_markup(GTK_LABEL(grid.c1.w), TRUE);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Video Output:"));
    grid.c2.w = config_vo;
    grid.c1.align = GTK_ALIGN_END;
    gtk_widget_set_size_request(GTK_WIDGET(config_vo), 200, -1);
    w_gtk_grid_append_row (&grid);

    grid.c2.w = gtk_check_button_new_with_label(_("Enable Video Hardware Support"));
    config_hardware_codecs = grid.c2.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_hardware_codecs), use_hardware_codecs);
    gtk_widget_set_tooltip_text(config_hardware_codecs, _("When this option is enabled, codecs or options will be enabled to accelerate video processing. These options may cause playback to fail in some cases."));
    w_gtk_grid_append_row (&grid);

    grid.c2.w = gtk_check_button_new_with_label(_("Enable CrystalHD Hardware Support"));
    config_crystalhd_codecs = grid.c2.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_crystalhd_codecs), use_crystalhd_codecs);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Audio Output:"));
    grid.c2.w = config_ao;
    gtk_widget_set_size_request(GTK_WIDGET(config_ao), 200, -1);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Audio Volume Type:"));
    grid.c2.w = conf_volume_label;
    grid.c2.align = GTK_ALIGN_START;
    w_gtk_grid_append_row (&grid);

    grid.c2.align = GTK_ALIGN_FILL;
#ifdef HAVE_ASOUNDLIB
    grid.c1.w = gtk_label_new(_("Default Mixer:"));
    grid.c2.w = config_mixer;
    gtk_widget_set_size_request(GTK_WIDGET(config_mixer), 200, -1);
    w_gtk_grid_append_row (&grid);
#endif

    grid.c1.w = gtk_label_new(_("Audio Channels to Output:"));
    grid.c2.w = config_audio_channels;
    gtk_widget_set_size_request(GTK_WIDGET(config_audio_channels), 200, -1);
    w_gtk_grid_append_row (&grid);

    grid.c2.w = config_use_hw_audio;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_use_hw_audio), use_hw_audio);
    w_gtk_grid_append_row (&grid);

    w_gtk_grid_append_row (&grid); /* space */

    grid.c1.w = gtk_label_new(_("<span weight=\"bold\">Configuration Settings</span>"));
    grid.c1.align = GTK_ALIGN_START;
    gtk_label_set_use_markup(GTK_LABEL(grid.c1.w), TRUE);
    w_gtk_grid_append_row (&grid);

#ifndef HAVE_ASOUNDLIB
    grid.c1.w = gtk_label_new(_("Default Volume Level:"));
    grid.c2.w = gtk_spin_button_new_with_range(0, 100, 1);
    grid.c1.align = GTK_ALIGN_END;
    config_volume = grid.c2.w;
    gtk_widget_set_tooltip_text(config_volume, _("Default volume for playback"));
    gtk_widget_set_size_request(config_volume, 100, -1);
    gm_store = gm_pref_store_new("g-mplayer");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_volume), gm_pref_store_get_int(gm_store, VOLUME));
    gm_pref_store_free(gm_store);
    gtk_entry_set_width_chars(GTK_ENTRY(config_volume), 6);
    gtk_editable_set_editable(GTK_EDITABLE(config_volume), TRUE);
    gtk_entry_set_alignment(GTK_ENTRY(config_volume), 1);
    w_gtk_grid_append_row (&grid);
#endif

    grid.c1.w = gtk_label_new(_("On Screen Display Level:"));
    grid.c2.w = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, 3.0, 1.0);
    grid.c1.align = GTK_ALIGN_END;
    config_osdlevel = grid.c2.w;
    gtk_range_set_value(GTK_RANGE(config_osdlevel), osdlevel);
    g_signal_connect(G_OBJECT(config_osdlevel), "format-value", G_CALLBACK(osdlevel_format_callback), NULL);
    g_signal_connect(G_OBJECT(config_osdlevel), "value-changed", G_CALLBACK(osdlevel_change_callback), NULL);
    gtk_widget_set_size_request(config_osdlevel, 150, -1);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Post-processing level:"));
    grid.c2.w = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, 6.0, 1.0);
    config_pplevel = grid.c2.w;
    g_signal_connect(G_OBJECT(config_pplevel), "format-value", G_CALLBACK(pplevel_format_callback), NULL);
    gtk_widget_set_size_request(config_pplevel, 150, -1);
    gtk_range_set_value(GTK_RANGE(config_pplevel), pplevel);
    w_gtk_grid_append_row (&grid);


    /// Page 1 - SUBTITLES
    grid.table = page_table[1];
    grid.row = 0;

    grid.c1.w = gtk_label_new(_("<span weight=\"bold\">Subtitle Settings</span>"));
    grid.c1.align = GTK_ALIGN_START;
    gtk_label_set_use_markup(GTK_LABEL(grid.c1.w), TRUE);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_mnemonic(_("Enable _Advanced Substation Alpha (ASS) Subtitle Support"));
    grid.c1.align = GTK_ALIGN_START;
    config_ass = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_ass), !disable_ass);
    g_signal_connect(G_OBJECT(config_ass), "toggled", G_CALLBACK(ass_toggle_callback), NULL);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_mnemonic(_("Use _Embedded Fonts (MKV only)"));
    config_embeddedfonts = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_embeddedfonts), !disable_embeddedfonts);
    gtk_widget_set_sensitive(config_embeddedfonts, !disable_ass);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Outline Subtitle Font"));
    config_subtitle_outline = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_subtitle_outline), subtitle_outline);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Shadow Subtitle Font"));
    config_subtitle_shadow = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_subtitle_shadow), subtitle_shadow);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Show Subtitles by Default"));
    config_show_subtitles = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_show_subtitles), showsubtitles);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Subtitle Font:"));
    grid.c2.w = gtk_font_button_new();
    grid.c1.align = GTK_ALIGN_END;
    config_subtitle_font = grid.c2.w;
    if (subtitlefont != NULL) {
        gtk_font_button_set_font_name(GTK_FONT_BUTTON(config_subtitle_font), subtitlefont);
    }
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(config_subtitle_font), TRUE);
    gtk_font_button_set_show_style(GTK_FONT_BUTTON(config_subtitle_font), TRUE);
    gtk_font_button_set_use_size(GTK_FONT_BUTTON(config_subtitle_font), FALSE);
    gtk_font_button_set_use_font(GTK_FONT_BUTTON(config_subtitle_font), TRUE);
    gtk_font_button_set_title(GTK_FONT_BUTTON(config_subtitle_font), _("Subtitle Font Selection"));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Subtitle Color:"));
    grid.c2.w = gtk_color_button_new();
    config_subtitle_color = grid.c2.w;
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
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Subtitle Font Scaling:"));
    grid.c2.w = gtk_spin_button_new_with_range(0.25, 10, 0.05);
    config_subtitle_scale = grid.c2.w;
    gtk_widget_set_size_request(config_subtitle_scale, -1, -1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_subtitle_scale), subtitle_scale);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Subtitle File Encoding:"));
    grid.c2.w = config_subtitle_codepage;
    gtk_widget_set_size_request(GTK_WIDGET(config_subtitle_codepage), 200, -1);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Subtitle Lower Margin (X11/XV Only):"));
    grid.c2.w = gtk_spin_button_new_with_range(0, 200, 1);
    config_subtitle_margin = grid.c2.w;
    gtk_widget_set_size_request(config_subtitle_scale, -1, -1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_subtitle_margin), subtitle_margin);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Subtitle Load Fuzziness:"));
    grid.c2.w = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, 2.0, 1.0);
    config_subtitle_fuzziness = grid.c2.w;
    g_signal_connect(G_OBJECT(config_subtitle_fuzziness), "format-value",
                     G_CALLBACK(subtitle_fuzziness_format_callback), NULL);
    gtk_widget_set_size_request(config_subtitle_fuzziness, 150, -1);
    gtk_range_set_value(GTK_RANGE(config_subtitle_fuzziness), subtitle_fuzziness);
    w_gtk_grid_append_row (&grid);


    /// Page 2
    grid.table = page_table[2];
    grid.row = 0;

    grid.c1.w = gtk_label_new(_("<span weight=\"bold\">Application Preferences</span>"));
    grid.c1.align = GTK_ALIGN_START;
    gtk_label_set_use_markup(GTK_LABEL(grid.c1.w), TRUE);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Media Resume:"));
    grid.c2.w = config_resume_mode;
    grid.c1.align = GTK_ALIGN_END;
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Start with playlist visible"));
    grid.c1.align = GTK_ALIGN_START;
    config_playlist_visible = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_playlist_visible), playlist_visible);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Start with details visible"));
    config_details_visible = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_details_visible), details_visible);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Respond to Keyboard Media Keys"));
    config_use_mediakeys = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_use_mediakeys), use_mediakeys);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Use default playlist"));
    config_use_defaultpl = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_use_defaultpl), use_defaultpl);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Show status icon"));
    config_show_status_icon = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_show_status_icon), show_status_icon);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Place playlist below media (requires application restart)"));
    config_vertical_layout = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_vertical_layout), vertical_layout);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Only allow one instance of G-MPlayer"));
    config_single_instance = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_single_instance), single_instance);
    g_signal_connect(G_OBJECT(config_single_instance), "toggled", G_CALLBACK(config_single_instance_callback), NULL);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("When opening in single instance mode, replace existing file"));
    grid.c1.margin_start = 12;
    config_replace_and_play = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_replace_and_play), replace_and_play);
    gtk_widget_set_sensitive(config_replace_and_play,
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_single_instance)));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("When opening file, bring main window to front"));
    grid.c1.margin_start = 12;
    config_bring_to_front = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_bring_to_front), bring_to_front);
    gtk_widget_set_sensitive(config_bring_to_front,
                             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_single_instance)));
    gtk_widget_set_margin_start (config_bring_to_front, 12);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Remember Window Location and Size"));
    config_remember_loc = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_remember_loc), remember_loc);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Resize window when new video is loaded"));
    config_resize_on_new_media = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_resize_on_new_media), resize_on_new_media);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Keep window above other windows"));
    config_keep_on_top = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_keep_on_top), keep_on_top);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Pause playback on mouse click"));
    config_pause_on_click = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_pause_on_click), !disable_pause_on_click);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Disable Fullscreen Control Bar Animation"));
    config_disable_animation = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_disable_animation), disable_animation);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Disable Cover Art Fetch"));
    config_disable_cover_art_fetch = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_disable_cover_art_fetch), disable_cover_art_fetch);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Use Mouse Wheel to change volume, instead of seeking"));
    config_mouse_wheel = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_mouse_wheel), mouse_wheel_changes_volume);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Verbose Debug Enabled"));
    config_verbose = grid.c1.w;
    gtk_widget_set_tooltip_text(config_verbose, _("When this option is set, extra debug information is sent to the terminal or into ~/.xsession-errors"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_verbose), verbose);
    w_gtk_grid_append_row (&grid);

    /// page 3
    grid.table = page_table[3];
    grid.row = 0;

    grid.c1.w = gtk_label_new(_("<span weight=\"bold\">Keyboard Shortcuts</span>\n\n"
                                 "Place the cursor in the box next to the shortcut you want to modify\n"
                                 "Then press the keys you would like for the shortcut"));
    grid.c1.align = GTK_ALIGN_START;
    gtk_label_set_use_markup(GTK_LABEL(grid.c1.w), TRUE);
    w_gtk_grid_append_row (&grid);

    grid.c1.align = GTK_ALIGN_END;

    for (j = 0; j < KEY_COUNT; j++)
    {
        grid.c1.w = gtk_label_new(accel_keys_description[j]);
        grid.c2.w = gtk_entry_new();
        config_accel_keys[j] = grid.c2.w;
        if (get_key_and_modifier(accel_keys[j], &key, &modifier)) {
            gtk_entry_set_text(GTK_ENTRY(config_accel_keys[j]),
                               g_strdup_printf("%s%s%s%s", (modifier & GDK_CONTROL_MASK) ? "Control-" : "",
                                               (modifier & GDK_MOD1_MASK) ? "Alt-" : "",
                                               (modifier & GDK_SHIFT_MASK) ? "Shift-" : "", gdk_keyval_name(key)));
        }
        g_signal_connect(G_OBJECT(config_accel_keys[j]), "key_press_event", G_CALLBACK(accel_key_key_press_event),
                         GINT_TO_POINTER(j));
        gtk_widget_set_size_request(GTK_WIDGET(config_accel_keys[j]), 150, -1);
        w_gtk_grid_append_row (&grid);
    }

    grid.c1.w = gtk_button_new_with_label(_("Reset Keys"));
    grid.c1.width = 1;
    g_signal_connect(G_OBJECT(grid.c1.w), "clicked", G_CALLBACK(reset_keys_callback), NULL);
    gtk_widget_set_size_request(grid.c1.w, 100, -1);
    w_gtk_grid_append_row (&grid);


    /// page 4
    grid.table = page_table[4];
    grid.row = 0;

    grid.c1.w = gtk_label_new(_("<span weight=\"bold\">Advanced Settings for MPlayer</span>"));
    grid.c1.align = GTK_ALIGN_START;
    gtk_label_set_use_markup(GTK_LABEL(grid.c1.w), TRUE);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = config_softvol;
    gtk_widget_set_tooltip_text (config_softvol, _("Set this option if changing the volume in G-MPlayer changes the master volume"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_softvol), softvol);
    g_signal_connect(G_OBJECT(config_softvol), "toggled", G_CALLBACK(config_softvol_callback), NULL);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Remember last software volume level"));
    config_remember_softvol = grid.c1.w;
    gtk_widget_set_tooltip_text (config_remember_softvol, _("Set this option if you want the software volume level to be remembered"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_remember_softvol), remember_softvol);
    gtk_widget_set_sensitive(config_remember_softvol, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_softvol)));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Volume Gain (-200dB to +60dB)"));
    grid.c2.w = gtk_spin_button_new_with_range(-200, 60, 1);
    config_volume_gain = grid.c2.w;
    gtk_widget_set_size_request(config_volume_gain, -1, -1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_volume_gain), volume_gain);
    gtk_widget_set_sensitive(config_volume_gain, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_softvol)));
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_mnemonic(_("De_interlace Video"));
    config_deinterlace = grid.c1.w;
    gtk_widget_set_tooltip_text(config_deinterlace, _("Set this option if video looks striped"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_deinterlace), !disable_deinterlace);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_mnemonic(_("_Drop frames"));
    config_framedrop = grid.c1.w;
    gtk_widget_set_tooltip_text(config_framedrop, _("Set this option if video is well behind the audio"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_framedrop), !disable_framedrop);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_check_button_new_with_label(_("Enable mplayer cache"));
    config_forcecache = grid.c1.w;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_forcecache), forcecache);
    g_signal_connect(G_OBJECT(config_forcecache), "toggled", G_CALLBACK(config_forcecache_callback), NULL);
    w_gtk_grid_append_row (&grid);

    grid.c1.w = gtk_label_new(_("Cache Size (KB):"));
    grid.c2.w = gtk_spin_button_new_with_range(32, 256 * 1024, 512);
    config_cachesize = grid.c2.w;
    gtk_widget_set_tooltip_text(config_cachesize, _
                                ("Amount of data to cache when playing media, use higher values for slow devices and sites."));
    gtk_widget_set_size_request(config_cachesize, 100, -1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_cachesize), cache_size);
    gtk_entry_set_width_chars(GTK_ENTRY(config_cachesize), 6);
    gtk_entry_set_alignment(GTK_ENTRY(config_cachesize), 1);
    gtk_widget_set_sensitive(config_cachesize, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_forcecache)));
    w_gtk_grid_append_row (&grid);

    w_gtk_grid_append_row (&grid); /* space */

    grid.c1.w = gtk_label_new(_("MPlayer Executable:"));
    grid.c2.w = gtk_file_chooser_button_new("Select", GTK_FILE_CHOOSER_ACTION_OPEN);
    config_mplayer_bin = grid.c2.w;
    gtk_widget_set_tooltip_text(config_mplayer_bin, _("Use this option to specify a mplayer application that is not in the path"));
    if (mplayer_bin != NULL) {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(config_mplayer_bin), mplayer_bin);
    }
    w_gtk_grid_append_row (&grid);

    w_gtk_grid_append_row (&grid); /* space */

    grid.c1.w = gtk_label_new(_("MPlayer Default Optical Device"));
    grid.c2.w  = config_mplayer_dvd_device;
    gtk_widget_set_size_request(GTK_WIDGET(config_mplayer_dvd_device), 200, -1);
    w_gtk_grid_append_row (&grid);

    w_gtk_grid_append_row (&grid); /* space */

    grid.c1.w = gtk_label_new(_("Extra Options to MPlayer:"));
    grid.c1.align = GTK_ALIGN_START;
    w_gtk_grid_append_row (&grid);

    config_extraopts = gtk_entry_new();
    grid.c1.w = config_extraopts;
    grid.c1.align = GTK_ALIGN_FILL;
    gtk_widget_set_tooltip_text(config_extraopts, _("Add any extra mplayer options here (filters etc)"));
    gtk_entry_set_text(GTK_ENTRY(config_extraopts), ((extraopts) ? extraopts : ""));
    gtk_entry_set_width_chars(GTK_ENTRY(config_extraopts), 40);
    w_gtk_grid_append_row (&grid);

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
    gchar *short_filename = NULL;
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
        w_gtk_image_set_from_icon_name (GTK_IMAGE(image_play), icon_play, GTK_ICON_SIZE_BUTTON);
        w_gtk_widget_change_tooltip (play_event_box, _("Play"));
        gmtk_media_tracker_set_position(GMTK_MEDIA_TRACKER(tracker), 0.0);
        gtk_widget_set_sensitive(ff_event_box, FALSE);
        gtk_widget_set_sensitive(rew_event_box, FALSE);
        gtk_widget_set_sensitive(prev_event_box, FALSE);
        gtk_widget_set_sensitive(next_event_box, FALSE);
        gtk_widget_set_sensitive(stop_event_box, FALSE);
        gtk_widget_hide (GTK_WIDGET (menuitem_pause));
        gtk_widget_show (GTK_WIDGET (menuitem_play));
        g_strlcpy(idledata->media_info, "", sizeof(idledata->media_info));
        gtk_widget_hide(media_hbox);
        g_strlcpy(idledata->display_name, "", sizeof(idledata->display_name));
        g_idle_add(set_title_bar, idledata);
        break;
    case MEDIA_STATE_PLAY:
        if (idledata->mapped_af_export == NULL)
            map_af_export_file(idledata);
        w_gtk_image_set_from_icon_name (GTK_IMAGE(image_play), icon_pause, GTK_ICON_SIZE_BUTTON);
        w_gtk_widget_change_tooltip (play_event_box, _("Play"));
        gtk_widget_set_sensitive(ff_event_box, TRUE);
        gtk_widget_set_sensitive(rew_event_box, TRUE);
        gtk_widget_set_sensitive(prev_event_box, TRUE);
        gtk_widget_set_sensitive(next_event_box, TRUE);
        gtk_widget_set_sensitive(stop_event_box, TRUE);
        gtk_widget_hide (GTK_WIDGET (menuitem_play));
        gtk_widget_show (GTK_WIDGET (menuitem_pause));

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
        w_gtk_image_set_from_icon_name (GTK_IMAGE(image_play), icon_play, GTK_ICON_SIZE_BUTTON);
        w_gtk_widget_change_tooltip (play_event_box, _("Play"));
        gtk_widget_set_sensitive(ff_event_box, FALSE);
        gtk_widget_set_sensitive(rew_event_box, FALSE);
        gtk_widget_set_sensitive(prev_event_box, FALSE);
        gtk_widget_set_sensitive(next_event_box, FALSE);
        gtk_widget_set_sensitive(stop_event_box, FALSE);
        gtk_widget_hide (GTK_WIDGET (menuitem_pause));
        gtk_widget_show (GTK_WIDGET (menuitem_play));
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
            gtk_menu_popup (GTK_MENU(popup_menu), NULL, NULL, NULL, NULL, event_button->button, event_button->time);
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
            gtk_menu_popup (GTK_MENU(popup_menu), NULL, NULL, NULL, NULL, event_button->button, event_button->time);
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

    gtk_accel_map_change_entry(ACCEL_PATH_VIEW_NORMAL, GDK_KEY(1), GDK_CONTROL_MASK, TRUE);
    gtk_accel_map_change_entry(ACCEL_PATH_VIEW_DOUBLE, GDK_KEY(2), GDK_CONTROL_MASK, TRUE);
}

GtkWidget *create_window()
{
    GtkIconTheme *icon_theme;
    GtkTargetEntry target_entry[3];
    gint i = 0;
    GtkAdjustment *adj;
    const char *icon_start, *icon_seek_forward, *icon_seek_backward, *icon_skip_forward, *icon_skip_backward;
    static const char *icon_stop;
    static const char *icon_ff;
    static const char *icon_rew;
    static const char *icon_next;
    static const char *icon_prev;
    static const char *icon_menu;

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
    create_popup_menu (accel_group);
    gtk_widget_hide (GTK_WIDGET (menuitem_pause));
    g_signal_connect_swapped(G_OBJECT(window), "button_release_event", G_CALLBACK(popup_handler), G_OBJECT(popup_menu));
    g_signal_connect_swapped(G_OBJECT(window), "enter_notify_event", G_CALLBACK(notification_handler), NULL);
    g_signal_connect_swapped(G_OBJECT(window), "leave_notify_event", G_CALLBACK(notification_handler), NULL);

    // File Menu
    recent_manager = gtk_recent_manager_get_default ();
    create_menubar (accel_group);

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

    g_signal_connect(G_OBJECT(menu_file_recent),
                     "item-activated", G_CALLBACK(menuitem_open_recent_callback), NULL);

    g_signal_connect(G_OBJECT(menuitem_file_quit), "activate", G_CALLBACK(menuitem_quit_callback), NULL);

    // Edit Menu
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
    g_signal_connect(G_OBJECT(menuitem_help_about), "activate", G_CALLBACK(menuitem_about_callback), NULL);

    // window
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
    //==================================
    create_details_table (details_vbox);
    //==================================
    g_signal_connect(details_vbox, "show", G_CALLBACK(view_option_show_callback), NULL);
    g_signal_connect(details_vbox, "size_allocate", G_CALLBACK(view_option_size_allocate_callback), NULL);

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

    if (gtk_icon_theme_has_icon(icon_theme, icon_start) &&
        gtk_icon_theme_has_icon(icon_theme, "media-playback-stop-symbolic") &&
        gtk_icon_theme_has_icon(icon_theme, "media-playback-pause-symbolic") &&
        gtk_icon_theme_has_icon(icon_theme, icon_seek_forward) &&
        gtk_icon_theme_has_icon(icon_theme, icon_seek_backward) &&
        gtk_icon_theme_has_icon(icon_theme, "view-sidebar-symbolic") &&
        gtk_icon_theme_has_icon(icon_theme, "view-fullscreen-symbolic"))
    {
        icon_play   = icon_start;
        icon_stop   = "media-playback-stop-symbolic";
        icon_pause  = "media-playback-pause-symbolic";
        icon_ff     = icon_seek_forward;
        icon_rew    = icon_seek_backward;
        icon_prev   = icon_skip_backward;
        icon_next   = icon_skip_forward;
        icon_menu   = "view-sidebar-symbolic";
        icon_fs     = "view-fullscreen-symbolic";
        icon_nofs   = "view-restore-symbolic";
    } else {
        icon_play   = "gtk-media-play";
        icon_stop   = "gtk-media-stop";
        icon_pause  = "gtk-media-pause";
        icon_ff     = "gtk-media-forward";
        icon_rew    = "gtk-media-rewind";
        icon_prev   = "gtk-media-previous";
        icon_next   = "gtk-media-next";
        icon_menu   = "gtk-index";
        icon_fs     = "gtk-fullscreen";
        icon_nofs   = "gtk-leave-fullscreen";
    }

    image_play = w_gtk_image_new_from_icon_name (icon_play, GTK_ICON_SIZE_BUTTON);
    image_fs   = w_gtk_image_new_from_icon_name (icon_fs,   GTK_ICON_SIZE_BUTTON);

    if (gtk_icon_theme_has_icon(icon_theme, "g-mplayer")) {
        gtk_window_set_default_icon_name ("g-mplayer");
        pb_icon = gtk_icon_theme_load_icon (icon_theme, "g-mplayer", 64, 0, NULL);
    } else {
        pb_icon = gdk_pixbuf_new_from_xpm_data((const char **) g_mplayer_xpm);
        if (pb_icon) {
            gtk_window_set_default_icon (pb_icon);
        }
    }

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

    menu_event_box = w_gtk_button_new (NULL, icon_menu, menu_callback, NULL);
    gtk_button_set_relief(GTK_BUTTON(menu_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(menu_event_box, _("Menu"));
    gtk_widget_set_can_focus(menu_event_box, FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), menu_event_box, FALSE, FALSE, 0);
    gtk_widget_show(menu_event_box);

    prev_event_box = w_gtk_button_new (NULL, icon_prev, prev_callback, NULL);
    gtk_button_set_relief(GTK_BUTTON(prev_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(prev_event_box, _("Previous"));
    gtk_widget_set_can_focus(prev_event_box, FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), prev_event_box, FALSE, FALSE, 0);
    gtk_widget_show(prev_event_box);
    gtk_widget_set_sensitive(prev_event_box, FALSE);

    rew_event_box = w_gtk_button_new (NULL, icon_rew, rew_callback, NULL);
    gtk_button_set_relief(GTK_BUTTON(rew_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(rew_event_box, _("Rewind"));
    gtk_widget_set_can_focus(rew_event_box, FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), rew_event_box, FALSE, FALSE, 0);
    gtk_widget_show(rew_event_box);
    gtk_widget_set_sensitive(rew_event_box, FALSE);

    play_event_box = w_gtk_button_new (NULL, NULL, play_callback, NULL);
    gtk_button_set_image (GTK_BUTTON(play_event_box), image_play);
    gtk_button_set_relief(GTK_BUTTON(play_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(play_event_box, _("Play"));
    gtk_widget_set_can_focus(play_event_box, FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), play_event_box, FALSE, FALSE, 0);
    gtk_widget_show(play_event_box);

    stop_event_box = w_gtk_button_new (NULL, icon_stop, stop_callback, NULL);
    gtk_button_set_relief(GTK_BUTTON(stop_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(stop_event_box, _("Stop"));
    gtk_widget_set_can_focus(stop_event_box, FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), stop_event_box, FALSE, FALSE, 0);
    gtk_widget_show(stop_event_box);
    gtk_widget_set_sensitive(stop_event_box, FALSE);

    ff_event_box = w_gtk_button_new (NULL, icon_ff, ff_callback, NULL);
    gtk_button_set_relief(GTK_BUTTON(ff_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(ff_event_box, _("Fast Forward"));
    gtk_widget_set_can_focus(ff_event_box, FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), ff_event_box, FALSE, FALSE, 0);
    gtk_widget_show(ff_event_box);
    gtk_widget_set_sensitive(ff_event_box, FALSE);

    next_event_box = w_gtk_button_new (NULL, icon_next, next_callback, NULL);
    gtk_button_set_relief(GTK_BUTTON(next_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(next_event_box, _("Next"));
    gtk_widget_set_can_focus(next_event_box, FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), next_event_box, FALSE, FALSE, 0);
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
    fs_event_box = w_gtk_button_new (NULL, NULL, fs_callback, NULL);
    gtk_button_set_image(GTK_BUTTON(fs_event_box), image_fs);
    gtk_button_set_relief(GTK_BUTTON(fs_event_box), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text(fs_event_box, _("Full Screen"));
    gtk_widget_set_can_focus(fs_event_box, FALSE);
    gtk_box_pack_end(GTK_BOX(hbox), fs_event_box, FALSE, FALSE, 0);
    gtk_widget_show(fs_event_box);
    // volume control
    if ((window_y > window_x)
        && (rpcontrols != NULL && g_ascii_strcasecmp(rpcontrols, "volumeslider") == 0)) {
        vol_slider = gtk_scale_new_with_range (GTK_ORIENTATION_VERTICAL, 0.0, 1.0, 0.20);
        gtk_widget_set_size_request(vol_slider, -1, window_y);
        gtk_range_set_inverted(GTK_RANGE(vol_slider), TRUE);
    } else {
        vol_slider = gtk_volume_button_new();
#if GTK_MAJOR_VERSION >= 3
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
        w_gtk_image_set_from_icon_name (GTK_IMAGE(image_fs), icon_nofs, button_size);
        if (gtk_widget_get_parent(GTK_WIDGET(hbox)) == controls_box) {
            g_object_ref(hbox);
            gtk_container_remove(GTK_CONTAINER(controls_box), hbox);
            gtk_container_add(GTK_CONTAINER(fs_controls), hbox);
            g_object_unref(hbox);
        }
        gtk_widget_show(fs_controls);
        gtk_widget_set_opacity (GTK_WIDGET(fs_controls), 0.75);

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
    Wg_mutex_lock(&fs_controls_lock);
    if (fs_controls != NULL) {
        w_gtk_image_set_from_icon_name (GTK_IMAGE(image_fs), icon_fs, button_size);
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
