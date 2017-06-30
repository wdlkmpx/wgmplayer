/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gmtk_media_player.c
 * Copyright (C) Kevin DeKorte 2009 <kdekorte@gmail.com>
 *
 * gmtk_media_player.c is free software.
 *
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * gmtk_media_tracker.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with playlist.c.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "gmtk_media_player.h"
#include "gmlib.h"
#include "gmtk_common.h"

G_DEFINE_TYPE(GmtkMediaPlayer, gmtk_media_player, GTK_TYPE_EVENT_BOX);
static GObjectClass *parent_class = NULL;

static void gmtk_media_player_dispose(GObject * object);
static gboolean gmtk_media_player_draw_event(GtkWidget * widget, cairo_t * cr);
static gboolean gmtk_media_player_expose_event(GtkWidget * widget, GdkEventExpose * event);
static void gmtk_media_player_size_allocate(GtkWidget * widget, GtkAllocation * allocation);
static gboolean player_key_press_event_callback(GtkWidget * widget, GdkEventKey * event, gpointer data);
static gboolean player_button_press_event_callback(GtkWidget * widget, GdkEventButton * event, gpointer data);
static gboolean player_motion_notify_event_callback(GtkWidget * widget, GdkEventMotion * event, gpointer data);
static void gmtk_media_player_restart_complete_callback(GmtkMediaPlayer * player, gpointer data);
static void gmtk_media_player_restart_shutdown_complete_callback(GmtkMediaPlayer * player, gpointer data);

// monitoring functions
gpointer launch_mplayer(gpointer data);
gboolean thread_reader_error(GIOChannel * source, GIOCondition condition, gpointer data);
gboolean thread_reader(GIOChannel * source, GIOCondition condition, gpointer data);
gboolean thread_query(gpointer data);
gboolean thread_complete(GIOChannel * source, GIOCondition condition, gpointer data);

gboolean write_to_mplayer(GmtkMediaPlayer * player, const gchar * cmd);
gboolean detect_mplayer_features(GmtkMediaPlayer * player);

static void player_realized(GtkWidget * widget, gpointer data)
{
    GtkStyle *style;

    style = gtk_widget_get_style(widget);
    gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &(style->black));
    gtk_widget_modify_bg(widget, GTK_STATE_ACTIVE, &(style->black));
    gtk_widget_modify_bg(widget, GTK_STATE_SELECTED, &(style->black));
    gtk_widget_modify_bg(widget, GTK_STATE_PRELIGHT, &(style->black));
    gtk_widget_modify_bg(widget, GTK_STATE_INSENSITIVE, &(style->black));

}

static void alignment_realized(GtkWidget * widget, gpointer data)
{
    GtkStyle *style;

    style = gtk_widget_get_style(widget);
    gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &(style->black));
    gtk_widget_modify_bg(widget, GTK_STATE_ACTIVE, &(style->black));
    gtk_widget_modify_bg(widget, GTK_STATE_SELECTED, &(style->black));
    gtk_widget_modify_bg(widget, GTK_STATE_PRELIGHT, &(style->black));
    gtk_widget_modify_bg(widget, GTK_STATE_INSENSITIVE, &(style->black));

}

static gboolean vodesc_looks_like_vo(gchar const *const desc, gchar const *const vo)
{
    if (g_strcmp0(desc, vo) == 0) {
        return TRUE;
    }
    if (!g_str_has_prefix(desc, vo)) {
        return FALSE;
    }
    /* we know that they are not equal but desc starts with vo, i.e. desc is longer than vo */
    /* now to check that desc looks like vo: */
    const size_t vol = strlen(vo);
    if (desc[vol] == ':') {
        return TRUE;
    }
    return FALSE;
}

static void socket_realized(GtkWidget * widget, gpointer data)
{
    GmtkMediaPlayer *player = GMTK_MEDIA_PLAYER(data);
    GtkStyle *style;

    player->socket_id = GPOINTER_TO_INT(gtk_socket_get_id(GTK_SOCKET(widget)));
    style = gtk_widget_get_style(widget);
    if (player->vo != NULL) {
        if (!vodesc_looks_like_vo(player->vo, "vdpau")) {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &(style->black));
            gtk_widget_modify_bg(widget, GTK_STATE_ACTIVE, &(style->black));
            gtk_widget_modify_bg(widget, GTK_STATE_SELECTED, &(style->black));
            gtk_widget_modify_bg(widget, GTK_STATE_PRELIGHT, &(style->black));
            gtk_widget_modify_bg(widget, GTK_STATE_INSENSITIVE, &(style->black));
        } else {
            gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, NULL);
            gtk_widget_modify_bg(widget, GTK_STATE_ACTIVE, NULL);
            gtk_widget_modify_bg(widget, GTK_STATE_SELECTED, NULL);
            gtk_widget_modify_bg(widget, GTK_STATE_PRELIGHT, NULL);
            gtk_widget_modify_bg(widget, GTK_STATE_INSENSITIVE, NULL);
        }
    } else {
        gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &(style->black));
        gtk_widget_modify_bg(widget, GTK_STATE_ACTIVE, &(style->black));
        gtk_widget_modify_bg(widget, GTK_STATE_SELECTED, &(style->black));
        gtk_widget_modify_bg(widget, GTK_STATE_PRELIGHT, &(style->black));
        gtk_widget_modify_bg(widget, GTK_STATE_INSENSITIVE, &(style->black));
    }

}


// use g_idle_add to emit the signals outside of the thread, makes them be emitted in the main loop
// this is done so that thread_enter/thread_leave is not needed
gboolean signal_event(gpointer data)
{
    GmtkMediaPlayerEvent *event = (GmtkMediaPlayerEvent *) data;

    if (event && event->event_name != NULL
        && (event->player->restart == FALSE || event->event_data_int == ATTRIBUTE_AF_EXPORT_FILENAME)) {

        switch (event->type) {
        case EVENT_TYPE_INT:
            g_signal_emit_by_name(event->player, event->event_name, event->event_data_int);
            break;

        case EVENT_TYPE_DOUBLE:
            g_signal_emit_by_name(event->player, event->event_name, event->event_data_double);
            break;

        case EVENT_TYPE_BOOLEAN:
            g_signal_emit_by_name(event->player, event->event_name, event->event_data_boolean);
            break;

        case EVENT_TYPE_ALLOCATION:
            if (gtk_widget_get_visible(GTK_WIDGET(event->player))) {
                if (!(event->event_allocation->width >= 65535 || event->event_allocation->height >= 65535))
                    g_signal_emit_by_name(event->player, event->event_name, event->event_allocation);
            }
            g_free(event->event_allocation);
            event->event_allocation = NULL;
            break;

        case EVENT_TYPE_STRING:
            g_signal_emit_by_name(event->player, event->event_name, event->event_data_string);
            g_free(event->event_data_string);
            event->event_data_string = NULL;
            break;

        default:
            gm_log(event->player->debug, G_LOG_LEVEL_MESSAGE, "undefined event %s", event->event_name);
        }
        g_free(event->event_name);
        event->event_name = NULL;
    }
    if (event)
        g_free(event);

    return FALSE;
}

// build the events we are going to signal
void create_event_int(GmtkMediaPlayer * player, const gchar * name, gint value)
{
    GmtkMediaPlayerEvent *event;

    event = g_new0(GmtkMediaPlayerEvent, 1);
    event->player = player;
    event->type = EVENT_TYPE_INT;
    event->event_name = g_strdup(name);
    event->event_data_int = value;
    g_idle_add(signal_event, event);
}

void create_event_double(GmtkMediaPlayer * player, const gchar * name, gdouble value)
{
    GmtkMediaPlayerEvent *event;

    event = g_new0(GmtkMediaPlayerEvent, 1);
    event->player = player;
    event->type = EVENT_TYPE_DOUBLE;
    event->event_name = g_strdup(name);
    event->event_data_double = value;
    g_idle_add(signal_event, event);
}

void create_event_boolean(GmtkMediaPlayer * player, const gchar * name, gboolean value)
{
    GmtkMediaPlayerEvent *event;

    event = g_new0(GmtkMediaPlayerEvent, 1);
    event->player = player;
    event->type = EVENT_TYPE_BOOLEAN;
    event->event_name = g_strdup(name);
    event->event_data_boolean = value;
    g_idle_add(signal_event, event);
}

void create_event_allocation(GmtkMediaPlayer * player, const gchar * name, GtkAllocation * allocation)
{
    GmtkMediaPlayerEvent *event;

    event = g_new0(GmtkMediaPlayerEvent, 1);
    event->player = player;
    event->type = EVENT_TYPE_ALLOCATION;
    event->event_name = g_strdup(name);
    event->event_allocation = g_new0(GtkAllocation, 1);
    memcpy(event->event_allocation, allocation, sizeof(GtkAllocation));
    g_idle_add(signal_event, event);

}

void create_event_string(GmtkMediaPlayer * player, const gchar * name, gchar * string)
{
    GmtkMediaPlayerEvent *event;

    event = g_new0(GmtkMediaPlayerEvent, 1);
    event->player = player;
    event->type = EVENT_TYPE_STRING;
    event->event_name = g_strdup(name);
    event->event_data_string = g_strdup(string);
    g_idle_add(signal_event, event);

}

// helper function to assist retry/fallback
gchar *gmtk_media_player_switch_protocol(const gchar * uri, gchar * new_protocol)
{
    gchar *p;

    p = g_strrstr(uri, "://");

    if (p != NULL)
        return g_strdup_printf("%s%s", new_protocol, p);
    else
        return NULL;
}

const gchar *gmtk_media_state_to_string(const GmtkMediaPlayerMediaState media_state)
{
    switch (media_state) {
    case MEDIA_STATE_UNKNOWN:
        return "unknown";
    case MEDIA_STATE_PLAY:
        return "play";
    case MEDIA_STATE_PAUSE:
        return "pause";
    case MEDIA_STATE_STOP:
        return "stop";
    case MEDIA_STATE_QUIT:
        return "quit";
    case MEDIA_STATE_BUFFERING:
        return "buffering";
    default:
        return "???";
    }
}

static void gmtk_media_player_log_state(GmtkMediaPlayer * player, char const *const context)
{
// Gmtk_Media_Player_Log_State
#define GMPLS_LEN 1024
    gchar msg[GMPLS_LEN] = "";
    gchar *tmp;

    if (context != NULL && (*context) != '\0') {
        g_strlcat(msg, context, GMPLS_LEN);
        g_strlcat(msg, ": ", GMPLS_LEN);
    }
    tmp = g_strdup_printf("position=%.3f length=%.3f start_time=%.3f run_time=%.3f volume=%.2f", player->position,
                          player->length, player->start_time, player->run_time, player->volume);
    g_strlcat(msg, tmp, GMPLS_LEN);
    g_free(tmp);

    if (player->muted) {
        g_strlcat(msg, " muted", GMPLS_LEN);
    }

    g_strlcat(msg, " player=", GMPLS_LEN);
    switch (player->player_state) {
    case PLAYER_STATE_DEAD:
        g_strlcat(msg, "dead", GMPLS_LEN);
        break;
    case PLAYER_STATE_RUNNING:
        g_strlcat(msg, "running", GMPLS_LEN);
        break;
    default:
        g_strlcat(msg, "???", GMPLS_LEN);
    }

    g_strlcat(msg, " media=", GMPLS_LEN);
    g_strlcat(msg, gmtk_media_state_to_string(player->media_state), GMPLS_LEN);
    g_strlcat(msg, " uri=", GMPLS_LEN);
    if (player->uri != NULL) {
        g_strlcat(msg, player->uri, GMPLS_LEN);
    }
    gm_log(player->debug, G_LOG_LEVEL_INFO, "%s", msg);
}

static void gmtk_media_player_class_init(GmtkMediaPlayerClass * class)
{
    GtkWidgetClass *widget_class;
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS(class);
    widget_class = GTK_WIDGET_CLASS(class);

    parent_class = g_type_class_peek_parent(class);
    G_OBJECT_CLASS(class)->dispose = gmtk_media_player_dispose;
#ifdef GTK3_ENABLED
    widget_class->draw = gmtk_media_player_draw_event;
#else
    widget_class->expose_event = gmtk_media_player_expose_event;
#endif
    widget_class->size_allocate = gmtk_media_player_size_allocate;

    g_signal_new("position-changed",
                 G_OBJECT_CLASS_TYPE(object_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GmtkMediaPlayerClass, position_changed),
                 NULL, NULL, g_cclosure_marshal_VOID__DOUBLE, G_TYPE_NONE, 1, G_TYPE_DOUBLE);

    g_signal_new("cache-percent-changed",
                 G_OBJECT_CLASS_TYPE(object_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GmtkMediaPlayerClass, cache_percent_changed),
                 NULL, NULL, g_cclosure_marshal_VOID__DOUBLE, G_TYPE_NONE, 1, G_TYPE_DOUBLE);

    g_signal_new("attribute-changed",
                 G_OBJECT_CLASS_TYPE(object_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GmtkMediaPlayerClass, attribute_changed),
                 NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

    g_signal_new("player-state-changed",
                 G_OBJECT_CLASS_TYPE(object_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GmtkMediaPlayerClass, player_state_changed),
                 NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

    g_signal_new("media-state-changed",
                 G_OBJECT_CLASS_TYPE(object_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GmtkMediaPlayerClass, media_state_changed),
                 NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

    g_signal_new("subtitles-changed",
                 G_OBJECT_CLASS_TYPE(object_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GmtkMediaPlayerClass, subtitles_changed),
                 NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

    g_signal_new("audio-tracks-changed",
                 G_OBJECT_CLASS_TYPE(object_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GmtkMediaPlayerClass, audio_tracks_changed),
                 NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

    g_signal_new("restart-shutdown-complete",
                 G_OBJECT_CLASS_TYPE(object_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GmtkMediaPlayerClass, restart_shutdown_complete),
                 NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    g_signal_new("restart-complete",
                 G_OBJECT_CLASS_TYPE(object_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GmtkMediaPlayerClass, restart_complete),
                 NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    g_signal_new("error-message",
                 G_OBJECT_CLASS_TYPE(object_class),
                 G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                 G_STRUCT_OFFSET(GmtkMediaPlayerClass, error_message),
                 NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

}

static void gmtk_media_player_init(GmtkMediaPlayer * player)
{
    GtkStyle *style;

    // player is an GtkEventBox that holds a GtkAlignment that holds a GtkSocket
    // mplayer uses the xwindow id from the socket to display media

    gtk_widget_add_events(GTK_WIDGET(player),
                          GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
                          GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                          GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_ENTER_NOTIFY_MASK | GDK_SCROLL_MASK);
#ifdef GTK3_ENABLED
    gtk_widget_set_app_paintable(GTK_WIDGET(player), TRUE);
#endif

    g_signal_connect(player, "key_press_event", G_CALLBACK(player_key_press_event_callback), NULL);
    g_signal_connect(player, "motion_notify_event", G_CALLBACK(player_motion_notify_event_callback), NULL);
    g_signal_connect(player, "button_press_event", G_CALLBACK(player_button_press_event_callback), NULL);
    gtk_widget_push_composite_child();

    player->alignment = gtk_alignment_new(0.0, 0.0, 1.0, 1.0);
    player->socket = gtk_socket_new();
    g_signal_connect(G_OBJECT(player), "realize", G_CALLBACK(player_realized), NULL);
    g_signal_connect(G_OBJECT(player->alignment), "realize", G_CALLBACK(alignment_realized), NULL);
    g_signal_connect(G_OBJECT(player->socket), "realize", G_CALLBACK(socket_realized), player);
    gtk_container_add(GTK_CONTAINER(player), player->alignment);
    gtk_container_add(GTK_CONTAINER(player->alignment), player->socket);
#ifdef GTK2_18_ENABLED
    gtk_widget_set_has_window(GTK_WIDGET(player->socket), TRUE);
    gtk_widget_set_can_focus(GTK_WIDGET(player->socket), TRUE);
    gtk_widget_set_can_default(GTK_WIDGET(player->socket), TRUE);
#else
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(player->socket), GTK_CAN_FOCUS);
    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(player->socket), GTK_CAN_DEFAULT);
#endif
    gtk_widget_activate(GTK_WIDGET(player->socket));

    g_signal_connect(player->socket, "key_press_event", G_CALLBACK(player_key_press_event_callback), player);

    g_signal_connect(player, "restart-shutdown-complete",
                     G_CALLBACK(gmtk_media_player_restart_shutdown_complete_callback), NULL);
    g_signal_connect(player, "restart-complete", G_CALLBACK(gmtk_media_player_restart_complete_callback), NULL);
    gtk_widget_pop_composite_child();

    gtk_widget_show_all(GTK_WIDGET(player));
    style = gtk_widget_get_style(GTK_WIDGET(player));
    player->default_background = gdk_color_copy(&(style->bg[GTK_STATE_NORMAL]));
    player->player_state = PLAYER_STATE_DEAD;
    player->media_state = MEDIA_STATE_UNKNOWN;
    player->uri = NULL;
    player->message = NULL;
    player->mplayer_thread = NULL;
    player->aspect_ratio = ASPECT_DEFAULT;
    player->mplayer_complete_cond = g_cond_new();
    player->thread_running = g_mutex_new();
    player->video_width = 0;
    player->video_height = 0;
    player->video_present = FALSE;
    player->media_device = NULL;
    player->type = TYPE_FILE;
    player->title_is_menu = FALSE;
    player->start_time = 0.0;
    player->run_time = 0.0;
    player->vo = NULL;
    player->ao = NULL;
    player->cache_size = 0;
    player->subtitles = NULL;
    player->audio_tracks = NULL;
    player->af_export_filename = gm_tempname(NULL, "mplayer-af_exportXXXXXX");
    player->brightness = 0;
    player->contrast = 0;
    player->gamma = 0;
    player->hue = 0;
    player->osdlevel = 0;
    player->saturation = 0;
    player->restart = FALSE;
    player->video_format = NULL;
    player->video_codec = NULL;
    player->audio_format = NULL;
    player->audio_codec = NULL;
    player->disable_upscaling = FALSE;
    player->mplayer_binary = NULL;
    player->extra_opts = NULL;
    player->use_mplayer2 = FALSE;
    player->features_detected = FALSE;
    player->zoom = 1.0;
    player->speed_multiplier = 1.0;
    player->subtitle_scale = 1.0;
    player->subtitle_delay = 0.0;
    player->subtitle_position = 0;
    player->subtitle_fuzziness = 0;
    player->audio_delay = 0.0;
    player->restart = FALSE;
    player->debug = 1;
    player->channel_in = NULL;
    player->channel_out = NULL;
    player->channel_err = NULL;
    player->retry_on_full_cache = FALSE;
    player->profile = NULL;
    player->alang = NULL;
    player->slang = NULL;
    player->artist = NULL;
    player->title = NULL;
    player->album = NULL;
    player->disposed = FALSE;
    player->player_lock = g_mutex_new();
    player->name_regex = g_regex_new(".*name\\s*:\\s*(.*)\n", G_REGEX_CASELESS, 0, NULL);
    player->genre_regex = g_regex_new(".*genre\\s*:\\s*(.*)\n", G_REGEX_CASELESS, 0, NULL);
    player->title_regex = g_regex_new(".*title\\s*:\\s*(.*)\n", G_REGEX_CASELESS, 0, NULL);
    player->artist_regex = g_regex_new(".*artist\\s*:\\s*(.*)\n", G_REGEX_CASELESS, 0, NULL);
    player->album_regex = g_regex_new(".*album\\s*:\\s*(.*)\n", G_REGEX_CASELESS, 0, NULL);
    player->deintN_regex = g_regex_new("(.*)(:deint=[0-9]+)\\b(.*)", G_REGEX_CASELESS, 0, NULL);

    gmtk_media_player_log_state(player, "after init");
}

static void gmtk_media_player_dispose(GObject * object)
{

    GmtkMediaPlayer *player;

    if (object == NULL) {
        return;
    }

    player = GMTK_MEDIA_PLAYER(object);

    if (player->disposed) {
        return;
    }
    player->disposed = TRUE;

    gm_log(player->debug, G_LOG_LEVEL_DEBUG, "gmtk_media_player_dispose");

    // cleanup the memory used

    if (player->uri != NULL) {
        g_free(player->uri);
        player->uri = NULL;
    }
    if (player->media_device != NULL) {
        g_free(player->media_device);
        player->media_device = NULL;
    }
    if (player->vo != NULL) {
        g_free(player->vo);
        player->vo = NULL;
    }
    if (player->ao != NULL) {
        g_free(player->ao);
        player->ao = NULL;
    }

    if (player->artist != NULL) {
        g_free(player->artist);
        player->artist = NULL;
    }

    if (player->title != NULL) {
        g_free(player->title);
        player->title = NULL;
    }

    if (player->message != NULL) {
        g_free(player->message);
        player->message = NULL;
    }

    if (player->extra_opts != NULL) {
        g_free(player->extra_opts);
        player->extra_opts = NULL;
    }

    if (player->video_format != NULL) {
        g_free(player->video_format);
        player->video_format = NULL;
    }

    if (player->video_codec != NULL) {
        g_free(player->video_codec);
        player->video_codec = NULL;
    }

    if (player->audio_format != NULL) {
        g_free(player->audio_format);
        player->audio_format = NULL;
    }

    if (player->audio_codec != NULL) {
        g_free(player->audio_codec);
        player->audio_codec = NULL;
    }

    if (player->af_export_filename != NULL) {
        g_free(player->af_export_filename);
        player->af_export_filename = NULL;
    }

    if (player->subtitle_font != NULL) {
        g_free(player->subtitle_font);
        player->subtitle_font = NULL;
    }

    if (player->subtitle_color != NULL) {
        g_free(player->subtitle_color);
        player->subtitle_color = NULL;
    }

    if (player->profile != NULL) {
        g_free(player->profile);
        player->profile = NULL;
    }

    if (player->slang != NULL) {
        g_free(player->slang);
        player->slang = NULL;
    }

    if (player->alang != NULL) {
        g_free(player->alang);
        player->alang = NULL;
    }

    gdk_color_free(player->default_background);

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static gboolean gmtk_media_player_draw_event(GtkWidget * widget, cairo_t * cr)
{
    GtkStyle *style;
    GtkAllocation allocation;

    style = gtk_widget_get_style(widget);
    gmtk_get_allocation(widget, &allocation);

    cairo_set_source_rgb(cr, style->black.red / 65535.0, style->black.green / 65535.0, style->black.blue / 65535.0);
    cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
    cairo_fill(cr);
    cairo_stroke(cr);

    return FALSE;
}

static gboolean gmtk_media_player_expose_event(GtkWidget * widget, GdkEventExpose * event)
{
    return FALSE;
}

gboolean gmtk_media_player_send_key_press_event(GmtkMediaPlayer * widget, GdkEventKey * event, gpointer data)
{
    return player_key_press_event_callback(GTK_WIDGET(widget), event, data);
}

/*
  sub_select [value]
    Display subtitle with index [value]. Turn subtitle display off if
    [value] is -1 or greater than the highest available subtitle index.
    Cycle through the available subtitles if [value] is omitted or less
    than -1 (forward or backward respectively).
    Supported subtitle sources are -sub options on the command
    line, VOBsubs, DVD subtitles, and Ogg and Matroska text streams.
    This command is mainly for cycling all subtitles, if you want to set
    a specific subtitle, use sub_file, sub_vob, or sub_demux.

    GmtkMediaPlayerSubtitle

    count: (gdouble) g_list_length(player->subtitles)

    iter = player->subtitles;
    while (iter) {
        subtitle = (GmtkMediaPlayerSubtitle *) iter->data;
        if (subtitle->id == player->subtitle_id && subtitle->is_file == player->subtitle_is_file)
            value = subtitle->label;
        iter = iter->next;
    }

    sub_visibility [0|1]

    properties: sub
       get_property <property>
       set_property <property> <value>

*/
static void gmtk_media_player_cycle_subtitles(GmtkMediaPlayer * player)
{
    write_to_mplayer(player, "sub_select\n");
}

static gboolean player_key_press_event_callback(GtkWidget * widget, GdkEventKey * event, gpointer data)
{
    GmtkMediaPlayer *player;
    GtkAllocation alloc;
    gchar *cmd;

    if (data != NULL) {
        player = GMTK_MEDIA_PLAYER(data);
    } else {
        player = GMTK_MEDIA_PLAYER(widget);
    }

    if (event->is_modifier)
        return TRUE;

    if ((event->state & GDK_CONTROL_MASK) == 0 && (event->state & GDK_MOD1_MASK) == 0) {
        switch (event->keyval) {
        case GDK_Right:
            if (player->title_is_menu) {
                write_to_mplayer(player, "dvdnav 4\n");
            }
            break;
        case GDK_Left:
            if (player->title_is_menu) {
                write_to_mplayer(player, "dvdnav 3\n");
            }
            break;
        case GDK_Up:
            if (player->title_is_menu) {
                write_to_mplayer(player, "dvdnav 1\n");
            }
            break;
        case GDK_Down:
            if (player->title_is_menu) {
                write_to_mplayer(player, "dvdnav 2\n");
            }
            break;
        case GDK_Return:
            if (player->title_is_menu) {
                write_to_mplayer(player, "dvdnav 6\n");
            }
            return TRUE;
            break;
        case GDK_Home:
            if (!player->title_is_menu) {
                write_to_mplayer(player, "dvdnav menu\n");
            }
            return TRUE;
            break;
        case GDK_space:
        case GDK_p:
            switch (player->media_state) {
            case MEDIA_STATE_PAUSE:
                gmtk_media_player_set_state(player, MEDIA_STATE_PLAY);
                break;
            case MEDIA_STATE_PLAY:
                gmtk_media_player_set_state(player, MEDIA_STATE_PAUSE);
                break;
            default:
                break;
            }
            break;
        case GDK_1:
            gmtk_media_player_set_attribute_integer_delta(player, ATTRIBUTE_CONTRAST, -5);
            break;
        case GDK_2:
            gmtk_media_player_set_attribute_integer_delta(player, ATTRIBUTE_CONTRAST, 5);
            break;
        case GDK_3:
            gmtk_media_player_set_attribute_integer_delta(player, ATTRIBUTE_BRIGHTNESS, -5);
            break;
        case GDK_4:
            gmtk_media_player_set_attribute_integer_delta(player, ATTRIBUTE_BRIGHTNESS, 5);
            break;
        case GDK_5:
            gmtk_media_player_set_attribute_integer_delta(player, ATTRIBUTE_HUE, -5);
            break;
        case GDK_6:
            gmtk_media_player_set_attribute_integer_delta(player, ATTRIBUTE_HUE, 5);
            break;
        case GDK_7:
            gmtk_media_player_set_attribute_integer_delta(player, ATTRIBUTE_SATURATION, -5);
            break;
        case GDK_8:
            gmtk_media_player_set_attribute_integer_delta(player, ATTRIBUTE_SATURATION, 5);
            break;
        case GDK_plus:
            write_to_mplayer(player, "audio_delay 0.1 0\n");
            break;
        case GDK_minus:
            write_to_mplayer(player, "audio_delay -0.1 0\n");
            break;
        case GDK_numbersign:
            write_to_mplayer(player, "switch_audio -1\n");
            return TRUE;
            break;
        case GDK_period:
            if (player->media_state == MEDIA_STATE_PAUSE)
                write_to_mplayer(player, "frame_step\n");
            break;
        case GDK_KP_Add:
            player->zoom += 0.10;
            player->zoom = CLAMP(player->zoom, 0.1, 10.0);
            gmtk_get_allocation(GTK_WIDGET(player), &alloc);
            gmtk_media_player_size_allocate(GTK_WIDGET(player), &alloc);
            break;
        case GDK_KP_Subtract:
            player->zoom -= 0.10;
            player->zoom = CLAMP(player->zoom, 0.1, 10.0);
            gmtk_get_allocation(GTK_WIDGET(player), &alloc);
            gmtk_media_player_size_allocate(GTK_WIDGET(player), &alloc);
            break;
        case GDK_KP_Enter:
            player->zoom = 1.0;
            gmtk_get_allocation(GTK_WIDGET(player), &alloc);
            gmtk_media_player_size_allocate(GTK_WIDGET(player), &alloc);
            break;
        case GDK_j:
            gmtk_media_player_cycle_subtitles(player);
            break;
        case GDK_d:
            write_to_mplayer(player, "frame_drop\n");
            cmd =
                g_strdup_printf("osd_show_property_text \"%s: ${framedropping}\" 1000 1\n",
                                g_dgettext(GETTEXT_PACKAGE, "Frame Dropping"));
            write_to_mplayer(player, cmd);
            g_free(cmd);
            break;
        case GDK_b:
            write_to_mplayer(player, "sub_pos -1 0\n");
            break;
        case GDK_B:
            write_to_mplayer(player, "sub_pos 1 0\n");
            break;
        case GDK_D:
            write_to_mplayer(player, "step_property deinterlace\n");
            cmd =
                g_strdup_printf("osd_show_property_text \"%s: ${deinterlace}\" 1000 1\n",
                                g_dgettext(GETTEXT_PACKAGE, "Deinterlace"));
            write_to_mplayer(player, cmd);
            g_free(cmd);
            break;
        case GDK_s:
        case GDK_S:
            write_to_mplayer(player, "screenshot 0\n");
            break;
        case GDK_x:
            write_to_mplayer(player, "sub_delay 0.1\n");
            break;
        case GDK_z:
            write_to_mplayer(player, "sub_delay -0.1\n");
            break;
        case GDK_o:
            write_to_mplayer(player, "osd\n");
            break;
		case GDK_h:
			write_to_mplayer(player, "tv_step_channel -1\n");
			break;
		case GDK_k:
			write_to_mplayer(player, "tv_step_channel 1\n");
			break;
        default:
            gm_log(player->debug, G_LOG_LEVEL_INFO, "ignoring key %s%s%s%s",
                   (event->state & GDK_CONTROL_MASK) ? "Control-" : "", (event->state & GDK_MOD1_MASK) ? "Alt-" : "",
                   (event->state & GDK_SHIFT_MASK) ? "Shift-" : "", gdk_keyval_name(event->keyval));
        }

    }
    return FALSE;
}

static gboolean player_button_press_event_callback(GtkWidget * widget, GdkEventButton * event, gpointer data)
{
    GmtkMediaPlayer *player = GMTK_MEDIA_PLAYER(widget);
    gchar *cmd;

    if (event->button == 1) {
        if (player->title_is_menu) {
            cmd = g_strdup_printf("dvdnav mouse\n");
            write_to_mplayer(player, cmd);
            g_free(cmd);
        }
    }


    return FALSE;
}

static gboolean player_motion_notify_event_callback(GtkWidget * widget, GdkEventMotion * event, gpointer data)
{
    GmtkMediaPlayer *player = GMTK_MEDIA_PLAYER(widget);
    gchar *cmd;
    gint x, y;
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 4
    GdkDeviceManager *device_manager;
    GdkDevice *pointer;
#endif

    if (player->title_is_menu) {
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 4
        device_manager = gdk_display_get_device_manager(gtk_widget_get_display(widget));
        pointer = gdk_device_manager_get_client_pointer(device_manager);
        gdk_window_get_device_position(gtk_widget_get_window(widget), pointer, &x, &y, NULL);
#else
        gtk_widget_get_pointer(player->socket, &x, &y);
#endif
        cmd = g_strdup_printf("set_mouse_pos %i %i\n", x, y);
        write_to_mplayer(player, cmd);
        g_free(cmd);
    }

    return FALSE;

}

static void gmtk_media_player_size_allocate(GtkWidget * widget, GtkAllocation * allocation)
{
    GmtkMediaPlayer *player = GMTK_MEDIA_PLAYER(widget);
    gdouble video_aspect;
    gdouble widget_aspect;
    gfloat xscale, yscale;

    if (allocation->width <= 0 || allocation->height <= 0) {
        gmtk_get_allocation(widget, allocation);
        gm_log(player->debug, G_LOG_LEVEL_DEBUG, "widget allocation %i x %i", allocation->width, allocation->height);
    }
    // protect against possible divide by zero
    if (allocation->width == 0 || allocation->height == 0) {
        return;
    }

    if (player->video_width == 0 || player->video_height == 0 || !gmtk_widget_get_realized(widget)) {
        gtk_alignment_set(GTK_ALIGNMENT(player->alignment), 0.0, 0.0, 1.0, 1.0);
    } else {
        switch (player->aspect_ratio) {
        case ASPECT_4X3:
            video_aspect = 4.0 / 3.0;
            break;
        case ASPECT_16X9:
            video_aspect = 16.0 / 9.0;
            break;
        case ASPECT_16X10:
            video_aspect = 16.0 / 10.0;
            break;
        case ASPECT_WINDOW:
            video_aspect = (gdouble) allocation->width / (gdouble) allocation->height;
            break;
        case ASPECT_ANAMORPHIC:
            video_aspect = 2.39;
            break;
        case ASPECT_DEFAULT:
        default:
            video_aspect = (gdouble) player->video_width / (gdouble) player->video_height;
            break;
        }

        widget_aspect = (gdouble) allocation->width / (gdouble) allocation->height;

        if (player->disable_upscaling && allocation->width > player->video_width
            && allocation->height > player->video_height) {

            gtk_alignment_set(GTK_ALIGNMENT(player->alignment), 0.5, 0.5,
                              CLAMP((gdouble) player->video_width / (gdouble) allocation->width, 0.1, 1.0),
                              CLAMP((gdouble) player->video_height / (gdouble) allocation->height, 0.1, 1.0));

        } else {
            if (video_aspect > widget_aspect) {
                yscale = ((gdouble) allocation->width / video_aspect) / (gdouble) allocation->height;

                gm_log(player->debug, G_LOG_LEVEL_DEBUG, "yscale = %lf", yscale);
                if (yscale > 0.0) {
                    gtk_alignment_set(GTK_ALIGNMENT(player->alignment), 0.0, 0.5, 1.0, CLAMP(yscale, 0.1, 1.0));
                } else {
                    gtk_alignment_set(GTK_ALIGNMENT(player->alignment), 0.0, 0.5, 1.0, 1.0);
                }
            } else {
                xscale = ((gdouble) allocation->height * video_aspect) / (gdouble) allocation->width;

                gm_log(player->debug, G_LOG_LEVEL_DEBUG, "xscale = %lf", xscale);
                if (xscale > 0.0) {
                    gtk_alignment_set(GTK_ALIGNMENT(player->alignment), 0.5, 0.0, CLAMP(xscale, 0.1, 1.0), 1.0);
                } else {
                    gtk_alignment_set(GTK_ALIGNMENT(player->alignment), 0.5, 0.0, 1.0, 1.0);
                }
            }
        }
    }


    gm_log(player->debug, G_LOG_LEVEL_DEBUG, "gmtk allocation video:%s %ix%i", gm_bool_to_string(player->video_present),
           allocation->width, allocation->height);
    GTK_WIDGET_CLASS(parent_class)->size_allocate(widget, allocation);

}

GtkWidget *gmtk_media_player_new()
{
    GtkWidget *player = g_object_new(GMTK_TYPE_MEDIA_PLAYER, NULL);

    return player;
}

static void gmtk_media_player_restart_complete_callback(GmtkMediaPlayer * player, gpointer data)
{
    gmtk_media_player_seek(player, player->restart_position, SEEK_ABSOLUTE);
    player->restart = FALSE;
    gm_log(player->debug, G_LOG_LEVEL_DEBUG, "restart state = %s, current state = %s",
           gmtk_media_state_to_string(player->restart_state),
           gmtk_media_state_to_string(gmtk_media_player_get_media_state(player)));
    if (player->restart_state != gmtk_media_player_get_media_state(player))
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(player), player->restart_state);
    gm_log(player->debug, G_LOG_LEVEL_INFO, "restart complete");
}

static void gmtk_media_player_restart_shutdown_complete_callback(GmtkMediaPlayer * player, gpointer data)
{
    if (player->restart) {
        if (player->restart_state != MEDIA_STATE_STOP) {
            gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(player), MEDIA_STATE_PLAY);
        } else {
            player->restart = FALSE;
        }
    }
}

void gmtk_media_player_restart(GmtkMediaPlayer * player)
{
    if (player->player_state == PLAYER_STATE_RUNNING) {
        player->restart = TRUE;
        player->restart_state = gmtk_media_player_get_media_state(player);
        gmtk_media_player_set_state(player, MEDIA_STATE_PAUSE);
        player->restart_position = player->position;
        gmtk_media_player_set_state(GMTK_MEDIA_PLAYER(player), MEDIA_STATE_QUIT);
    }
}

// this will take a new URI, but better to shut it down and start a new instance
void gmtk_media_player_set_uri(GmtkMediaPlayer * player, const gchar * uri)
{
    gchar *cmd;
    gchar *filename = NULL;

    player->uri = g_strdup(uri);
    player->video_width = 0;
    player->video_height = 0;
    player->length = 0;

    if (player->player_state == PLAYER_STATE_RUNNING) {
        if (player->uri != NULL) {
            filename = g_filename_from_uri(player->uri, NULL, NULL);
        }
        cmd = g_strdup_printf("loadfile \"%s\" 0\n", filename);
        write_to_mplayer(player, cmd);
        g_free(cmd);
        if (filename != NULL) {
            g_free(filename);
            filename = NULL;
        }
        if (player->media_state == MEDIA_STATE_STOP) {
            gmtk_media_player_set_state(player, MEDIA_STATE_PLAY);
        }
    }

}

const gchar *gmtk_media_player_get_uri(GmtkMediaPlayer * player)
{
    return player->uri;
}

void gmtk_media_player_set_state(GmtkMediaPlayer * player, const GmtkMediaPlayerMediaState new_media_state)
{
    gmtk_media_player_log_state(player, "old");
    gm_log(player->debug, G_LOG_LEVEL_DEBUG, "setting media state to %s", gmtk_media_state_to_string(new_media_state));

    if (player->player_state == PLAYER_STATE_DEAD) {

        if (new_media_state == MEDIA_STATE_QUIT) {
            player->media_state = MEDIA_STATE_UNKNOWN;
        }

        if (new_media_state == MEDIA_STATE_STOP) {
            player->media_state = MEDIA_STATE_UNKNOWN;
        }

        if (new_media_state == MEDIA_STATE_PAUSE) {
            player->media_state = MEDIA_STATE_UNKNOWN;
        }

        if (new_media_state == MEDIA_STATE_PLAY) {
            // launch player
            gm_log(player->debug, G_LOG_LEVEL_DEBUG, "launching launch_mplayer thread");
            player->mplayer_thread = g_thread_create(launch_mplayer, player, TRUE, NULL);
            if (player->mplayer_thread != NULL) {
                if (player->message != NULL) {
                    g_free(player->message);
                    player->message = NULL;
                }
                player->message = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "Loading..."));
                if (!player->restart)
                    g_signal_emit_by_name(player, "attribute-changed", ATTRIBUTE_MESSAGE);
                player->player_state = PLAYER_STATE_RUNNING;
                if (!player->restart)
                    g_signal_emit_by_name(player, "player-state-changed", player->player_state);
                gmtk_media_player_log_state(player, "new");
                return;
            }
        }

    }

    if (player->player_state == PLAYER_STATE_RUNNING) {
        if (new_media_state == MEDIA_STATE_STOP) {
            if (player->type == TYPE_NETWORK) {
                write_to_mplayer(player, "quit\n");
            } else {
                write_to_mplayer(player, "seek 0 2\n");
                if (player->media_state == MEDIA_STATE_PLAY) {
                    write_to_mplayer(player, "pause\n");
                }
                player->media_state = MEDIA_STATE_STOP;
                g_signal_emit_by_name(player, "position-changed", 0.0);
                if (!player->restart)
                    g_signal_emit_by_name(player, "media-state-changed", player->media_state);
            }
        }

        if (new_media_state == MEDIA_STATE_PLAY) {
            gtk_widget_show(GTK_WIDGET(player->socket));

            if (player->media_state == MEDIA_STATE_PAUSE || player->media_state == MEDIA_STATE_STOP) {
                write_to_mplayer(player, "pause\n");
                player->media_state = MEDIA_STATE_PLAY;
                if (!player->restart)
                    g_signal_emit_by_name(player, "media-state-changed", player->media_state);
            }
            if (player->media_state == MEDIA_STATE_UNKNOWN) {
                player->media_state = MEDIA_STATE_PLAY;
                if (!player->restart)
                    g_signal_emit_by_name(player, "media-state-changed", player->media_state);
            }
        }

        if (new_media_state == MEDIA_STATE_PAUSE) {
            if (player->media_state == MEDIA_STATE_PLAY) {
                write_to_mplayer(player, "pause\n");
                player->media_state = MEDIA_STATE_PAUSE;
                if (!player->restart)
                    g_signal_emit_by_name(player, "media-state-changed", player->media_state);
            }
        }

        if (new_media_state == MEDIA_STATE_QUIT) {
            write_to_mplayer(player, "quit\n");
        }
    }

    gmtk_media_player_log_state(player, "new");
}

GmtkMediaPlayerMediaState gmtk_media_player_get_media_state(GmtkMediaPlayer * player)
{
    return player->media_state;
}

void gmtk_media_player_send_command(GmtkMediaPlayer * player, GmtkMediaPlayerCommand command)
{
    gchar *cmd = NULL;

    if (player->player_state == PLAYER_STATE_RUNNING) {
        switch (command) {
        case COMMAND_SHOW_DVD_MENU:
            write_to_mplayer(player, "dvdnav 5\n");
            break;

        case COMMAND_TAKE_SCREENSHOT:
            write_to_mplayer(player, "screenshot 0\n");
            break;

        case COMMAND_SWITCH_ANGLE:
            write_to_mplayer(player, "switch_angle\n");
            break;

        case COMMAND_SWITCH_AUDIO:
            write_to_mplayer(player, "switch_audio\n");
            break;

        case COMMAND_FRAME_STEP:
            if (player->media_state == MEDIA_STATE_PAUSE)
                write_to_mplayer(player, "frame_step\n");
            break;

        case COMMAND_SUBTITLE_SELECT:
            gmtk_media_player_cycle_subtitles(player);
            break;

        case COMMAND_SUBTITLE_STEP_FORWARD:
            write_to_mplayer(player, "sub_step 1\n");
            break;

        case COMMAND_SUBTITLE_STEP_BACKWARD:
            write_to_mplayer(player, "sub_step -1\n");
            break;

        case COMMAND_SWITCH_FRAME_DROP:
            write_to_mplayer(player, "frame_drop\n");
            cmd =
                g_strdup_printf("osd_show_property_text \"%s ${framedropping}\" 1000 1\n",
                                g_dgettext(GETTEXT_PACKAGE, "Frame Dropping"));
            write_to_mplayer(player, cmd);
            g_free(cmd);
            cmd = NULL;
            break;

        default:
            gm_log(player->debug, G_LOG_LEVEL_INFO, "Unknown command");
        }
    }
}

void gmtk_media_player_set_attribute_boolean(GmtkMediaPlayer * player,
                                             GmtkMediaPlayerMediaAttributes attribute, gboolean value)
{
    gchar *cmd = NULL;

    switch (attribute) {
    case ATTRIBUTE_SUB_VISIBLE:
        player->sub_visible = value;
        if (player->player_state == PLAYER_STATE_RUNNING) {
            cmd = g_strdup_printf("set_property sub_visibility %i\n", value);
            write_to_mplayer(player, cmd);
            g_free(cmd);
            cmd = NULL;
            if (value) {
                cmd =
                    g_strdup_printf("osd_show_property_text \"%s\" 1000 1\n",
                                    g_dgettext(GETTEXT_PACKAGE, "Subtitles Visible"));
            } else {
                cmd =
                    g_strdup_printf("osd_show_property_text \"%s\" 1000 1\n",
                                    g_dgettext(GETTEXT_PACKAGE, "Subtitles Hidden"));
            }
            write_to_mplayer(player, cmd);
            g_free(cmd);
            cmd = NULL;
        }
        break;

    case ATTRIBUTE_ENABLE_FRAME_DROP:
        player->frame_drop = value;
        if (player->player_state == PLAYER_STATE_RUNNING) {
            cmd = g_strdup_printf("frame_drop %i\n", value);
            write_to_mplayer(player, cmd);
            g_free(cmd);
            cmd = NULL;
        }
        break;

    case ATTRIBUTE_PLAYLIST:
        player->playlist = value;
        break;

    case ATTRIBUTE_DISABLE_UPSCALING:
        player->disable_upscaling = value;
        break;

    case ATTRIBUTE_SOFTVOL:
        player->softvol = value;
        break;

    case ATTRIBUTE_MUTED:
        player->muted = value;
        if (player->player_state == PLAYER_STATE_RUNNING) {
            cmd = g_strdup_printf("mute %i\n", value);
            write_to_mplayer(player, cmd);
            g_free(cmd);
            cmd = NULL;
        }
        break;

    case ATTRIBUTE_ENABLE_ADVANCED_SUBTITLES:
        player->enable_advanced_subtitles = value;
        break;

    case ATTRIBUTE_ENABLE_EMBEDDED_FONTS:
        player->enable_embedded_fonts = value;
        break;

    case ATTRIBUTE_SUBTITLE_OUTLINE:
        player->subtitle_outline = value;
        break;

    case ATTRIBUTE_SUBTITLE_SHADOW:
        player->subtitle_shadow = value;
        break;

    case ATTRIBUTE_DEINTERLACE:
        player->deinterlace = value;
        break;

    case ATTRIBUTE_ENABLE_DEBUG:
        player->debug = value;
        break;

    case ATTRIBUTE_HARDWARE_AC3:
        player->hardware_ac3 = value;
        break;

    case ATTRIBUTE_ENABLE_HARDWARE_CODECS:
        player->enable_hardware_codecs = value;
        break;

    case ATTRIBUTE_ENABLE_CRYSTALHD_CODECS:
        player->enable_crystalhd_codecs = value;
        break;

    case ATTRIBUTE_FORCE_CACHE:
        player->force_cache = value;
        break;

    default:
        gm_log(player->debug, G_LOG_LEVEL_INFO, "Unsupported Attribute");
    }

    return;
}

gboolean gmtk_media_player_get_attribute_boolean(GmtkMediaPlayer * player, GmtkMediaPlayerMediaAttributes attribute)
{
    gboolean ret = FALSE;

    switch (attribute) {
    case ATTRIBUTE_SUB_VISIBLE:
        ret = player->sub_visible;
        break;

    case ATTRIBUTE_ENABLE_FRAME_DROP:
        ret = player->frame_drop;
        break;

    case ATTRIBUTE_SUBS_EXIST:
        ret = g_list_length(player->subtitles);
        break;

    case ATTRIBUTE_SOFTVOL:
        ret = player->softvol;
        break;

    case ATTRIBUTE_PLAYLIST:
        ret = player->playlist;
        break;

    case ATTRIBUTE_SEEKABLE:
        ret = player->seekable;
        break;

    case ATTRIBUTE_HAS_CHAPTERS:
        ret = player->has_chapters;
        break;

    case ATTRIBUTE_TITLE_IS_MENU:
        ret = player->title_is_menu;
        break;

    case ATTRIBUTE_DISABLE_UPSCALING:
        ret = player->disable_upscaling;
        break;

    case ATTRIBUTE_VIDEO_PRESENT:
        ret = player->video_present;
        break;

    case ATTRIBUTE_MUTED:
        ret = player->muted;
        break;

    case ATTRIBUTE_ENABLE_ADVANCED_SUBTITLES:
        ret = player->enable_advanced_subtitles;
        break;

    case ATTRIBUTE_ENABLE_EMBEDDED_FONTS:
        ret = player->enable_embedded_fonts;
        break;

    case ATTRIBUTE_SUBTITLE_OUTLINE:
        ret = player->subtitle_outline;
        break;

    case ATTRIBUTE_SUBTITLE_SHADOW:
        ret = player->subtitle_shadow;
        break;

    case ATTRIBUTE_DEINTERLACE:
        ret = player->deinterlace;
        break;

    case ATTRIBUTE_ENABLE_DEBUG:
        ret = player->debug;
        break;

    case ATTRIBUTE_HARDWARE_AC3:
        ret = player->hardware_ac3;
        break;

    case ATTRIBUTE_RETRY_ON_FULL_CACHE:
        ret = player->retry_on_full_cache;
        break;

    case ATTRIBUTE_ENABLE_HARDWARE_CODECS:
        ret = player->enable_hardware_codecs;
        break;

    case ATTRIBUTE_ENABLE_CRYSTALHD_CODECS:
        ret = player->enable_crystalhd_codecs;
        break;

    case ATTRIBUTE_FORCE_CACHE:
        ret = player->force_cache;
        break;

    default:
        gm_log(player->debug, G_LOG_LEVEL_INFO, "Unsupported Attribute");
    }
    return ret;
}

void gmtk_media_player_set_attribute_double(GmtkMediaPlayer * player,
                                            GmtkMediaPlayerMediaAttributes attribute, gdouble value)
{
    gchar *cmd;
    gchar *tmp;

    switch (attribute) {
    case ATTRIBUTE_CACHE_SIZE:
        player->cache_size = value;
        break;

    case ATTRIBUTE_START_TIME:
        player->start_time = value;
        break;

    case ATTRIBUTE_RUN_TIME:
        player->run_time = value;
        break;

    case ATTRIBUTE_ZOOM:
        player->zoom = CLAMP(value, 0.1, 10.0);
        break;

    case ATTRIBUTE_VOLUME_GAIN:
        player->volume_gain = CLAMP(value, -200.0, 60.0);
        break;

    case ATTRIBUTE_SPEED_MULTIPLIER:
        player->speed_multiplier = CLAMP(value, 0.1, 10.0);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            if (player->speed_multiplier == 1.0) {
                tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
                tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->speed_multiplier);
                cmd = g_strdup_printf("speed_set %s\n", tmp);
                g_free(tmp);

            } else {
                tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
                tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->speed_multiplier);
                cmd = g_strdup_printf("speed_mult %s\n", tmp);
                g_free(tmp);
            }
            write_to_mplayer(player, cmd);
            g_free(cmd);
            cmd = NULL;
            write_to_mplayer(player, "get_property speed\n");
        }
        break;

    case ATTRIBUTE_SPEED_SET:
        player->speed = CLAMP(value, 0.1, 10.0);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
            tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->speed);
            cmd = g_strdup_printf("speed_set %s\n", tmp);
            g_free(tmp);
            write_to_mplayer(player, cmd);
            g_free(cmd);
            cmd = NULL;
            write_to_mplayer(player, "get_property speed\n");
        }
        break;

    case ATTRIBUTE_SUBTITLE_SCALE:
        player->subtitle_scale = CLAMP(value, 0.2, 100.0);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
            tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->subtitle_scale);
            cmd = g_strdup_printf("sub_scale %s\n", tmp);
            g_free(tmp);

            write_to_mplayer(player, cmd);
            g_free(cmd);
            cmd = NULL;
        }
        break;

    case ATTRIBUTE_SUBTITLE_DELAY:
        player->subtitle_delay = value;
        if (player->player_state == PLAYER_STATE_RUNNING) {
            tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
            tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->subtitle_delay);
            cmd = g_strdup_printf("set_property sub_delay %s\n", tmp);
            g_free(tmp);
            write_to_mplayer(player, cmd);
            g_free(cmd);
            cmd = NULL;
        }
        break;

    case ATTRIBUTE_AUDIO_DELAY:
        player->audio_delay = CLAMP(value, -100.0, 100.0);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
            tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->audio_delay);
            cmd = g_strdup_printf("set_property audio_delay %s\n", tmp);
            g_free(tmp);
            write_to_mplayer(player, cmd);
            g_free(cmd);
            cmd = NULL;
        }
        break;

    default:
        gm_log(player->debug, G_LOG_LEVEL_INFO, "Unsupported Attribute");
    }

    return;
}

gdouble gmtk_media_player_get_attribute_double(GmtkMediaPlayer * player, GmtkMediaPlayerMediaAttributes attribute)
{
    gdouble ret = 0.0;

    switch (attribute) {
    case ATTRIBUTE_LENGTH:
        ret = player->length;
        break;

    case ATTRIBUTE_POSITION:
        ret = player->position;
        break;

    case ATTRIBUTE_POSITION_PERCENT:
        if (player->length != 0) {
            ret = (player->position - player->start_time) / player->length;
        } else {
            ret = 0.0;
        }
        break;

    case ATTRIBUTE_START_TIME:
        ret = player->start_time;
        break;

    case ATTRIBUTE_WIDTH:
        ret = (gdouble) player->video_width;
        break;

    case ATTRIBUTE_HEIGHT:
        ret = (gdouble) player->video_height;
        break;

    case ATTRIBUTE_SUB_COUNT:
        ret = (gdouble) g_list_length(player->subtitles);
        break;

    case ATTRIBUTE_AUDIO_TRACK_COUNT:
        ret = (gdouble) g_list_length(player->audio_tracks);
        break;

    case ATTRIBUTE_ZOOM:
        ret = player->zoom;
        break;

    case ATTRIBUTE_SPEED_MULTIPLIER:
        ret = player->speed_multiplier;
        break;

    case ATTRIBUTE_SPEED_SET:
        ret = player->speed;
        break;

    case ATTRIBUTE_SUBTITLE_SCALE:
        ret = player->subtitle_scale;
        break;

    case ATTRIBUTE_SUBTITLE_DELAY:
        ret = player->subtitle_delay;
        break;

    case ATTRIBUTE_AUDIO_DELAY:
        ret = player->audio_delay;
        break;

    case ATTRIBUTE_VIDEO_FPS:
        ret = player->video_fps;
        break;

    case ATTRIBUTE_VOLUME_GAIN:
        ret = player->volume_gain;
        break;

    case ATTRIBUTE_CACHE_PERCENT:
        ret = player->cache_percent;
        break;

    default:
        gm_log(player->debug, G_LOG_LEVEL_INFO, "Unsupported Attribute");
    }
    return ret;
}

void gmtk_media_player_set_attribute_string(GmtkMediaPlayer * player,
                                            GmtkMediaPlayerMediaAttributes attribute, const gchar * value)
{
    gchar *cmd;

    switch (attribute) {
    case ATTRIBUTE_VO:
        if (player->vo != NULL) {
            g_free(player->vo);
            player->vo = NULL;
        }
        if (value == NULL || strlen(value) == 0) {
            player->vo = NULL;
        } else {
            player->vo = g_strdup(value);
        }
        break;

    case ATTRIBUTE_AO:
        if (player->ao != NULL) {
            g_free(player->ao);
            player->ao = NULL;
        }
        if (value == NULL || strlen(value) == 0) {
            player->ao = NULL;
        } else {
            player->ao = g_strdup(value);
        }
        break;

    case ATTRIBUTE_MEDIA_DEVICE:
        if (player->media_device != NULL) {
            g_free(player->media_device);
            player->media_device = NULL;
        }
        if (value == NULL || strlen(value) == 0) {
            player->media_device = NULL;
        } else {
            player->media_device = g_strdup(value);
        }
        break;

    case ATTRIBUTE_EXTRA_OPTS:
        if (player->extra_opts != NULL) {
            g_free(player->extra_opts);
            player->extra_opts = NULL;
        }
        if (value == NULL || strlen(value) == 0) {
            player->extra_opts = NULL;
        } else {
            player->extra_opts = g_strdup(value);
        }
        break;

    case ATTRIBUTE_MPLAYER_BINARY:
        if (player->mplayer_binary != NULL) {
            g_free(player->mplayer_binary);
            player->mplayer_binary = NULL;
        }
        if (value == NULL || strlen(value) == 0) {
            player->mplayer_binary = NULL;
        } else {
            player->mplayer_binary = g_strdup(value);
        }
        player->features_detected = FALSE;
        break;

    case ATTRIBUTE_AUDIO_TRACK_FILE:
        if (player->audio_track_file != NULL) {
            g_free(player->audio_track_file);
            player->audio_track_file = NULL;
        }
        if (value == NULL || strlen(value) == 0) {
            player->audio_track_file = NULL;
        } else {
            player->audio_track_file = g_strdup(value);
        }
        break;

    case ATTRIBUTE_SUBTITLE_FILE:
        if (player->subtitle_file != NULL) {
            g_free(player->subtitle_file);
        }
        if (value == NULL || strlen(value) == 0) {
            player->subtitle_file = NULL;
        } else {
            player->subtitle_file = g_strdup(value);
            if (player->player_state == PLAYER_STATE_RUNNING) {
                write_to_mplayer(player, "sub_remove\n");
                cmd = g_strdup_printf("sub_load \"%s\" 1\n", player->subtitle_file);
                write_to_mplayer(player, cmd);
                g_free(cmd);
                cmd = g_strdup_printf("sub_file 0\n");
                write_to_mplayer(player, cmd);
                g_free(cmd);
            }
        }
        break;

    case ATTRIBUTE_SUBTITLE_COLOR:
        if (player->subtitle_color != NULL) {
            g_free(player->subtitle_color);
        }
        if (value == NULL || strlen(value) == 0) {
            player->subtitle_color = NULL;
        } else {
            player->subtitle_color = g_strdup(value);
        }
        break;

    case ATTRIBUTE_SUBTITLE_CODEPAGE:
        if (player->subtitle_codepage != NULL) {
            g_free(player->subtitle_codepage);
        }
        if (value == NULL || strlen(value) == 0) {
            player->subtitle_codepage = NULL;
        } else {
            player->subtitle_codepage = g_strdup(value);
        }
        break;

    case ATTRIBUTE_SUBTITLE_FONT:
        if (player->subtitle_font != NULL) {
            g_free(player->subtitle_font);
        }
        if (value == NULL || strlen(value) == 0) {
            player->subtitle_font = NULL;
        } else {
            player->subtitle_font = g_strdup(value);
        }
        break;

    case ATTRIBUTE_PROFILE:
        if (player->profile != NULL) {
            g_free(player->profile);
        }
        if (value == NULL || strlen(value) == 0) {
            player->profile = NULL;
        } else {
            player->profile = g_strdup(value);
        }
        break;

    case ATTRIBUTE_PREFERRED_AUDIO_LANGUAGE:
        if (player->alang != NULL) {
            g_free(player->alang);
        }
        if (value == NULL || strlen(value) == 0) {
            player->alang = NULL;
        } else {
            player->alang = g_strdup(value);
        }
        break;

    case ATTRIBUTE_PREFERRED_SUBTITLE_LANGUAGE:
        if (player->slang != NULL) {
            g_free(player->slang);
        }
        if (value == NULL || strlen(value) == 0) {
            player->slang = NULL;
        } else {
            player->slang = g_strdup(value);
        }
        break;

    default:
        gm_log(player->debug, G_LOG_LEVEL_INFO, "Unsupported Attribute");
    }
}

const gchar *gmtk_media_player_get_attribute_string(GmtkMediaPlayer * player, GmtkMediaPlayerMediaAttributes attribute)
{
    gchar *value = NULL;
    GList *iter;
    GmtkMediaPlayerAudioTrack *track;
    GmtkMediaPlayerSubtitle *subtitle;

    switch (attribute) {
    case ATTRIBUTE_AF_EXPORT_FILENAME:
        value = player->af_export_filename;
        break;

    case ATTRIBUTE_MEDIA_DEVICE:
        value = player->media_device;
        break;

    case ATTRIBUTE_EXTRA_OPTS:
        value = player->extra_opts;
        break;

    case ATTRIBUTE_MESSAGE:
        value = player->message;
        break;

    case ATTRIBUTE_AUDIO_TRACK:
        iter = player->audio_tracks;
        while (iter) {
            track = (GmtkMediaPlayerAudioTrack *) iter->data;
            if (track->id == player->audio_track_id)
                value = track->label;
            iter = iter->next;
        }
        break;

    case ATTRIBUTE_SUBTITLE:
        iter = player->subtitles;
        while (iter) {
            subtitle = (GmtkMediaPlayerSubtitle *) iter->data;
            if (subtitle->id == player->subtitle_id && subtitle->is_file == player->subtitle_is_file)
                value = subtitle->label;
            iter = iter->next;
        }
        break;

    case ATTRIBUTE_SUBTITLE_COLOR:
        value = player->subtitle_color;
        break;

    case ATTRIBUTE_SUBTITLE_CODEPAGE:
        value = player->subtitle_codepage;
        break;

    case ATTRIBUTE_SUBTITLE_FONT:
        value = player->subtitle_font;
        break;

    case ATTRIBUTE_VIDEO_FORMAT:
        value = player->video_format;
        break;

    case ATTRIBUTE_VIDEO_CODEC:
        value = player->video_codec;
        break;

    case ATTRIBUTE_AUDIO_FORMAT:
        value = player->audio_format;
        break;

    case ATTRIBUTE_AUDIO_CODEC:
        value = player->audio_codec;
        break;

    case ATTRIBUTE_ARTIST:
        if (player->artist == NULL || strlen(player->artist) == 0) {
            value = NULL;
        } else {
            value = player->artist;
        }
        break;

    case ATTRIBUTE_TITLE:
        if (player->title == NULL || strlen(player->title) == 0) {
            value = NULL;
        } else {
            value = player->title;
        }
        break;

    case ATTRIBUTE_ALBUM:
        if (player->album == NULL || strlen(player->album) == 0) {
            value = NULL;
        } else {
            value = player->album;
        }
        break;

    case ATTRIBUTE_GENRE:
        if (player->genre == NULL || strlen(player->genre) == 0) {
            value = NULL;
        } else {
            value = player->genre;
        }
        break;

    case ATTRIBUTE_PROFILE:
        value = player->profile;
        break;

    default:
        gm_log(player->debug, G_LOG_LEVEL_INFO, "Unsupported Attribute");
    }

    return value;
}

void gmtk_media_player_set_attribute_integer(GmtkMediaPlayer * player, GmtkMediaPlayerMediaAttributes attribute,
                                             gint value)
{
    gchar *cmd = NULL;

    switch (attribute) {
    case ATTRIBUTE_BRIGHTNESS:
        player->brightness = CLAMP(value, -100.0, 100.0);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            cmd = g_strdup_printf("set_property brightness %i\n", value);
            write_to_mplayer(player, cmd);
            g_free(cmd);
        }
        break;

    case ATTRIBUTE_CONTRAST:
        player->contrast = CLAMP(value, -100.0, 100.0);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            cmd = g_strdup_printf("set_property contrast %i\n", value);
            write_to_mplayer(player, cmd);
            g_free(cmd);
        }
        break;

    case ATTRIBUTE_GAMMA:
        player->gamma = CLAMP(value, -100.0, 100.0);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            cmd = g_strdup_printf("set_property gamma %i\n", value);
            write_to_mplayer(player, cmd);
            g_free(cmd);
        }
        break;

    case ATTRIBUTE_HUE:
        player->hue = CLAMP(value, -100.0, 100.0);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            cmd = g_strdup_printf("set_property hue %i\n", value);
            write_to_mplayer(player, cmd);
            g_free(cmd);
        }
        break;

    case ATTRIBUTE_SATURATION:
        player->saturation = CLAMP(value, -100.0, 100.0);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            cmd = g_strdup_printf("set_property saturation %i\n", value);
            write_to_mplayer(player, cmd);
            g_free(cmd);
        }
        break;

    case ATTRIBUTE_SUBTITLE_MARGIN:
        player->subtitle_margin = CLAMP(value, 0.0, 200.0);
        break;

    case ATTRIBUTE_SUBTITLE_POSITION:
        player->subtitle_position = CLAMP(value, 0, 100);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            cmd = g_strdup_printf("set_property sub_pos %i\n", player->subtitle_position);
            write_to_mplayer(player, cmd);
            g_free(cmd);
        }
        break;

    case ATTRIBUTE_OSDLEVEL:
        player->osdlevel = CLAMP(value, 0, 3);
        if (player->player_state == PLAYER_STATE_RUNNING) {
            cmd = g_strdup_printf("set_property osdlevel %i\n", player->osdlevel);
            write_to_mplayer(player, cmd);
            g_free(cmd);
        }
        break;

    case ATTRIBUTE_POST_PROCESSING_LEVEL:
        player->post_processing_level = value;
        break;

    case ATTRIBUTE_SUBTITLE_FUZZINESS:
        player->subtitle_fuzziness = CLAMP(value, 0, 2);
        break;

    case ATTRIBUTE_AUDIO_CHANNELS:
        player->audio_channels = CLAMP(value, 0, 3);
        break;

    default:
        gm_log(player->debug, G_LOG_LEVEL_INFO, "Unsupported Attribute");
    }

    return;

}

void gmtk_media_player_set_attribute_integer_delta(GmtkMediaPlayer * player, GmtkMediaPlayerMediaAttributes attribute,
                                                   gint delta)
{
    gint value;

    switch (attribute) {
    case ATTRIBUTE_BRIGHTNESS:
        value = player->brightness + delta;
        break;

    case ATTRIBUTE_CONTRAST:
        value = player->contrast + delta;
        break;

    case ATTRIBUTE_GAMMA:
        value = player->gamma + delta;
        break;

    case ATTRIBUTE_HUE:
        value = player->hue + delta;
        break;

    case ATTRIBUTE_SATURATION:
        value = player->saturation + delta;
        break;

    default:
        return;
    }

    gmtk_media_player_set_attribute_integer(player, attribute, value);

}

gint gmtk_media_player_get_attribute_integer(GmtkMediaPlayer * player, GmtkMediaPlayerMediaAttributes attribute)
{
    gint ret;

    switch (attribute) {
    case ATTRIBUTE_BRIGHTNESS:
        ret = player->brightness;
        break;

    case ATTRIBUTE_CONTRAST:
        ret = player->contrast;
        break;

    case ATTRIBUTE_GAMMA:
        ret = player->gamma;
        break;

    case ATTRIBUTE_HUE:
        ret = player->hue;
        break;

    case ATTRIBUTE_SATURATION:
        ret = player->saturation;
        break;

    case ATTRIBUTE_WIDTH:
        ret = player->video_width;
        break;

    case ATTRIBUTE_HEIGHT:
        ret = player->video_height;
        break;

    case ATTRIBUTE_SUBTITLE_MARGIN:
        ret = player->subtitle_margin;
        break;

    case ATTRIBUTE_SUBTITLE_POSITION:
        ret = player->subtitle_position;
        break;

    case ATTRIBUTE_CHAPTERS:
        ret = player->chapters;
        break;

    case ATTRIBUTE_VIDEO_BITRATE:
        ret = player->video_bitrate;
        break;

    case ATTRIBUTE_AUDIO_BITRATE:
        ret = player->audio_bitrate;
        break;

    case ATTRIBUTE_AUDIO_RATE:
        ret = player->audio_rate;
        break;

    case ATTRIBUTE_AUDIO_NCH:
        ret = player->audio_nch;
        break;

    case ATTRIBUTE_OSDLEVEL:
        ret = player->osdlevel;
        break;

    case ATTRIBUTE_POST_PROCESSING_LEVEL:
        ret = player->post_processing_level;
        break;

    case ATTRIBUTE_SUBTITLE_FUZZINESS:
        ret = player->subtitle_fuzziness;
        break;

    default:
        return 0;
    }

    return ret;
}


void gmtk_media_player_seek(GmtkMediaPlayer * player, gdouble value, GmtkMediaPlayerSeekType seek_type)
{
    gchar *cmd;

    cmd = g_strdup_printf("seek %lf %i\n", value, seek_type);
    write_to_mplayer(player, cmd);
    g_free(cmd);
}

void gmtk_media_player_seek_chapter(GmtkMediaPlayer * player, gint value, GmtkMediaPlayerSeekType seek_type)
{
    gchar *cmd;
    if (seek_type != SEEK_RELATIVE)
        seek_type = 1;

    cmd = g_strdup_printf("seek_chapter %i %i\n", value, seek_type);
    write_to_mplayer(player, cmd);
    g_free(cmd);
}

void gmtk_media_player_set_volume(GmtkMediaPlayer * player, gdouble value)
{
    gchar *cmd;

    player->volume = value;
    if (player->player_state == PLAYER_STATE_RUNNING) {
        cmd = g_strdup_printf("volume %i 1\n", (gint) (player->volume * 100.0));
        write_to_mplayer(player, cmd);
        g_free(cmd);
    }
}

gdouble gmtk_media_player_get_volume(GmtkMediaPlayer * player)
{
    return player->volume;
}

void gmtk_media_player_set_media_device(GmtkMediaPlayer * player, gchar * media_device)
{
    if (player->media_device != NULL) {
        g_free(player->media_device);
    }
    if (media_device == NULL) {
        player->media_device = NULL;
    } else {
        player->media_device = g_strdup(media_device);
    }
}

void gmtk_media_player_set_aspect(GmtkMediaPlayer * player, GmtkMediaPlayerAspectRatio aspect)
{
    GtkAllocation alloc;

    player->aspect_ratio = aspect;
    gmtk_get_allocation(GTK_WIDGET(player), &alloc);
    gmtk_media_player_size_allocate(GTK_WIDGET(player), &alloc);
}

GmtkMediaPlayerAspectRatio gmtk_media_player_get_aspect(GmtkMediaPlayer * player)
{
    return player->aspect_ratio;
}

void gmtk_media_player_set_media_type(GmtkMediaPlayer * player, GmtkMediaPlayerMediaType type)
{
    player->type = type;
}

GmtkMediaPlayerMediaType gmtk_media_player_get_media_type(GmtkMediaPlayer * player)
{
    return player->type;
}

void gmtk_media_player_select_subtitle(GmtkMediaPlayer * player, const gchar * label)
{
    GList *list;
    GmtkMediaPlayerSubtitle *subtitle;
    gchar *cmd;

    list = player->subtitles;
    subtitle = NULL;

    while (list != NULL) {
        subtitle = (GmtkMediaPlayerSubtitle *) list->data;
        if (g_ascii_strcasecmp(subtitle->label, label) == 0) {
            break;
        }
        list = list->next;
    }

    if (list != NULL && subtitle != NULL && player->player_state == PLAYER_STATE_RUNNING) {
        if (subtitle->is_file) {
            cmd = g_strdup_printf("sub_file %i \n", subtitle->id);
        } else {
            cmd = g_strdup_printf("sub_demux %i \n", subtitle->id);
        }
        player->subtitle_id = subtitle->id;
        player->subtitle_is_file = subtitle->is_file;
        write_to_mplayer(player, cmd);
        g_free(cmd);
    }
}

void gmtk_media_player_select_audio_track(GmtkMediaPlayer * player, const gchar * label)
{
    GList *list;
    GmtkMediaPlayerAudioTrack *track;
    gchar *cmd;

    list = player->audio_tracks;
    track = NULL;

    while (list != NULL) {
        track = (GmtkMediaPlayerAudioTrack *) list->data;
        if (g_ascii_strcasecmp(track->label, label) == 0) {
            break;
        }
        list = list->next;
    }

    if (list != NULL && track != NULL && player->player_state == PLAYER_STATE_RUNNING) {
        cmd = g_strdup_printf("switch_audio %i \n", track->id);
        player->audio_track_id = track->id;
        write_to_mplayer(player, cmd);
        g_free(cmd);
    }
}

void gmtk_media_player_select_subtitle_by_id(GmtkMediaPlayer * player, gint id)
{
    GList *list;
    GmtkMediaPlayerSubtitle *subtitle;
    gchar *cmd;

    list = player->subtitles;
    subtitle = NULL;

    while (list != NULL) {
        subtitle = (GmtkMediaPlayerSubtitle *) list->data;
        if (subtitle->id == id) {
            break;
        }
        list = list->next;
    }

    if (list != NULL && subtitle != NULL && player->player_state == PLAYER_STATE_RUNNING) {
        if (subtitle->is_file) {
            cmd = g_strdup_printf("sub_file %i \n", subtitle->id);
        } else {
            cmd = g_strdup_printf("sub_demux %i \n", subtitle->id);
        }
        player->subtitle_id = subtitle->id;
        player->subtitle_is_file = subtitle->is_file;
        write_to_mplayer(player, cmd);
        g_free(cmd);
    }
}

void gmtk_media_player_select_audio_track_by_id(GmtkMediaPlayer * player, gint id)
{
    GList *list;
    GmtkMediaPlayerAudioTrack *track;
    gchar *cmd;

    list = player->audio_tracks;
    track = NULL;

    while (list != NULL) {
        track = (GmtkMediaPlayerAudioTrack *) list->data;
        if (track->id == id) {
            break;
        }
        list = list->next;
    }

    if (list != NULL && track != NULL && player->player_state == PLAYER_STATE_RUNNING) {
        cmd = g_strdup_printf("switch_audio %i \n", track->id);
        player->audio_track_id = track->id;
        write_to_mplayer(player, cmd);
        g_free(cmd);
    }
}

static const gchar *playback_error_to_string(const GmtkMediaPlayerPlaybackError playback_error)
{
    switch (playback_error) {
    case NO_ERROR:
        return "NO_ERROR";
    case ERROR_RETRY_WITH_PLAYLIST:
        return "RETRY_WITH_PLAYLIST";
    case ERROR_RETRY_WITH_HTTP:
        return "RETRY_WITH_HTTP";
    case ERROR_RETRY_WITH_HTTP_AND_PLAYLIST:
        return "RETRY_WITH_HTTP_AND_PLAYLIST";
    case ERROR_RETRY_WITH_MMSHTTP:
        return "RETRY_WITH_MMSHTTP";
    case ERROR_RETRY_WITHOUT_DIVX_VDPAU:
        return "RETRY_WITHOUT_DIVX_VDPAU";
    case ERROR_RETRY_WITHOUT_XVMC:
        return "RETRY_WITHOUT_XVMC";
    case ERROR_RETRY_ALSA_BUSY:
        return "RETRY_ALSA_BUSY";
    case ERROR_RETRY_VDPAU:
        return "RETRY_VDPAU";
    case ERROR_RETRY_WITHOUT_HARDWARE_CODECS:
        return "RETRY_WITHOUT_HARDWARE_CODECS";
    case ERROR_RETRY:
        return "RETRY";
    default:
        return "???";
    }

}

/* if it contains a deint=N leave as is, otherwise add deint=2
   returns newly-allocated string, passing ownership to caller */
static gchar *vdpau_compute_vo_with_deint(GmtkMediaPlayer * player, gchar const *const vodesc)
{
    gchar *ret;
    if (g_regex_match(player->deintN_regex, vodesc, 0, NULL)) {
        ret = g_strdup(vodesc);
    } else {
        ret = g_strdup_printf("%s:deint=2", vodesc);
    }
    return ret;
}

/* if it contains a deint=N  remove that, otherwise leave as is
   returns newly-allocated string, passing ownership to caller */
static gchar *vdpau_compute_vo_without_deint(GmtkMediaPlayer * player, gchar const *const vodesc)
{
    GMatchInfo *match_info = NULL;
    gchar *ret;
    if (g_regex_match(player->deintN_regex, vodesc, 0, &match_info)) {
        gchar *before = g_match_info_fetch(match_info, 1);
        gchar *after = g_match_info_fetch(match_info, 3);
        ret = g_strdup_printf("%s%s", before, after);
        g_free(before);
        g_free(after);
    } else {
        ret = g_strdup(vodesc);
    }
    g_match_info_free(match_info);
    return ret;
}

/* replace the vo part with "gl_nosw"
   returns newly-allocated string, passing ownership to caller */
static gchar *vodesc_replace_gl_with_gl_nosw(GmtkMediaPlayer * player, gchar const *const vodesc)
{
    /* find the first colon : and replace the part before that with gl_nosw */
    char *colonptr = strchr(vodesc, ':');
    gchar *ret;
    if (colonptr == NULL) {
        ret = g_strdup("gl_nosw");
    } else {
        ret = g_strdup_printf("gl_nosw%s", colonptr);
    }
    return ret;
}

gpointer launch_mplayer(gpointer data)
{
    GmtkMediaPlayer *player = GMTK_MEDIA_PLAYER(data);
    gchar *argv[255];
    gchar *filename = NULL;
    gint argn;
    GPid pid;
    GError *error;
    gint i;
    gboolean spawn;
    gchar *fontname;
    gchar *size;
    gchar *tmp;
    GList *list;
    GmtkMediaPlayerSubtitle *subtitle;
    GmtkMediaPlayerAudioTrack *track;
    gchar *codecs_vdpau = NULL;
    gchar *codecs_crystalhd = NULL;
    gchar *codecs = NULL;
    GmtkMediaPlayerPlaybackError last_error = NO_ERROR;
#ifdef GIO_ENABLED
    GFile *file;
#endif

    gm_log_name_this_thread("lm");
    gm_log(player->debug, G_LOG_LEVEL_DEBUG, "within launch_mplayer()");

    player->seekable = FALSE;
    player->has_chapters = FALSE;
    //player->video_present = FALSE;
    player->position = 0.0;
    player->cache_percent = -1.0;
    player->title_is_menu = FALSE;
    player->enable_divx = TRUE;
    player->disable_xvmc = FALSE;
    player->retry_on_full_cache = FALSE;
    player->speed = 1.0;
    player->hardware_ac3 = FALSE;

    gm_log(player->debug, G_LOG_LEVEL_DEBUG, "locking thread_running");
    g_mutex_lock(player->thread_running);

    do {
        gm_log(player->debug, G_LOG_LEVEL_INFO, "setting up mplayer");

        list = player->subtitles;
        while (list) {
            subtitle = (GmtkMediaPlayerSubtitle *) list->data;
            g_free(subtitle->lang);
            g_free(subtitle->name);
            g_free(subtitle->label);
            list = g_list_remove(list, subtitle);
        }
        player->subtitles = NULL;

        list = player->audio_tracks;
        while (list) {
            track = (GmtkMediaPlayerAudioTrack *) list->data;
            g_free(track->lang);
            g_free(track->name);
            g_free(track->label);
            list = g_list_remove(list, track);
        }
        player->audio_tracks = NULL;
        player->has_metadata = FALSE;
        if (player->artist) {
            g_free(player->artist);
            player->artist = NULL;
        }
        if (player->title) {
            g_free(player->title);
            player->title = NULL;
        }
        if (player->album) {
            g_free(player->album);
            player->album = NULL;
        }

        argn = 0;
        player->playback_error = NO_ERROR;
        if (player->uri != NULL) {
#ifdef GIO_ENABLED
            file = g_file_new_for_uri(player->uri);
            if (file != NULL) {
                filename = g_file_get_path(file);
                g_object_unref(file);
            }
#else
            filename = g_filename_from_uri(player->uri, NULL, NULL);
#endif
            if (filename != NULL)
                player->type = TYPE_FILE;
        }

        player->minimum_mplayer = detect_mplayer_features(player);

        if (player->mplayer_binary == NULL || !g_file_test(player->mplayer_binary, G_FILE_TEST_EXISTS)) {
            argv[argn++] = g_strdup_printf("mplayer");
        } else {
            argv[argn++] = g_strdup_printf("%s", player->mplayer_binary);
        }

        // use the profile to set up some default values
        if (player->profile != NULL) {
            argv[argn++] = g_strdup_printf("-profile");
            argv[argn++] = g_strdup(player->profile);
        }

        if (player->vo != NULL) {
            argv[argn++] = g_strdup_printf("-vo");

            if (vodesc_looks_like_vo(player->vo, "vdpau")) {

                if (player->deinterlace) {
                    /* if it contains a deint=N leave as is, otherwise add deint=2 */
                    gchar *vo_with_deint = vdpau_compute_vo_with_deint(player, player->vo);
                    argv[argn++] = g_strdup_printf("%s,gl,x11,", player->vo);
                    g_free(vo_with_deint);
                } else {
                    /* if it contains a deint=N remove that, otherwise leave as is */
                    gchar *vo_without_deint = vdpau_compute_vo_without_deint(player, player->vo);
                    argv[argn++] = g_strdup_printf("%s,gl,x11,", player->vo);
                    g_free(vo_without_deint);
                }

                // told by uau that vdpau without hardware decoding is often what you want
                if (player->enable_hardware_codecs) {
                    if (player->enable_divx) {
                        codecs_vdpau =
                            g_strdup_printf("ffmpeg12vdpau,ffh264vdpau,ffwmv3vdpau,ffvc1vdpau,ffodivxvdpau,");
                    } else {
                        codecs_vdpau = g_strdup_printf("ffmpeg12vdpau,ffh264vdpau,ffwmv3vdpau,ffvc1vdpau,");
                    }
                }

            } else if (vodesc_looks_like_vo(player->vo, "vaapi")) {
                argv[argn++] = g_strdup_printf("%s,", player->vo);
                argv[argn++] = g_strdup_printf("-va");
                argv[argn++] = g_strdup_printf("vaapi");

            } else if (vodesc_looks_like_vo(player->vo, "xvmc")) {

                if (player->disable_xvmc) {
                    argv[argn++] = g_strdup_printf("xv,");
                } else {
                    argv[argn++] = g_strdup_printf("%s,xv,", player->vo);
                }

            } else {

                if (vodesc_looks_like_vo(player->vo, "gl") && player->enable_hardware_codecs) {
                    gchar *vodesc = vodesc_replace_gl_with_gl_nosw(player, player->vo);
                    argv[argn++] = vodesc;
                } else if (vodesc_looks_like_vo(player->vo, "gl2") && player->enable_hardware_codecs) {
                    gchar *vodesc = vodesc_replace_gl_with_gl_nosw(player, player->vo);
                    argv[argn++] = vodesc;
                } else {
                    argv[argn++] = g_strdup_printf("%s", player->vo);
                    if (vodesc_looks_like_vo(player->vo, "x11")) {
                        argv[argn++] = g_strdup_printf("-zoom");
                    }
                }

                if (player->deinterlace) {
                    argv[argn++] = g_strdup_printf("-vf-pre");
                    argv[argn++] = g_strdup_printf("yadif,softskip,scale");
                }

                if (player->post_processing_level > 0) {
                    argv[argn++] = g_strdup_printf("-vf-add");
                    argv[argn++] = g_strdup_printf("pp=ac/tn:a");
                    argv[argn++] = g_strdup_printf("-autoq");
                    argv[argn++] = g_strdup_printf("%d", player->post_processing_level);
                }

                argv[argn++] = g_strdup_printf("-vf-add");
                argv[argn++] = g_strdup_printf("screenshot");

            }
        }

        if (player->enable_crystalhd_codecs) {
            codecs_crystalhd = g_strdup_printf
                ("ffmpeg2crystalhd,ffdivxcrystalhd,ffwmv3crystalhd,ffvc1crystalhd,ffh264crystalhd,ffodivxcrystalhd,");
        }

        if (codecs_vdpau && codecs_crystalhd) {
            codecs = g_strconcat(codecs_vdpau, codecs_crystalhd, NULL);
            g_free(codecs_vdpau);
            g_free(codecs_crystalhd);
            codecs_vdpau = NULL;
            codecs_crystalhd = NULL;
        } else if (codecs_vdpau) {
            codecs = g_strdup(codecs_vdpau);
            g_free(codecs_vdpau);
            codecs_vdpau = NULL;
        } else if (codecs_crystalhd) {
            codecs = g_strdup(codecs_crystalhd);
            g_free(codecs_crystalhd);
            codecs_crystalhd = NULL;
        }

        if (codecs != NULL) {
            argv[argn++] = g_strdup_printf("-vc");
            argv[argn++] = g_strdup_printf("%s", codecs);
            g_free(codecs);
            codecs = NULL;
        }

        if (player->ao != NULL) {

            argv[argn++] = g_strdup_printf("-ao");
            argv[argn++] = g_strdup_printf("%s", player->ao);

            if (player->alsa_mixer != NULL) {
                argv[argn++] = g_strdup_printf("-mixer-channel");
                argv[argn++] = g_strdup_printf("%s", player->alsa_mixer);
            }
        }

        argv[argn++] = g_strdup_printf("-channels");
        switch (player->audio_channels) {
        case 1:
            argv[argn++] = g_strdup_printf("4");
            break;
        case 2:
            argv[argn++] = g_strdup_printf("6");
            break;
        case 3:
            argv[argn++] = g_strdup_printf("8");
            break;
        default:
            argv[argn++] = g_strdup_printf("2");
            break;
        }

        if (player->hardware_ac3) {
            argv[argn++] = g_strdup_printf("-afm");
            argv[argn++] = g_strdup_printf("hwac3,");
        } else {
            argv[argn++] = g_strdup_printf("-af-add");
            argv[argn++] = g_strdup_printf("export=%s:512", player->af_export_filename);
        }

        argv[argn++] = g_strdup_printf("-quiet");
        argv[argn++] = g_strdup_printf("-slave");
        argv[argn++] = g_strdup_printf("-noidle");
        argv[argn++] = g_strdup_printf("-noconsolecontrols");
        argv[argn++] = g_strdup_printf("-nostop-xscreensaver");
        argv[argn++] = g_strdup_printf("-identify");
        if (player->softvol) {
            if ((gint) (player->volume * 100) != 0) {
                argv[argn++] = g_strdup_printf("-volume");
                argv[argn++] = g_strdup_printf("%i", (gint) (player->volume * 100));
            }

            if ((gint) (player->volume_gain) != 0) {
                argv[argn++] = g_strdup_printf("-af-add");
                argv[argn++] = g_strdup_printf("volume=%lf:0", player->volume_gain);
            }

            argv[argn++] = g_strdup_printf("-softvol");
        }

        if ((gint) (player->start_time) > 0) {
            argv[argn++] = g_strdup_printf("-ss");
            tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
            tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->start_time);
            argv[argn++] = g_strdup(tmp);
            g_free(tmp);
        }

        if ((gint) (player->run_time) > 0) {
            argv[argn++] = g_strdup_printf("-endpos");
            tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
            tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->run_time);
            argv[argn++] = g_strdup(tmp);
            g_free(tmp);
        }

        if (player->frame_drop)
            argv[argn++] = g_strdup_printf("-framedrop");

        argv[argn++] = g_strdup_printf("-msglevel");
        argv[argn++] = g_strdup_printf("all=5");

        argv[argn++] = g_strdup_printf("-osdlevel");
        argv[argn++] = g_strdup_printf("%i", player->osdlevel);

        argv[argn++] = g_strdup_printf("-delay");
        tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
        tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->audio_delay);
        argv[argn++] = g_strdup(tmp);
        g_free(tmp);

        argv[argn++] = g_strdup_printf("-subdelay");
        tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
        tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->subtitle_delay);
        argv[argn++] = g_strdup(tmp);
        g_free(tmp);

        argv[argn++] = g_strdup_printf("-subpos");
        argv[argn++] = g_strdup_printf("%i", player->subtitle_position);

        argv[argn++] = g_strdup_printf("-sub-fuzziness");
        argv[argn++] = g_strdup_printf("%i", player->subtitle_fuzziness);

        // wait upto 1 second for the socket_id to be valid, but in plugin mode it may not so timeout
        i = 0;
        while (player->socket_id == 0 && (i++ < 100)) {
            g_usleep(1000);
        }

        argv[argn++] = g_strdup_printf("-wid");
        argv[argn++] = g_strdup_printf("0x%x", player->socket_id);

        argv[argn++] = g_strdup_printf("-brightness");
        argv[argn++] = g_strdup_printf("%i", player->brightness);
        argv[argn++] = g_strdup_printf("-contrast");
        argv[argn++] = g_strdup_printf("%i", player->contrast);
        //argv[argn++] = g_strdup_printf("-gamma");
        //argv[argn++] = g_strdup_printf("%i", player->gamma);
        argv[argn++] = g_strdup_printf("-hue");
        argv[argn++] = g_strdup_printf("%i", player->hue);
        argv[argn++] = g_strdup_printf("-saturation");
        argv[argn++] = g_strdup_printf("%i", player->saturation);

        if (player->alang) {
            argv[argn++] = g_strdup_printf("-alang");
            argv[argn++] = g_strdup_printf("%s", player->alang);
        }

        if (player->slang) {
            argv[argn++] = g_strdup_printf("-slang");
            argv[argn++] = g_strdup_printf("%s", player->slang);
        }

        /* disable msg stuff to make sure extra console characters don't mess around */
        argv[argn++] = g_strdup_printf("-nomsgcolor");
        argv[argn++] = g_strdup_printf("-nomsgmodule");

        // mplayer says that nokeepaspect isn't supported by all vo's but it seems to work
        //if (player->use_mplayer2)
        argv[argn++] = g_strdup_printf("-nokeepaspect");

        if (player->audio_track_file != NULL && strlen(player->audio_track_file) > 0) {
            argv[argn++] = g_strdup_printf("-audiofile");
            argv[argn++] = g_strdup_printf("%s", player->audio_track_file);
        }

        if (player->subtitle_file != NULL && strlen(player->subtitle_file) > 0) {
            argv[argn++] = g_strdup_printf("-sub");
            argv[argn++] = g_strdup_printf("%s", player->subtitle_file);
        }
        // subtitle stuff
        if (player->enable_advanced_subtitles) {
            argv[argn++] = g_strdup_printf("-ass");

            if (player->subtitle_margin > 0) {
                argv[argn++] = g_strdup_printf("-ass-bottom-margin");
                argv[argn++] = g_strdup_printf("%i", player->subtitle_margin);
                argv[argn++] = g_strdup_printf("-ass-use-margins");
            }

            if (player->enable_embedded_fonts) {
                argv[argn++] = g_strdup_printf("-embeddedfonts");
            } else {
                argv[argn++] = g_strdup_printf("-noembeddedfonts");

                if (player->subtitle_font != NULL && strlen(player->subtitle_font) > 0) {
                    fontname = g_strdup(player->subtitle_font);
                    size = g_strrstr(fontname, " ");
                    if (size)
                        size[0] = '\0';
                    size = g_strrstr(fontname, " Bold");
                    if (size)
                        size[0] = '\0';
                    size = g_strrstr(fontname, " Italic");
                    if (size)
                        size[0] = '\0';
                    argv[argn++] = g_strdup_printf("-ass-force-style");
                    argv[argn++] = g_strconcat("FontName=", fontname,
                                               ((g_strrstr(player->subtitle_font, "Italic") !=
                                                 NULL) ? ",Italic=1" : ",Italic=0"),
                                               ((g_strrstr(player->subtitle_font, "Bold") !=
                                                 NULL) ? ",Bold=1" : ",Bold=0"),
                                               (player->subtitle_outline ? ",Outline=1" : ",Outline=0"),
                                               (player->subtitle_shadow ? ",Shadow=2" : ",Shadow=0"), NULL);
                    g_free(fontname);
                }
            }

            argv[argn++] = g_strdup_printf("-ass-font-scale");
            tmp = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
            tmp = g_ascii_dtostr(tmp, G_ASCII_DTOSTR_BUF_SIZE, player->subtitle_scale);
            argv[argn++] = g_strdup(tmp);
            g_free(tmp);

            if (player->subtitle_color != NULL && strlen(player->subtitle_color) > 0) {
                argv[argn++] = g_strdup_printf("-ass-color");
                argv[argn++] = g_strdup_printf("%s", player->subtitle_color);
            }

        } else {
            if (player->subtitle_scale != 0) {
                argv[argn++] = g_strdup_printf("-subfont-text-scale");
                argv[argn++] = g_strdup_printf("%d", (int) (player->subtitle_scale * 5));       // 5 is the default
            }

            if (player->subtitle_font != NULL && strlen(player->subtitle_font) > 0) {
                fontname = g_strdup(player->subtitle_font);
                size = g_strrstr(fontname, " ");
                if (size)
                    size[0] = '\0';
                argv[argn++] = g_strdup_printf("-subfont");
                argv[argn++] = g_strdup_printf("%s", fontname);
                g_free(fontname);
            }
        }

        if (player->subtitle_codepage != NULL && strlen(player->subtitle_codepage) > 0) {
            argv[argn++] = g_strdup_printf("-subcp");
            argv[argn++] = g_strdup_printf("%s", player->subtitle_codepage);
        }

        if (player->extra_opts != NULL) {
            char **opts = g_strsplit(player->extra_opts, " ", -1);
            int i;
            for (i = 0; opts[i] != NULL; i++)
                argv[argn++] = g_strdup(opts[i]);
            g_strfreev(opts);
        }

        switch (player->type) {
        case TYPE_FILE:
            if (filename != NULL) {
                if (player->force_cache && player->cache_size >= 32) {
                    argv[argn++] = g_strdup_printf("-cache");
                    argv[argn++] = g_strdup_printf("%i", (gint) player->cache_size);
                }
                if (player->playlist) {
                    argv[argn++] = g_strdup_printf("-playlist");
                }
                argv[argn++] = g_strdup_printf("%s", filename);
                break;
            }
        case TYPE_CD:
            argv[argn++] = g_strdup_printf("-cache");
            argv[argn++] = g_strdup_printf("%i", (gint) player->cache_size);
            argv[argn++] = g_strdup_printf("%s", player->uri);
            if (player->media_device != NULL) {
                argv[argn++] = g_strdup_printf("-dvd-device");
                argv[argn++] = g_strdup_printf("%s", player->media_device);
            }
            break;

        case TYPE_DVD:
            argv[argn++] = g_strdup_printf("-mouse-movements");
            argv[argn++] = g_strdup_printf("-nocache");
            argv[argn++] = g_strdup_printf("dvdnav://");
            if (player->media_device != NULL) {
                argv[argn++] = g_strdup_printf("-dvd-device");
                argv[argn++] = g_strdup_printf("%s", player->media_device);
            }
            break;

        case TYPE_VCD:
            argv[argn++] = g_strdup_printf("-nocache");
            argv[argn++] = g_strdup_printf("%s", player->uri);
            if (player->media_device != NULL) {
                argv[argn++] = g_strdup_printf("-dvd-device");
                argv[argn++] = g_strdup_printf("%s", player->media_device);
            }
            break;

        case TYPE_NETWORK:
            if (g_strrstr(player->uri, "apple.com")) {
                argv[argn++] = g_strdup_printf("-user-agent");
                argv[argn++] = g_strdup_printf("QuickTime/7.6.9");
            }
            if (player->cache_size >= 32) {
                argv[argn++] = g_strdup_printf("-cache");
                argv[argn++] = g_strdup_printf("%i", (gint) player->cache_size);
            }
            if (player->playlist) {
                argv[argn++] = g_strdup_printf("-playlist");
            }
            argv[argn++] = g_strdup_printf("%s", player->uri);
            break;

        case TYPE_DVB:
        case TYPE_TV:
            if (player->tv_device != NULL) {
                argv[argn++] = g_strdup_printf("-tv:device");
                argv[argn++] = g_strdup_printf("%s", player->tv_device);
            }
            if (player->tv_driver != NULL) {
                argv[argn++] = g_strdup_printf("-tv:driver");
                argv[argn++] = g_strdup_printf("%s", player->tv_driver);
            }
            if (player->tv_input != NULL) {
                argv[argn++] = g_strdup_printf("-tv:input");
                argv[argn++] = g_strdup_printf("%s", player->tv_input);
            }
            if (player->tv_width > 0) {
                argv[argn++] = g_strdup_printf("-tv:width");
                argv[argn++] = g_strdup_printf("%i", player->tv_width);
            }
            if (player->tv_height > 0) {
                argv[argn++] = g_strdup_printf("-tv:height");
                argv[argn++] = g_strdup_printf("%i", player->tv_height);
            }
            if (player->tv_fps > 0) {
                argv[argn++] = g_strdup_printf("-tv:fps");
                argv[argn++] = g_strdup_printf("%i", player->tv_fps);
            }
            argv[argn++] = g_strdup_printf("-nocache");
            argv[argn++] = g_strdup_printf("%s", player->uri);


        default:
            break;
        }
        argv[argn] = NULL;

        gchar *allargs = g_strjoinv(" ", argv);
        gm_log(player->debug, G_LOG_LEVEL_INFO, "%s", allargs);
        g_free(allargs);

        error = NULL;
        player->std_in = -1;
        player->std_out = -1;
        player->std_err = -1;
        spawn =
            g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &pid,
                                     &(player->std_in), &(player->std_out), &(player->std_err), &error);

        gm_log(player->debug, G_LOG_LEVEL_DEBUG, "spawn = %s files in %i out %i err %i", gm_bool_to_string(spawn),
               player->std_in, player->std_out, player->std_err);

        if (error != NULL) {
            gm_log(player->debug, G_LOG_LEVEL_INFO, "error code = %i - %s", error->code, error->message);
            g_error_free(error);
            error = NULL;
        }

        argn = 0;
        while (argv[argn] != NULL) {
            g_free(argv[argn]);
            argv[argn] = NULL;
            argn++;
        }

        if (spawn) {
            gmtk_media_player_log_state(player, "launched");
            gm_log(player->debug, G_LOG_LEVEL_DEBUG, "spawn succeeded, setup up channels");

            player->player_state = PLAYER_STATE_RUNNING;
            player->media_state = MEDIA_STATE_BUFFERING;
            if (player->channel_in != NULL) {
                g_io_channel_unref(player->channel_in);
                player->channel_in = NULL;
            }

            if (player->channel_out != NULL) {
                g_io_channel_unref(player->channel_out);
                player->channel_out = NULL;
            }

            if (player->channel_err != NULL) {
                g_io_channel_unref(player->channel_err);
                player->channel_err = NULL;
            }

            player->channel_in = g_io_channel_unix_new(player->std_in);
            g_io_channel_set_encoding(player->channel_in, NULL, NULL);
            player->channel_out = g_io_channel_unix_new(player->std_out);
            g_io_channel_set_encoding(player->channel_out, NULL, NULL);
            player->channel_err = g_io_channel_unix_new(player->std_err);
            g_io_channel_set_encoding(player->channel_err, NULL, NULL);

            g_io_channel_set_close_on_unref(player->channel_in, TRUE);
            g_io_channel_set_close_on_unref(player->channel_out, TRUE);
            g_io_channel_set_close_on_unref(player->channel_err, TRUE);

            player->watch_in_id =
                g_io_add_watch_full(player->channel_out, G_PRIORITY_LOW, G_IO_IN, thread_reader, player, NULL);
            player->watch_err_id =
                g_io_add_watch_full(player->channel_err, G_PRIORITY_LOW, G_IO_IN, thread_reader_error, player, NULL);
            player->watch_in_hup_id =
                g_io_add_watch_full(player->channel_out, G_PRIORITY_LOW, G_IO_HUP, thread_complete, player, NULL);

#ifdef GLIB2_14_ENABLED
            g_timeout_add_seconds(1, thread_query, player);
#else
            g_timeout_add(1000, thread_query, player);
#endif

            // /////////////////////////////////////////////////////////////////////
            // Now this thread waits till somebody signals player->mplayer_complete_cond

            gm_log(player->debug, G_LOG_LEVEL_DEBUG, "waiting for mplayer_complete_cond");
            g_cond_wait(player->mplayer_complete_cond, player->thread_running);
            gm_log(player->debug, G_LOG_LEVEL_DEBUG, "mplayer_complete_cond was signalled");

            g_source_remove(player->watch_in_id);
            g_source_remove(player->watch_err_id);
            g_source_remove(player->watch_in_hup_id);
            if (player->channel_in != NULL) {
                g_io_channel_shutdown(player->channel_in, FALSE, NULL);
                g_io_channel_unref(player->channel_in);
                player->channel_in = NULL;
            }

            if (player->channel_out != NULL) {
                g_io_channel_shutdown(player->channel_out, FALSE, NULL);
                g_io_channel_unref(player->channel_out);
                player->channel_out = NULL;
            }

            if (player->channel_err != NULL) {
                g_io_channel_shutdown(player->channel_err, FALSE, NULL);
                g_io_channel_unref(player->channel_err);
                player->channel_err = NULL;
            }
            close(player->std_in);
            player->std_in = -1;
            g_spawn_close_pid(pid);

        }

        if (player->cache_percent < 0.0 && g_str_has_prefix(player->uri, "mms"))
            player->playback_error = ERROR_RETRY_WITH_MMSHTTP;

        if (player->cache_percent < 0.0 && g_str_has_prefix(player->uri, "mmshttp"))
            player->playback_error = ERROR_RETRY_WITH_HTTP;

        switch (player->playback_error) {
        case ERROR_RETRY_WITH_PLAYLIST:
            player->playlist = TRUE;
            break;

        case ERROR_RETRY_WITH_HTTP:
            tmp = gmtk_media_player_switch_protocol(player->uri, "http");
            g_free(player->uri);
            player->uri = tmp;
            break;

        case ERROR_RETRY_WITH_HTTP_AND_PLAYLIST:
            tmp = gmtk_media_player_switch_protocol(player->uri, "http");
            g_free(player->uri);
            player->uri = tmp;
            player->playlist = TRUE;
            break;

        case ERROR_RETRY_WITH_MMSHTTP:
            tmp = gmtk_media_player_switch_protocol(player->uri, "mmsh");
            g_free(player->uri);
            player->uri = tmp;
            break;

        case ERROR_RETRY_WITHOUT_DIVX_VDPAU:
            player->enable_divx = FALSE;
            break;

        case ERROR_RETRY_WITHOUT_XVMC:
            player->disable_xvmc = TRUE;
            break;

        case ERROR_RETRY_ALSA_BUSY:
        case ERROR_RETRY_VDPAU:
            break;
        case ERROR_RETRY:
            if (last_error == NO_ERROR) {
                last_error = ERROR_RETRY;
            } else {
                last_error = NO_ERROR;
                player->playback_error = NO_ERROR;
            }
            break;

        case ERROR_RETRY_WITHOUT_HARDWARE_CODECS:
            player->enable_hardware_codecs = FALSE;
            break;

        case ERROR_RETRY_WITHOUT_AF_EXPORT:
            player->hardware_ac3 = TRUE;
            break;

        default:
            break;
        }

        gm_log(player->debug, G_LOG_LEVEL_DEBUG, "playback error code is %s",
               playback_error_to_string(player->playback_error));

    } while (player->playback_error != NO_ERROR);

    gm_log(player->debug, G_LOG_LEVEL_DEBUG, "marking playback complete");
    player->player_state = PLAYER_STATE_DEAD;
    player->media_state = MEDIA_STATE_UNKNOWN;
    player->mplayer_thread = NULL;
    player->start_time = 0.0;
    player->run_time = 0.0;
    if (player->artist) {
        g_free(player->artist);
        player->artist = NULL;
    }
    if (player->title) {
        g_free(player->title);
        player->title = NULL;
    }
    if (player->album) {
        g_free(player->album);
        player->album = NULL;
    }

    gmtk_media_player_log_state(player, "finished");

    gm_log(player->debug, G_LOG_LEVEL_DEBUG, "unlocking thread_running");
    g_mutex_unlock(player->thread_running);

    if (player->restart) {
        g_signal_emit_by_name(player, "restart-shutdown-complete", NULL);
    } else {
        create_event_double(player, "position-changed", 0.0);
        create_event_int(player, "player-state-changed", player->player_state);
        create_event_int(player, "media-state-changed", player->media_state);
    }

    return NULL;
}

static void finalize_mplayer(GmtkMediaPlayer * player)
{
    g_source_remove(player->watch_in_id);
    g_source_remove(player->watch_in_hup_id);
    g_source_remove(player->watch_err_id);
    g_unlink(player->af_export_filename);
    gmtk_media_player_log_state(player, "completed");
    gm_log(player->debug, G_LOG_LEVEL_DEBUG, "signaling mplayer_complete_cond");
    g_cond_signal(player->mplayer_complete_cond);
}

// this executes in the main thread
gboolean thread_complete(GIOChannel * source, GIOCondition condition, gpointer data)
{
    GmtkMediaPlayer *player = GMTK_MEDIA_PLAYER(data);

    gm_log(player->debug, G_LOG_LEVEL_DEBUG, "thread_complete()");
    player->player_state = PLAYER_STATE_DEAD;
    player->media_state = MEDIA_STATE_UNKNOWN;
    finalize_mplayer(player);

    return FALSE;
}

// this executes in the main thread
gboolean thread_reader_error(GIOChannel * source, GIOCondition condition, gpointer data)
{
    GmtkMediaPlayer *player = GMTK_MEDIA_PLAYER(data);
    GString *mplayer_output;
    GIOStatus status;
    gchar *error_msg = NULL;
    //GtkWidget *dialog;
    gchar *buf;

    if (player == NULL) {
        gm_log(player->debug, G_LOG_LEVEL_DEBUG, "player is NULL");
        finalize_mplayer(player);
        return FALSE;
    }

    if (source == NULL) {
        gm_log(player->debug, G_LOG_LEVEL_DEBUG, "source is null");
        finalize_mplayer(player);
        return FALSE;
    }

    if (player->player_state == PLAYER_STATE_DEAD) {
        gm_log(player->debug, G_LOG_LEVEL_DEBUG, "player is dead");
        finalize_mplayer(player);
        return FALSE;
    }

    mplayer_output = g_string_new("");
    status = g_io_channel_read_line_string(source, mplayer_output, NULL, NULL);
    if (status == G_IO_STATUS_ERROR) {
        gm_logsp(player->debug, G_LOG_LEVEL_INFO, "GIO IO Error:", mplayer_output->str);
        return TRUE;
    } else {
        if (g_strrstr(mplayer_output->str, "ANS") == NULL) {
            gm_logsp(player->debug, G_LOG_LEVEL_INFO, "< ERROR:", mplayer_output->str);
        }

        if (strstr(mplayer_output->str, "Couldn't open DVD device") != 0) {
            error_msg = g_strdup(mplayer_output->str);
        }

        if (strstr(mplayer_output->str, "X11 error") != 0) {
            create_event_int(player, "attribute-changed", ATTRIBUTE_SIZE);
        }

        if (strstr(mplayer_output->str, "signal") != NULL) {
            if (strstr(mplayer_output->str, "decode") != NULL) {
                create_event_int(player, "attribute-changed", ATTRIBUTE_SIZE);
                if (player->position == 0) {
                    player->playback_error = ERROR_RETRY;
                }
            } else if (strstr(mplayer_output->str, "filter video") != NULL) {
                player->playback_error = ERROR_RETRY;
            } else {
                error_msg = g_strdup(mplayer_output->str);
            }
        }
        if (strstr(mplayer_output->str, "Error when calling vdp_output_surface_create") != NULL) {
            create_event_int(player, "attribute-changed", ATTRIBUTE_SIZE);
            if (player->position == 0) {
                player->playback_error = ERROR_RETRY;
            }
        }

        if (strstr(mplayer_output->str, "Failed creating VDPAU decoder") != NULL) {
            if (player->enable_divx && vodesc_looks_like_vo(player->vo, "vdpau"))
                player->playback_error = ERROR_RETRY_WITHOUT_DIVX_VDPAU;
        }

        if (strstr(mplayer_output->str, "decoding to PIX_FMT_NONE is not supported") != NULL) {
            if (player->enable_divx)
                player->playback_error = ERROR_RETRY_WITHOUT_HARDWARE_CODECS;
        }

        if (strstr(mplayer_output->str, "The selected video_out device is incompatible with this codec") != NULL) {
            if (!player->disable_xvmc && vodesc_looks_like_vo(player->vo, "xvmc"))
                player->playback_error = ERROR_RETRY_WITHOUT_XVMC;
        }

        if (strstr(mplayer_output->str, "[AO_ALSA] Playback open error: Device or resource busy") != NULL) {
            player->playback_error = ERROR_RETRY_ALSA_BUSY;
        }

        if (strstr(mplayer_output->str, "Sample format big-endian AC3 not yet supported") != NULL) {
            player->playback_error = ERROR_RETRY_WITHOUT_AF_EXPORT;
        }

        /*
           if (strstr(mplayer_output->str, "Error when calling vdp_output_surface_create") != NULL) {
           create_event_int(player, "attribute-changed", ATTRIBUTE_SIZE);
           player->playback_error = ERROR_RETRY_VDPAU;
           write_to_mplayer(player, "quit\n");
           }
         */

        if (strstr(mplayer_output->str, "Failed to open") != NULL) {
            if (strstr(mplayer_output->str, "LIRC") == NULL &&
                strstr(mplayer_output->str, "input.conf") == NULL &&
                strstr(mplayer_output->str, "/dev/rtc") == NULL &&
                strstr(mplayer_output->str, "VDPAU") == NULL && strstr(mplayer_output->str, "registry file") == NULL) {
                if (strstr(mplayer_output->str, "<") == NULL && strstr(mplayer_output->str, ">") == NULL
                    && player->type == TYPE_FILE) {
                    error_msg =
                        g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "Failed to open %s"),
                                        mplayer_output->str + strlen("Failed to open "));
                }

                if (strstr(mplayer_output->str, "mms://") != NULL && player->type == TYPE_NETWORK) {
                    player->playback_error = ERROR_RETRY_WITH_MMSHTTP;
                }
            }
        }

        if (strstr(mplayer_output->str, "MPlayer interrupted by signal 13 in module: open_stream") != NULL
            && g_strrstr(player->uri, "mms://") != NULL) {
            player->playback_error = ERROR_RETRY_WITH_MMSHTTP;
        }

        if (strstr(mplayer_output->str, "No stream found to handle url mmshttp://") != NULL) {
            player->playback_error = ERROR_RETRY_WITH_HTTP;
        }

        if (strstr(mplayer_output->str, "Server returned 404:File Not Found") != NULL
            && g_strrstr(player->uri, "mmshttp://") != NULL) {
            player->playback_error = ERROR_RETRY_WITH_HTTP;
        }

        if (strstr(mplayer_output->str, "unknown ASF streaming type") != NULL
            && g_strrstr(player->uri, "mmshttp://") != NULL) {
            player->playback_error = ERROR_RETRY_WITH_HTTP;
        }

        if (strstr(mplayer_output->str, "Error while parsing chunk header") != NULL) {
            player->playback_error = ERROR_RETRY_WITH_HTTP_AND_PLAYLIST;
        }

        if (strstr(mplayer_output->str, "Failed to initiate \"video/X-ASF-PF\" RTP subsession") != NULL) {
            player->playback_error = ERROR_RETRY_WITH_PLAYLIST;
        }

        if (strstr(mplayer_output->str, "playlist support will not be used") != NULL) {
            player->playback_error = ERROR_RETRY_WITH_PLAYLIST;
        }

        if (strstr(mplayer_output->str, "Compressed SWF format not supported") != NULL) {
            error_msg = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "Compressed SWF format not supported"));
        }

        if (strstr(mplayer_output->str, "moov atom not found") != NULL) {
            player->retry_on_full_cache = TRUE;
            create_event_boolean(player, "attribute-changed", ATTRIBUTE_RETRY_ON_FULL_CACHE);
        }

        if (strstr(mplayer_output->str, "MOV: missing header (moov/cmov) chunk") != NULL) {
            player->retry_on_full_cache = TRUE;
            create_event_boolean(player, "attribute-changed", ATTRIBUTE_RETRY_ON_FULL_CACHE);
        }

        if (strstr(mplayer_output->str, "Seek failed") != NULL) {
            write_to_mplayer(player, "quit\n");
            player->retry_on_full_cache = TRUE;
            create_event_boolean(player, "attribute-changed", ATTRIBUTE_RETRY_ON_FULL_CACHE);
        }

        if (strstr(mplayer_output->str, "Title: ") != 0) {
            buf = strstr(mplayer_output->str, "Title:");
            buf = strstr(mplayer_output->str, "Title: ") + strlen("Title: ");
            buf = g_strchomp(buf);
            if (player->title != NULL) {
                g_free(player->title);
                player->title = NULL;
            }

            player->title = g_locale_to_utf8(buf, -1, NULL, NULL, NULL);
            if (player->title == NULL) {
                player->title = g_strdup(buf);
                gm_str_strip_unicode(player->title, strlen(player->title));
            }
            create_event_int(player, "attribute-changed", ATTRIBUTE_TITLE);
        }

        if (error_msg != NULL && player->playback_error == NO_ERROR) {
            create_event_string(player, "error-message", error_msg);
            /*
               dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
               GTK_BUTTONS_CLOSE, "%s", error_msg);
               gtk_window_set_title(GTK_WINDOW(dialog), g_dgettext(GETTEXT_PACKAGE, "GNOME MPlayer Error"));
               gtk_dialog_run(GTK_DIALOG(dialog));
               gtk_widget_destroy(dialog);
             */
            g_free(error_msg);
            error_msg = NULL;
        }

    }
    g_string_free(mplayer_output, TRUE);

    return TRUE;
}

// this executes in the main thread
gboolean thread_reader(GIOChannel * source, GIOCondition condition, gpointer data)
{
    GmtkMediaPlayer *player = GMTK_MEDIA_PLAYER(data);
    GString *mplayer_output;
    GIOStatus status;
    GError *error = NULL;
    gchar *buf, *title, *message = NULL, *icy = NULL;
    gint w, h, i;
    gfloat percent, oldposition;
    gchar vm[10];
    gint id;
    GtkAllocation allocation = { 0 };
    GmtkMediaPlayerSubtitle *subtitle = NULL;
    GmtkMediaPlayerAudioTrack *audio_track = NULL;
    GList *iter;
    GtkWidget *dialog;
    gchar **split;
    gint index;

    if (player == NULL) {
        gm_log(player->debug, G_LOG_LEVEL_MESSAGE, "player is NULL");
        finalize_mplayer(player);
        return FALSE;
    }

    if (source == NULL) {
        gm_log(player->debug, G_LOG_LEVEL_INFO, "source is null");
        finalize_mplayer(player);
        return FALSE;
    }

    if (player->player_state == PLAYER_STATE_DEAD) {
        gm_log(player->debug, G_LOG_LEVEL_INFO, "player is dead");
        finalize_mplayer(player);
        return FALSE;
    }

    mplayer_output = g_string_new("");

    status = g_io_channel_read_line_string(source, mplayer_output, NULL, &error);
    if (status == G_IO_STATUS_ERROR) {
        gm_logsp(player->debug, G_LOG_LEVEL_INFO, "GIO IO Error:", mplayer_output->str);
        return TRUE;
    } else {
        if (g_strrstr(mplayer_output->str, "ANS") == NULL) {
            gm_logsp(player->debug, G_LOG_LEVEL_INFO, "<", mplayer_output->str);
        } else {
            gm_logsp(player->debug, G_LOG_LEVEL_DEBUG, "<", mplayer_output->str);
        }

        if (strstr(mplayer_output->str, "Cache fill") != 0) {
            buf = strstr(mplayer_output->str, "Cache fill");
            sscanf(buf, "Cache fill: %f%%", &percent);
            buf = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "Cache fill: %2.2f%%"), percent);
            player->cache_percent = percent / 100.0;
            create_event_double(player, "cache-percent-changed", player->cache_percent);
        }

        if (strstr(mplayer_output->str, "AO:") != NULL) {
            write_to_mplayer(player, "get_property switch_audio\n");
        }

        if (strstr(mplayer_output->str, "VO:") != NULL) {
            buf = strstr(mplayer_output->str, "VO:");
            sscanf(buf, "VO: [%[^]]] %ix%i => %ix%i", vm, &w, &h, &(player->video_width), &(player->video_height));
            gm_log(player->debug, G_LOG_LEVEL_DEBUG, "%ix%i => %ix%i", w, h, player->video_width, player->video_height);
            gmtk_get_allocation(GTK_WIDGET(player), &allocation);
            player->media_state = MEDIA_STATE_PLAY;
            if (player->restart) {
                g_signal_emit_by_name(player, "restart-complete", NULL);
            } else {
                create_event_int(player, "media-state-changed", player->media_state);
                allocation.width = player->video_width;
                allocation.height = player->video_height;
				if (player->video_present == FALSE) {
            		create_event_allocation(player, "size_allocate", &allocation);
				}
                player->video_present = TRUE;
                buf = g_strdup_printf("set_property sub_visibility %i\n", player->sub_visible);
                write_to_mplayer(player, buf);
                g_free(buf);
                write_to_mplayer(player, "get_property sub_source\n");
                write_to_mplayer(player, "get_property sub_visibility\n");
                create_event_int(player, "attribute-changed", ATTRIBUTE_SIZE);
                create_event_int(player, "attribute-changed", ATTRIBUTE_VIDEO_PRESENT);
                create_event_int(player, "subtitles-changed", g_list_length(player->subtitles));
                create_event_int(player, "audio-tracks-changed", g_list_length(player->audio_tracks));
                create_event_double(player, "cache-percent-changed", 0.0);
            }
            create_event_int(player, "attribute-changed", ATTRIBUTE_AF_EXPORT_FILENAME);
            create_event_int(player, "attribute-changed", ATTRIBUTE_AUDIO_TRACK);
            create_event_int(player, "attribute-changed", ATTRIBUTE_SUBTITLE);
            gmtk_media_player_log_state(player, "media_loaded");
        }

        if (strstr(mplayer_output->str, "Video: no video") != NULL) {
            gm_log(player->debug, G_LOG_LEVEL_MESSAGE, "Running in audio only mode");
            player->video_width = 0;
            player->video_height = 0;
            gmtk_get_allocation(GTK_WIDGET(player), &allocation);
            player->media_state = MEDIA_STATE_PLAY;
            if (player->restart) {
                g_signal_emit_by_name(player, "restart-complete", NULL);
            } else {
                create_event_int(player, "media-state-changed", player->media_state);
                player->video_present = FALSE;
                create_event_int(player, "attribute-changed", ATTRIBUTE_SIZE);
                create_event_int(player, "attribute-changed", ATTRIBUTE_VIDEO_PRESENT);
                create_event_int(player, "subtitles-changed", g_list_length(player->subtitles));
                create_event_int(player, "audio-tracks-changed", g_list_length(player->audio_tracks));
                create_event_double(player, "cache-percent-changed", 0.0);
            }
            create_event_int(player, "attribute-changed", ATTRIBUTE_AF_EXPORT_FILENAME);
            create_event_int(player, "attribute-changed", ATTRIBUTE_AUDIO_TRACK);
            create_event_int(player, "attribute-changed", ATTRIBUTE_SUBTITLE);
            gmtk_media_player_log_state(player, "media_loaded");
        }

        if (strstr(mplayer_output->str, "ANS_TIME_POSITION") != 0) {
            buf = strstr(mplayer_output->str, "ANS_TIME_POSITION");
            oldposition = player->position;
            sscanf(buf, "ANS_TIME_POSITION=%lf", &player->position);
            if (oldposition != player->position)
                create_event_double(player, "position-changed", player->position);
            if (player->position > player->length)
                write_to_mplayer(player, "get_time_length\n");
        }

        if (strstr(mplayer_output->str, "ID_START_TIME") != 0) {
            buf = strstr(mplayer_output->str, "ID_START_TIME");
            sscanf(buf, "ID_START_TIME=%lf", &player->start_time);
            create_event_int(player, "attribute-changed", ATTRIBUTE_START_TIME);
        }

        if (strstr(mplayer_output->str, "ID_LENGTH") != 0) {
            buf = strstr(mplayer_output->str, "ID_LENGTH");
            sscanf(buf, "ID_LENGTH=%lf", &player->length);
            create_event_int(player, "attribute-changed", ATTRIBUTE_LENGTH);
        }

        if (strstr(mplayer_output->str, "ANS_LENGTH") != 0) {
            buf = strstr(mplayer_output->str, "ANS_LENGTH");
            sscanf(buf, "ANS_LENGTH=%lf", &player->length);
            create_event_int(player, "attribute-changed", ATTRIBUTE_LENGTH);
        }

        if (strstr(mplayer_output->str, "ID_AUDIO_TRACK") != 0) {
            buf = strstr(mplayer_output->str, "ID_AUDIO_TRACK");
            id = player->audio_track_id;
            sscanf(buf, "ID_AUDIO_TRACK=%i", &player->audio_track_id);
            if (id != player->audio_track_id)
                create_event_int(player, "attribute-changed", ATTRIBUTE_AUDIO_TRACK);
        }

        if (strstr(mplayer_output->str, "ANS_switch_audio") != 0) {
            buf = strstr(mplayer_output->str, "ANS_switch_audio");
            id = player->audio_track_id;
            sscanf(buf, "ANS_switch_audio=%i", &player->audio_track_id);
            if (id != player->audio_track_id)
                create_event_int(player, "attribute-changed", ATTRIBUTE_AUDIO_TRACK);
        }

        if (strstr(mplayer_output->str, "ANS_sub_source") != 0) {
            buf = strstr(mplayer_output->str, "ANS_sub_source");
            sscanf(buf, "ANS_sub_source=%i", &player->subtitle_source);
            switch (player->subtitle_source) {
            case 0:
                player->subtitle_is_file = TRUE;
                write_to_mplayer(player, "get_property sub_file\n");
                break;
            case 1:
                player->subtitle_is_file = FALSE;
                write_to_mplayer(player, "get_property sub_vob\n");
                break;
            case 2:
                player->subtitle_is_file = FALSE;
                write_to_mplayer(player, "get_property sub_demux\n");
                break;
            }
        }

        if (strstr(mplayer_output->str, "ANS_sub_file") != 0) {
            buf = strstr(mplayer_output->str, "ANS_sub_file");
            sscanf(buf, "ANS_sub_file=%i", &player->subtitle_id);
            create_event_int(player, "attribute-changed", ATTRIBUTE_SUBTITLE);
        }

        if (strstr(mplayer_output->str, "ANS_sub_demux") != 0) {
            buf = strstr(mplayer_output->str, "ANS_sub_demux");
            sscanf(buf, "ANS_sub_demux=%i", &player->subtitle_id);
            create_event_int(player, "attribute-changed", ATTRIBUTE_SUBTITLE);
        }

        if (strstr(mplayer_output->str, "ANS_sub_visibility") != 0) {
            if (strstr(mplayer_output->str, "ANS_sub_visibility=yes") != 0) {
                player->sub_visible = TRUE;
            } else {
                player->sub_visible = FALSE;
            }
            create_event_int(player, "attribute-changed", ATTRIBUTE_SUB_VISIBLE);
        }

        if (strstr(mplayer_output->str, "ANS_speed") != 0) {
            buf = strstr(mplayer_output->str, "ANS_speed");
            sscanf(buf, "ANS_speed=%lf", &player->speed);
            gm_log(player->debug, G_LOG_LEVEL_MESSAGE, "new speed is %lf", player->speed);
            create_event_int(player, "attribute-changed", ATTRIBUTE_SPEED_SET);
        }

        if (strstr(mplayer_output->str, "DVDNAV_TITLE_IS_MENU") != 0) {
            player->title_is_menu = TRUE;
            write_to_mplayer(player, "get_time_length\n");
        }

        if (strstr(mplayer_output->str, "DVDNAV_TITLE_IS_MOVIE") != 0) {
            player->title_is_menu = FALSE;
            write_to_mplayer(player, "get_time_length\n");
        }

        if (strstr(mplayer_output->str, "ID_SUBTITLE_ID=") != 0) {
            buf = strstr(mplayer_output->str, "ID_SUBTITLE_ID");
            sscanf(buf, "ID_SUBTITLE_ID=%i", &id);
            subtitle = g_new0(GmtkMediaPlayerSubtitle, 1);
            subtitle->id = id;
            subtitle->lang = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "Unknown"));
            subtitle->name = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "Unknown"));
            subtitle->label = g_strdup_printf("%s (%s) - %i", subtitle->name, subtitle->lang, subtitle->id);
            player->subtitles = g_list_append(player->subtitles, subtitle);

        }

        if (strstr(mplayer_output->str, "ID_SID_") != 0) {
            buf = strstr(mplayer_output->str, "ID_SID_");
            sscanf(buf, "ID_SID_%i_", &id);
            g_string_truncate(mplayer_output, mplayer_output->len - 1);
            buf = strstr(mplayer_output->str, "_LANG=");
            if (buf != NULL) {
                buf += strlen("_LANG=");
                iter = player->subtitles;
                while (iter) {
                    subtitle = ((GmtkMediaPlayerSubtitle *) (iter->data));
                    if (subtitle->id == id && subtitle->is_file == FALSE) {
                        if (subtitle->lang != NULL) {
                            g_free(subtitle->lang);
                            subtitle->lang = NULL;
                        }
                        subtitle->lang = g_strdup(buf);
                    }
                    iter = iter->next;
                }
            }
            buf = strstr(mplayer_output->str, "_NAME=");
            if (buf != NULL) {
                buf += strlen("_NAME=");
                iter = player->subtitles;
                while (iter) {
                    subtitle = ((GmtkMediaPlayerSubtitle *) (iter->data));
                    if (subtitle->id == id && subtitle->is_file == FALSE) {
                        if (subtitle->name != NULL) {
                            g_free(subtitle->name);
                            subtitle->name = NULL;
                        }
                        subtitle->name = g_strdup(buf);
                    }
                    iter = iter->next;
                }
            }
            if (subtitle) {
                if (subtitle->label != NULL) {
                    g_free(subtitle->label);
                    subtitle->label = NULL;
                }
                subtitle->label =
                    g_strdup_printf("%s (%s) - %i",
                                    (subtitle->name) ? subtitle->name : g_dgettext(GETTEXT_PACKAGE, "Unknown"),
                                    subtitle->lang, subtitle->id);
            }
        }

        if (strstr(mplayer_output->str, "ID_FILE_SUB_ID=") != 0) {
            buf = strstr(mplayer_output->str, "ID_FILE_SUB_ID");
            sscanf(buf, "ID_FILE_SUB_ID=%i", &id);
            subtitle = g_new0(GmtkMediaPlayerSubtitle, 1);
            subtitle->id = id;
            subtitle->is_file = TRUE;
            subtitle->label = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "External Subtitle #%i"), id + 1);
            player->subtitles = g_list_append(player->subtitles, subtitle);
            create_event_int(player, "subtitles-changed", g_list_length(player->subtitles));
        }

        if (strstr(mplayer_output->str, "ID_AUDIO_ID=") != 0) {
            buf = strstr(mplayer_output->str, "ID_AUDIO_ID");
            sscanf(buf, "ID_AUDIO_ID=%i", &id);
            iter = player->audio_tracks;
            gboolean found = FALSE;
            while (iter) {
                audio_track = ((GmtkMediaPlayerAudioTrack *) (iter->data));
                if (audio_track->id == id) {
                    found = TRUE;
                }
                iter = iter->next;
            }

            if (!found) {
                audio_track = g_new0(GmtkMediaPlayerAudioTrack, 1);
                audio_track->id = id;
                audio_track->lang = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "Unknown"));
                audio_track->name = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "Unknown"));
                audio_track->label = g_strdup_printf("%s (%s) - %i", audio_track->name, audio_track->lang,
                                                     audio_track->id);
                player->audio_tracks = g_list_append(player->audio_tracks, audio_track);
            }
        }

        if (strstr(mplayer_output->str, "ID_AID_") != 0) {
            buf = strstr(mplayer_output->str, "ID_AID_");
            sscanf(buf, "ID_AID_%i_", &id);
            g_string_truncate(mplayer_output, mplayer_output->len - 1);
            buf = strstr(mplayer_output->str, "_LANG=");
            if (buf != NULL) {
                buf += strlen("_LANG=");
                iter = player->audio_tracks;
                gboolean updated = FALSE;
                while (iter) {
                    audio_track = ((GmtkMediaPlayerAudioTrack *) (iter->data));
                    if (audio_track->id == id) {
                        updated = TRUE;
                        if (audio_track->lang != NULL) {
                            g_free(audio_track->lang);
                            audio_track->lang = NULL;
                        }
                        audio_track->lang = g_strdup(buf);
                    }
                    iter = iter->next;
                }
                if (updated == FALSE) {
                    audio_track = g_new0(GmtkMediaPlayerAudioTrack, 1);
                    audio_track->id = id;
                    audio_track->lang = g_strdup_printf("%s", buf);
                    audio_track->name = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "Unknown"));
                    audio_track->label = g_strdup_printf("%s (%s) - %i", audio_track->name, audio_track->lang,
                                                         audio_track->id);
                    player->audio_tracks = g_list_append(player->audio_tracks, audio_track);
                    create_event_int(player, "audio-tracks-changed", g_list_length(player->audio_tracks));
                }
            }
            buf = strstr(mplayer_output->str, "_NAME=");
            if (buf != NULL) {
                buf += strlen("_NAME=");
                iter = player->audio_tracks;
                while (iter) {
                    audio_track = ((GmtkMediaPlayerAudioTrack *) (iter->data));
                    if (audio_track->id == id) {
                        if (audio_track->name != NULL) {
                            g_free(audio_track->name);
                            audio_track->name = NULL;
                        }
                        audio_track->name = g_strdup(buf);
                    }
                    iter = iter->next;
                }
            }

            if (audio_track) {
                if (audio_track->label != NULL) {
                    g_free(audio_track->label);
                    audio_track->label = NULL;
                }

                audio_track->label =
                    g_strdup_printf("%s (%s) - %i",
                                    (audio_track->name) ? audio_track->name : g_dgettext(GETTEXT_PACKAGE, "Unknown"),
                                    audio_track->lang, audio_track->id);
            }
        }

        if ((strstr(mplayer_output->str, "ID_CHAPTERS=") != NULL)) {
            buf = strstr(mplayer_output->str, "ID_CHAPTERS");
            sscanf(buf, "ID_CHAPTERS=%i", &player->chapters);
            if (player->chapters > 1) {
                player->has_chapters = TRUE;
            } else {
                player->has_chapters = FALSE;
            }
            create_event_int(player, "attribute-changed", ATTRIBUTE_HAS_CHAPTERS);
            create_event_int(player, "attribute-changed", ATTRIBUTE_CHAPTERS);
        }

        if ((strstr(mplayer_output->str, "ID_SEEKABLE=") != NULL)
            && !(strstr(mplayer_output->str, "ID_SEEKABLE=0") != NULL)) {
            player->seekable = TRUE;
            create_event_int(player, "attribute-changed", ATTRIBUTE_SEEKABLE);
        }

        if (strstr(mplayer_output->str, "ID_VIDEO_FORMAT") != 0) {
            g_string_truncate(mplayer_output, mplayer_output->len - 1);
            buf = strstr(mplayer_output->str, "ID_VIDEO_FORMAT") + strlen("ID_VIDEO_FORMAT=");
            if (player->video_format != NULL) {
                g_free(player->video_format);
                player->video_format = NULL;
            }
            player->video_format = g_strdup(buf);
            create_event_int(player, "attribute-changed", ATTRIBUTE_VIDEO_FORMAT);
            if (player->video_width == 0 && player->video_height == 0) {
                gm_log(player->debug, G_LOG_LEVEL_INFO,
                       "Setting to minimum size so that mplayer has something to draw to");
                allocation.width = 32;
                allocation.height = 16;
                create_event_allocation(player, "size_allocate", &allocation);
            }
        }

        if (strstr(mplayer_output->str, "ID_VIDEO_CODEC") != 0) {
            g_string_truncate(mplayer_output, mplayer_output->len - 1);
            buf = strstr(mplayer_output->str, "ID_VIDEO_CODEC") + strlen("ID_VIDEO_CODEC=");
            if (player->video_codec != NULL) {
                g_free(player->video_codec);
                player->video_codec = NULL;
            }
            player->video_codec = g_strdup(buf);
            create_event_int(player, "attribute-changed", ATTRIBUTE_VIDEO_CODEC);
        }

        if (strstr(mplayer_output->str, "ID_VIDEO_FPS") != 0) {
            buf = strstr(mplayer_output->str, "ID_VIDEO_FPS");
            sscanf(buf, "ID_VIDEO_FPS=%lf", &player->video_fps);
            create_event_int(player, "attribute-changed", ATTRIBUTE_VIDEO_FPS);
        }

        if (strstr(mplayer_output->str, "ID_VIDEO_BITRATE") != 0) {
            buf = strstr(mplayer_output->str, "ID_VIDEO_BITRATE");
            sscanf(buf, "ID_VIDEO_BITRATE=%i", &player->video_bitrate);
            create_event_int(player, "attribute-changed", ATTRIBUTE_VIDEO_BITRATE);
        }

        if (strstr(mplayer_output->str, "ID_AUDIO_FORMAT") != 0) {
            g_string_truncate(mplayer_output, mplayer_output->len - 1);
            buf = strstr(mplayer_output->str, "ID_AUDIO_FORMAT") + strlen("ID_AUDIO_FORMAT=");
            if (player->audio_format != NULL) {
                g_free(player->audio_format);
                player->audio_format = NULL;
            }
            player->audio_format = g_strdup(buf);
            create_event_int(player, "attribute-changed", ATTRIBUTE_AUDIO_FORMAT);
        }

        if (strstr(mplayer_output->str, "ID_AUDIO_CODEC") != 0) {
            g_string_truncate(mplayer_output, mplayer_output->len - 1);
            buf = strstr(mplayer_output->str, "ID_AUDIO_CODEC") + strlen("ID_AUDIO_CODEC=");
            if (player->audio_codec != NULL) {
                g_free(player->audio_codec);
                player->audio_codec = NULL;
            }
            player->audio_codec = g_strdup(buf);
            create_event_int(player, "attribute-changed", ATTRIBUTE_AUDIO_CODEC);
        }

        if (strstr(mplayer_output->str, "ID_AUDIO_BITRATE") != 0) {
            buf = strstr(mplayer_output->str, "ID_AUDIO_BITRATE");
            sscanf(buf, "ID_AUDIO_BITRATE=%i", &player->audio_bitrate);
            create_event_int(player, "attribute-changed", ATTRIBUTE_AUDIO_BITRATE);
        }

        if (strstr(mplayer_output->str, "ID_AUDIO_RATE") != 0) {
            buf = strstr(mplayer_output->str, "ID_AUDIO_RATE");
            sscanf(buf, "ID_AUDIO_RATE=%i", &player->audio_rate);
            g_signal_emit_by_name(player, "attribute-changed", ATTRIBUTE_AUDIO_RATE);
        }

        if (strstr(mplayer_output->str, "ID_AUDIO_NCH") != 0) {
            buf = strstr(mplayer_output->str, "ID_AUDIO_NCH");
            sscanf(buf, "ID_AUDIO_NCH=%i", &player->audio_nch);
            create_event_int(player, "attribute-changed", ATTRIBUTE_AUDIO_NCH);
        }

        if (strstr(mplayer_output->str, "*** screenshot") != 0) {
            buf = strstr(mplayer_output->str, "'") + 1;
            buf[12] = '\0';
            message = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "Screenshot saved to '%s'"), buf);
            dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO,
                                            GTK_BUTTONS_OK, "%s", message);
            gtk_window_set_title(GTK_WINDOW(dialog), g_dgettext(GETTEXT_PACKAGE, "GNOME MPlayer Notification"));
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            g_free(message);
            message = NULL;
        }

        if (strstr(mplayer_output->str, "failed (forgot -vf screenshot?)") != 0) {
            dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_OK, g_dgettext(GETTEXT_PACKAGE, "Failed to take screenshot"));
            gtk_window_set_title(GTK_WINDOW(dialog), g_dgettext(GETTEXT_PACKAGE, "GNOME MPlayer Notification"));
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }

        if (g_regex_match(player->name_regex, mplayer_output->str, 0, NULL)
            && (g_strrstr(mplayer_output->str, "CPU vendor name:") == NULL)) {
            gm_logs(player->debug, G_LOG_LEVEL_DEBUG, "recognized movie name - updating UI etc");
            split = g_regex_split(player->name_regex, mplayer_output->str, 0);
            index = 0;
            while (split[index]) {
                if (strlen(split[index]) > 0) {
                    if (player->title != NULL) {
                        g_free(player->title);
                        player->title = NULL;
                    }

                    player->title = g_locale_to_utf8(split[index], -1, NULL, NULL, NULL);
                    if (player->title == NULL) {
                        player->title = g_strdup(split[index]);
                        gm_str_strip_unicode(player->title, strlen(player->title));
                    }
                    player->has_metadata = TRUE;
                    create_event_int(player, "attribute-changed", ATTRIBUTE_TITLE);
                }
                index++;
            }
            g_strfreev(split);
        }

        if (g_regex_match(player->genre_regex, mplayer_output->str, 0, NULL)) {
            gm_logs(player->debug, G_LOG_LEVEL_DEBUG, "recognized genre - updating UI etc");
            split = g_regex_split(player->genre_regex, mplayer_output->str, 0);
            index = 0;
            while (split[index]) {
                if (strlen(split[index]) > 0) {
                    if (player->genre != NULL) {
                        g_free(player->genre);
                        player->genre = NULL;
                    }

                    player->genre = g_locale_to_utf8(split[index], -1, NULL, NULL, NULL);
                    if (player->genre == NULL) {
                        player->genre = g_strdup(split[index]);
                        gm_str_strip_unicode(player->genre, strlen(player->genre));
                    }
                    player->has_metadata = TRUE;
                    create_event_int(player, "attribute-changed", ATTRIBUTE_GENRE);
                }
                index++;
            }
            g_strfreev(split);
        }

        if (g_regex_match(player->title_regex, mplayer_output->str, 0, NULL)) {
            gm_logs(player->debug, G_LOG_LEVEL_DEBUG, "recognized title - updating UI etc");
            split = g_regex_split(player->title_regex, mplayer_output->str, 0);
            index = 0;
            while (split[index]) {
                if (strlen(split[index]) > 0) {
                    if (player->title != NULL) {
                        g_free(player->title);
                        player->title = NULL;
                    }

                    player->title = g_locale_to_utf8(split[index], -1, NULL, NULL, NULL);
                    if (player->title == NULL) {
                        player->title = g_strdup(split[index]);
                        gm_str_strip_unicode(player->title, strlen(player->title));
                    }
                    player->has_metadata = TRUE;
                    create_event_int(player, "attribute-changed", ATTRIBUTE_TITLE);
                }
                index++;
            }
            g_strfreev(split);
        }

        if (g_regex_match(player->artist_regex, mplayer_output->str, 0, NULL)) {
            gm_logs(player->debug, G_LOG_LEVEL_DEBUG, "recognized artist - updating UI etc");
            split = g_regex_split(player->artist_regex, mplayer_output->str, 0);
            index = 0;
            while (split[index]) {
                if (strlen(split[index]) > 0) {
                    if (player->artist != NULL) {
                        g_free(player->artist);
                        player->artist = NULL;
                    }

                    player->artist = g_locale_to_utf8(split[index], -1, NULL, NULL, NULL);
                    if (player->artist == NULL) {
                        player->artist = g_strdup(split[index]);
                        gm_str_strip_unicode(player->artist, strlen(player->artist));
                    }
                    player->has_metadata = TRUE;
                    create_event_int(player, "attribute-changed", ATTRIBUTE_ARTIST);
                }
                index++;
            }
            g_strfreev(split);
        }

        if (g_regex_match(player->album_regex, mplayer_output->str, 0, NULL)) {
            gm_logs(player->debug, G_LOG_LEVEL_DEBUG, "recognized album - updating UI etc");
            split = g_regex_split(player->album_regex, mplayer_output->str, 0);
            index = 0;
            while (split[index]) {
                if (strlen(split[index]) > 0) {
                    if (player->album != NULL) {
                        g_free(player->album);
                        player->album = NULL;
                    }

                    player->album = g_locale_to_utf8(split[index], -1, NULL, NULL, NULL);
                    if (player->album == NULL) {
                        player->album = g_strdup(split[index]);
                        gm_str_strip_unicode(player->album, strlen(player->album));
                    }
                    player->has_metadata = TRUE;
                    create_event_int(player, "attribute-changed", ATTRIBUTE_ALBUM);
                }
                index++;
            }
            g_strfreev(split);
        }

        if (player->minimum_mplayer == FALSE) {
            message = g_strdup_printf(g_dgettext(GETTEXT_PACKAGE, "MPlayer should be Upgraded to a Newer Version"));
            dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO,
                                            GTK_BUTTONS_OK, "%s", message);
            gtk_window_set_title(GTK_WINDOW(dialog), g_dgettext(GETTEXT_PACKAGE, "GNOME MPlayer Notification"));
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            g_free(message);
            message = NULL;
            player->minimum_mplayer = TRUE;
        }

        if (strstr(mplayer_output->str, "ICY Info") != NULL) {
            buf = strstr(mplayer_output->str, "'");
            if (message) {
                g_free(message);
                message = NULL;
            }
            if (buf != NULL) {
                for (i = 1; i < (int) strlen(buf) - 1; i++) {
                    if (!strncmp(&buf[i], "\';", 2)) {
                        buf[i] = '\0';
                        break;
                    }
                }
                if (g_ascii_strcasecmp(buf + 1, " - ") != 0) {
                    if (g_utf8_validate(buf + 1, strlen(buf + 1), 0))
                        message = g_markup_printf_escaped("<small>\n\t<big><b>%s</b></big>\n</small>", buf + 1);
                }
            }
            if (message) {
                // reset max values in audio meter
                g_free(message);
                message = g_markup_printf_escaped("\n\t<b>%s</b>\n", buf + 1);
                icy = g_strdup(buf + 1);
                buf = strstr(icy, " - ");
                title = buf + 3;

                if (buf == NULL) {
                    buf = strstr(icy, ":");
                    title = buf + 1;
                }

                if (buf != NULL) {

                    if (player->title)
                        g_free(player->title);
                    player->title = g_strdup(title);
                    create_event_int(player, "attribute-changed", ATTRIBUTE_TITLE);
                    buf[0] = '\0';
                    if (player->artist)
                        g_free(player->artist);
                    player->artist = g_strdup(icy);
                    create_event_int(player, "attribute-changed", ATTRIBUTE_ARTIST);
                    if (player->album)
                        g_free(player->album);
                    player->album = NULL;
                    create_event_int(player, "attribute-changed", ATTRIBUTE_ALBUM);

                }
                g_free(icy);
                icy = NULL;
                g_free(message);
                message = NULL;
            }
        }

        if (strstr(mplayer_output->str, "ID_FILENAME") != NULL && player->has_metadata == FALSE) {
            buf = g_strrstr(mplayer_output->str, ".");
            if (buf)
                buf[0] = '\0';

            buf = g_strrstr(mplayer_output->str, "/");
            icy = g_strdup(buf + 1);
            buf = strstr(icy, " - ");
            title = buf + 3;

            if (buf == NULL) {
                buf = strstr(icy, ":");
                title = buf + 1;
            }

            if (buf != NULL) {

                if (player->title)
                    g_free(player->title);
                player->title = g_strdup(title);
                create_event_int(player, "attribute-changed", ATTRIBUTE_TITLE);
                buf[0] = '\0';
                if (player->artist)
                    g_free(player->artist);
                player->artist = g_strdup(icy);
                create_event_int(player, "attribute-changed", ATTRIBUTE_ARTIST);
                if (player->album)
                    g_free(player->album);
                player->album = NULL;
                create_event_int(player, "attribute-changed", ATTRIBUTE_ALBUM);

            }
            g_free(icy);
            icy = NULL;
            g_free(message);
            message = NULL;
        }

        if (strstr(mplayer_output->str, "ID_SIGNAL") != 0) {
            if (player->position == 0) {
                player->playback_error = ERROR_RETRY;
            }
        }

    }

    g_string_free(mplayer_output, TRUE);
    return TRUE;
}

// this executes in the main thread
gboolean thread_query(gpointer data)
{
    GmtkMediaPlayer *player = GMTK_MEDIA_PLAYER(data);
    gint written;

    // gm_log(player->debug, G_LOG_LEVEL_DEBUG, "in thread_query, data = %p", data);
    if (player == NULL) {
        gm_log(player->debug, G_LOG_LEVEL_DEBUG, "thread_query called with player == NULL");
        finalize_mplayer(player);
        return FALSE;
    }

    if (player->player_state == PLAYER_STATE_RUNNING) {
        if (player->media_state == MEDIA_STATE_PLAY) {
            // gm_log(player->debug, G_LOG_LEVEL_DEBUG, "writing");
            if (player->use_mplayer2) {
                written = write(player->std_in, "get_time_pos\n", strlen("get_time_pos\n"));
            } else {
                written =
                    write(player->std_in, "pausing_keep_force get_time_pos\n",
                          strlen("pausing_keep_force get_time_pos\n"));
            }
            // gm_log(player->debug, G_LOG_LEVEL_DEBUG, "written = %i", written);
            if (written == -1) {
                //return TRUE;
                gm_log(player->debug, G_LOG_LEVEL_INFO, "thread_query, write failed");
                return FALSE;
            } else {
                return TRUE;
            }
        } else {
            return TRUE;
        }
    } else {
        gm_log(player->debug, G_LOG_LEVEL_DEBUG, "thread_query, player is dead");
        finalize_mplayer(player);
        return FALSE;
    }
}

gboolean write_to_mplayer(GmtkMediaPlayer * player, const gchar * cmd)
{
    GIOStatus result;
    gsize bytes_written;
    gchar *pkf_cmd;

    /* ending \n is part of cmd */
    gm_logsp(player->debug, G_LOG_LEVEL_DEBUG, ">", cmd);

    if (player->channel_in) {
        if (player->use_mplayer2) {
            pkf_cmd = g_strdup(cmd);
        } else {
            /* if cmd starts with "pause" (non case sensitive) */
            if (g_ascii_strncasecmp(cmd, "pause", strlen("pause")) == 0) {
                pkf_cmd = g_strdup(cmd);
            } else {
                pkf_cmd = g_strdup_printf("pausing_keep_force %s", cmd);
            }
        }
        result = g_io_channel_write_chars(player->channel_in, pkf_cmd, -1, &bytes_written, NULL);
        g_free(pkf_cmd);
        if (result != G_IO_STATUS_ERROR && bytes_written > 0) {
            result = g_io_channel_flush(player->channel_in, NULL);
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }

}

gboolean detect_mplayer_features(GmtkMediaPlayer * player)
{
    gchar *av[255];
    gint ac = 0, i;
    gchar **output;
    GError *error;
    gint exit_status;
    gchar *out = NULL;
    gchar *err = NULL;
    gboolean ret = TRUE;

    if (player->features_detected)
        return ret;

    if (player->mplayer_binary == NULL || !g_file_test(player->mplayer_binary, G_FILE_TEST_EXISTS)) {
        av[ac++] = g_strdup_printf("mplayer");
    } else {
        av[ac++] = g_strdup_printf("%s", player->mplayer_binary);
    }
    av[ac++] = g_strdup_printf("-noidle");
    av[ac++] = g_strdup_printf("-softvol");
    av[ac++] = g_strdup_printf("-volume");
    av[ac++] = g_strdup_printf("100");
    av[ac++] = g_strdup_printf("-nostop-xscreensaver");

    // enable these lines to force newer mplayer
    //av[ac++] = g_strdup_printf("-gamma");
    //av[ac++] = g_strdup_printf("0");

    av[ac] = NULL;

    error = NULL;

    g_spawn_sync(NULL, av, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &out, &err, &exit_status, &error);

    for (i = 0; i < ac; i++) {
        g_free(av[i]);
    }

    if (error != NULL) {
        gm_log(player->debug, G_LOG_LEVEL_MESSAGE, "Error when running: %s", error->message);
        g_error_free(error);
        error = NULL;
        if (out != NULL) {
            g_free(out);
            out = NULL;
        }
        if (err != NULL) {
            g_free(err);
            err = NULL;
        }
        return FALSE;
    }
    output = g_strsplit(out, "\n", 0);
    ac = 0;
    while (output[ac] != NULL) {
        /* if output[ac] starts with "Unknown option" (non case sensitive) */
        if (g_ascii_strncasecmp(output[ac], "Unknown option", strlen("Unknown option")) == 0) {
            ret = FALSE;
        }
        /* if output[ac] starts with "MPlayer2" (non case sensitive) */
        if (g_ascii_strncasecmp(output[ac], "MPlayer2", strlen("MPlayer2")) == 0) {
            player->use_mplayer2 = TRUE;
        }
        ac++;
    }
    g_strfreev(output);
    g_free(out);
    out = NULL;
    g_free(err);
    err = NULL;

    player->features_detected = TRUE;
    if (!ret) {
        gm_log(player->debug, G_LOG_LEVEL_MESSAGE,
               g_dgettext(GETTEXT_PACKAGE, "You might want to consider upgrading mplayer to a newer version"));
    }
    return ret;
}
