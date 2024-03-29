/*
 * gmtk_output_combo_box.c
 * Copyright (C) Kevin DeKorte 2009 <kdekorte@gmail.com>
 * 
 * gmtk_output_combo_box.c is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * gmtk_output_combo_box.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 */

#include "gmtk_output_combo_box.h"
#include "gmtk_common.h"
#include "gm_log.h"

static GObjectClass *parent_class = NULL;

gint sort_iter_compare_func(GtkTreeModel * model, GtkTreeIter * a, GtkTreeIter * b, gpointer data)
{
    int sortcol = GPOINTER_TO_INT(data); // must be text
    int ret = 0;
    char *a_desc;
    char *b_desc;

    gtk_tree_model_get (model, a, sortcol, &a_desc, -1);
    gtk_tree_model_get (model, b, sortcol, &b_desc, -1);

    ret = g_strcmp0 (a_desc, b_desc);
    g_free(a_desc);
    g_free(b_desc);

    return ret;
}


#ifdef HAVE_PULSEAUDIO
void pa_sink_cb(pa_context * c, const pa_sink_info * i, int eol, gpointer data)
{
    GmtkOutputComboBox *output = GMTK_OUTPUT_COMBO_BOX(data);
    GtkTreeIter iter;
    gchar *name;
    gchar *device;
    if (i) {
        name = g_strdup_printf("%s (PulseAudio)", i->description);
        device = g_strdup_printf("pulse::%i", i->index);
        gtk_list_store_append(output->list, &iter);
        gtk_list_store_set(output->list, &iter,
                           OUTPUT_TYPE_COLUMN, OUTPUT_TYPE_PULSE,
                           OUTPUT_DESCRIPTION_COLUMN, name,
                           OUTPUT_CARD_COLUMN, -1,
                           OUTPUT_DEVICE_COLUMN, -1,
                           OUTPUT_INDEX_COLUMN, i->index,
                           OUTPUT_MPLAYER_DEVICE_COLUMN, device, -1);
        g_free(device);
        g_free(name);
    }
}

void context_state_callback(pa_context * context, gpointer data)
{
    int i;

    gm_log(FALSE, G_LOG_LEVEL_DEBUG, "context state callback");

    switch (pa_context_get_state(context)) {
    case PA_CONTEXT_READY:{
            for (i = 0; i < 255; i++) {
                pa_context_get_sink_info_by_index(context, i, pa_sink_cb, data);
            }
        }
    default:
        return;
    }
}
#endif



G_DEFINE_TYPE(GmtkOutputComboBox, gmtk_output_combo_box, GTK_TYPE_COMBO_BOX);

static void gmtk_output_combo_box_finalize(GObject * object)
{
    //fprintf(stdout, "gmtk_output_combo_box_finalize: %p\n", object);
    GmtkOutputComboBox *output = GMTK_OUTPUT_COMBO_BOX(object);

    if (output->list) {
        gtk_list_store_clear(output->list);
        g_object_unref(output->list);
        output->list = NULL;
    }
    G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void gmtk_output_combo_box_class_init(GmtkOutputComboBoxClass * class)
{
    GObjectClass *oc = G_OBJECT_CLASS(class);
    oc->finalize = gmtk_output_combo_box_finalize;
    //GtkWidgetClass *wc = GTK_WIDGET_CLASS(class);

    parent_class = g_type_class_peek_parent(class);
}


static void gmtk_output_combo_box_init(GmtkOutputComboBox * output)
{
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkTreeSortable *sortable;

    gint card, err, dev;
    gchar *name = NULL;
    gchar *menu;
    gchar *mplayer_device;

#ifdef HAVE_ASOUNDLIB
    snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
    snd_ctl_t *handle;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
#endif

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(output), renderer, FALSE);
    gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(output), renderer, "text", 0);

    output->list =
        gtk_list_store_new(OUTPUT_N_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
                           G_TYPE_STRING);

    gtk_list_store_append(output->list, &iter);
    gtk_list_store_set(output->list, &iter,
                       OUTPUT_DESCRIPTION_COLUMN, _("Default"),
                       OUTPUT_TYPE_COLUMN, OUTPUT_TYPE_SOFTVOL,
                       OUTPUT_CARD_COLUMN, -1,
                       OUTPUT_DEVICE_COLUMN, -1,
                       OUTPUT_MPLAYER_DEVICE_COLUMN, "", -1);

    gtk_list_store_append(output->list, &iter);
    gtk_list_store_set(output->list, &iter,
                       OUTPUT_DESCRIPTION_COLUMN, "JACK",
                       OUTPUT_TYPE_COLUMN, OUTPUT_TYPE_SOFTVOL,
                       OUTPUT_CARD_COLUMN, -1,
                       OUTPUT_DEVICE_COLUMN, -1,
                       OUTPUT_MPLAYER_DEVICE_COLUMN, "jack", -1);
#ifndef __linux__
    gtk_list_store_append(output->list, &iter);
    gtk_list_store_set(output->list, &iter,
                       OUTPUT_DESCRIPTION_COLUMN, "OSS",
                       OUTPUT_TYPE_COLUMN, OUTPUT_TYPE_SOFTVOL,
                       OUTPUT_CARD_COLUMN, -1,
                       OUTPUT_DEVICE_COLUMN, -1,
                       OUTPUT_MPLAYER_DEVICE_COLUMN, "oss", -1);
#endif

    gtk_list_store_append(output->list, &iter);
    gtk_list_store_set(output->list, &iter,
                       OUTPUT_DESCRIPTION_COLUMN, "ALSA",
                       OUTPUT_TYPE_COLUMN, OUTPUT_TYPE_SOFTVOL,
                       OUTPUT_CARD_COLUMN, -1,
                       OUTPUT_DEVICE_COLUMN, -1,
                       OUTPUT_MPLAYER_DEVICE_COLUMN, "alsa", -1);

    gtk_list_store_append(output->list, &iter);
    gtk_list_store_set(output->list, &iter,
                       OUTPUT_DESCRIPTION_COLUMN, "PulseAudio",
                       OUTPUT_TYPE_COLUMN, OUTPUT_TYPE_SOFTVOL,
                       OUTPUT_CARD_COLUMN, -1,
                       OUTPUT_DEVICE_COLUMN, -1,
                       OUTPUT_INDEX_COLUMN, -1,
                       OUTPUT_MPLAYER_DEVICE_COLUMN, "pulse", -1);

#ifdef HAVE_ASOUNDLIB
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    card = -1;

    while (snd_card_next(&card) >= 0) {
        gm_log(FALSE, G_LOG_LEVEL_DEBUG, "card = %i", card);
        if (card < 0)
            break;
        if (name != NULL) {
            free(name);
            name = NULL;
        }
        name = malloc(32);
        sprintf(name, "hw:%i", card);
        gm_log(FALSE, G_LOG_LEVEL_DEBUG, "name = %s", name);

        if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
            continue;
        }

        if ((err = snd_ctl_card_info(handle, info)) < 0) {
            snd_ctl_close(handle);
            continue;
        }

        dev = -1;
        while (1) {
            snd_ctl_pcm_next_device(handle, &dev);
            if (dev < 0)
                break;
            snd_pcm_info_set_device(pcminfo, dev);
            snd_pcm_info_set_subdevice(pcminfo, 0);
            snd_pcm_info_set_stream(pcminfo, stream);
            if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                continue;
            }

            menu = g_strdup_printf("%s (%s) (alsa)", snd_ctl_card_info_get_name(info), snd_pcm_info_get_name(pcminfo));
            mplayer_device = g_strdup_printf("alsa:device=hw=%i.%i", card, dev);

            gtk_list_store_append(output->list, &iter);
            gtk_list_store_set(output->list, &iter,
                               OUTPUT_DESCRIPTION_COLUMN, menu,
                               OUTPUT_TYPE_COLUMN, OUTPUT_TYPE_ALSA,
                               OUTPUT_CARD_COLUMN, card,
                               OUTPUT_DEVICE_COLUMN, dev,
                               OUTPUT_MPLAYER_DEVICE_COLUMN, mplayer_device, -1);
        }
        snd_ctl_close(handle);
    }
#endif

#ifdef __OpenBSD__
    gtk_list_store_append(output->list, &iter);
    gtk_list_store_set(output->list, &iter,
                       OUTPUT_DESCRIPTION_COLUMN, "SNDIO",
                       OUTPUT_TYPE_COLUMN, OUTPUT_TYPE_SOFTVOL,
                       OUTPUT_CARD_COLUMN, -1,
                       OUTPUT_DEVICE_COLUMN, -1,
                       OUTPUT_MPLAYER_DEVICE_COLUMN, "sndio", -1);
#endif

#ifdef HAVE_PULSEAUDIO
    pa_glib_mainloop *loop = pa_glib_mainloop_new(g_main_context_default());
    pa_context *context = pa_context_new(pa_glib_mainloop_get_api(loop), "gmtk context");
    if (context) {
        pa_context_connect(context, NULL, 0, NULL);
        pa_context_set_state_callback(context, context_state_callback, output);
    }
    // make sure the pulse events are done before we exit this function
    gm_log(FALSE, G_LOG_LEVEL_DEBUG, "waiting for all events to drain");
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
#endif

    sortable = GTK_TREE_SORTABLE(output->list);
    gtk_tree_sortable_set_sort_func(sortable, OUTPUT_DESCRIPTION_COLUMN,
                                    sort_iter_compare_func,
                                    GINT_TO_POINTER(OUTPUT_DESCRIPTION_COLUMN), NULL);
    gtk_tree_sortable_set_sort_column_id(sortable, OUTPUT_DESCRIPTION_COLUMN, GTK_SORT_ASCENDING);

    gtk_combo_box_set_model(GTK_COMBO_BOX(output), GTK_TREE_MODEL(output->list));
}


GtkWidget *gmtk_output_combo_box_new()
{
    GtkWidget *output;
    output = g_object_new(GMTK_TYPE_OUTPUT_COMBO_BOX, NULL);
    return output;
}


const gchar *gmtk_output_combo_box_get_active_device(GmtkOutputComboBox * output)
{
    GtkTreeIter iter;
    const gchar *device = NULL;

    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(output), &iter)) {
        gtk_tree_model_get(GTK_TREE_MODEL(output->list), &iter, OUTPUT_MPLAYER_DEVICE_COLUMN, &device, -1);
    }
    return device;
}


const gchar *gmtk_output_combo_box_get_active_description(GmtkOutputComboBox * output)
{
    GtkTreeIter iter;
    const gchar *desc = NULL;

    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(output), &iter)) {
        gtk_tree_model_get(GTK_TREE_MODEL(output->list), &iter, OUTPUT_DESCRIPTION_COLUMN, &desc, -1);
    }
    return desc;
}


GmtkOutputType gmtk_output_combo_box_get_active_type(GmtkOutputComboBox * output)
{
    GtkTreeIter iter;
    GmtkOutputType type = OUTPUT_TYPE_SOFTVOL;

    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(output), &iter)) {
        gtk_tree_model_get(GTK_TREE_MODEL(output->list), &iter, OUTPUT_TYPE_COLUMN, &type, -1);
    }
    return type;
}


gint gmtk_output_combo_box_get_active_card(GmtkOutputComboBox * output)
{
    GtkTreeIter iter;
    gint card;

    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(output), &iter)) {
        gtk_tree_model_get(GTK_TREE_MODEL(output->list), &iter, OUTPUT_CARD_COLUMN, &card, -1);
    }
    return card;
}


gint gmtk_output_combo_box_get_active_index(GmtkOutputComboBox * output)
{
    GtkTreeIter iter;
    gint index;

    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(output), &iter)) {
        gtk_tree_model_get(GTK_TREE_MODEL(output->list), &iter, OUTPUT_INDEX_COLUMN, &index, -1);
    }
    return index;
}
