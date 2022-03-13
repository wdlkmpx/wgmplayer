/*
 * This is free and unencumbered software released into the public domain.
 *
 * For more information, please refer to <https://unlicense.org>
 */

#ifndef __W_GTK_H__
#define __W_GTK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "gtkcompat.h"

#ifndef GTK_APPLICATION
#define GtkApplication void
#endif

GtkWidget * w_gtk_window_new (const char * title,
                              GtkWindow * parent,
                              GtkApplication * app, // gtkcompat.h < 3 = `void * app`
                              gboolean resizable);

GtkWidget * w_gtk_dialog_new (const char * title,
                              GtkWindow * parent,
                              gboolean resizable,
                              GtkWidget ** main_vbox); /* out */

GtkWidget * w_gtk_frame_vbox_new (char * label,
                                  GtkWidget * parent_box,
                                  int children_spacing,
                                  int box_padding);

GtkWidget * w_gtk_button_new (const char * label,
                              const char * icon_name,
                              gpointer clicked_cb,
                              gpointer cdata);

GtkWidget * w_gtk_image_new_from_icon_name (const char *icon_name, GtkIconSize size);
void w_gtk_image_set_from_icon_name (GtkImage *img, const char *icon_name, GtkIconSize size);
void w_gtk_button_set_icon_name (GtkButton *button, const char *icon_name);

void w_gtk_glist_to_combo (GtkComboBox *combo, GList *strings, int default_index);
void w_gtk_strv_to_combo (GtkComboBox *combo, char **strv, int default_index);

#if ! GTK_CHECK_VERSION (3, 0, 0)
void gtk_widget_set_halign (GtkWidget *widget, GtkAlign align);
void gtk_widget_set_valign (GtkWidget *widget, GtkAlign align);
void gtk_widget_set_margin_start  (GtkWidget *widget, gint margin);
#define gtk_widget_set_margin_end(widget,margin) 
void gtk_widget_set_margin_top (GtkWidget *widget, gint margin);
#define gtk_widget_set_margin_bottom(widget,margin)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __W_GTK_H__ */
