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

GtkWidget * w_gtk_notebook_add_tab (GtkWidget * notebook, char * label_str);
void w_gtk_widget_change_tooltip (GtkWidget *widget, const char *new_text);

int  w_gtk_tree_view_get_num_selected (GtkWidget *tv);
void w_gtk_tree_view_clear (GtkWidget *tv);
void w_gtk_tree_view_select_all (GtkWidget *tv);
void w_gtk_tree_view_deselect_all (GtkWidget *tv);
void w_gtk_tree_view_select_row (GtkWidget *tv, int n);

void w_gtk_combo_box_populate_from_glist (GtkWidget *combo, GList *strings, int default_index);
void w_gtk_combo_box_populate_from_strv (GtkWidget *combo,
                                         const char **strv,
                                         int default_index,
                                         gboolean gettext);

int w_gtk_combo_box_get_count (GtkWidget *combo);
void w_gtk_combo_box_find_and_select (GtkWidget *combo, char *str);
gboolean w_gtk_combo_box_find_str (GtkWidget *combo, const char *str,
                                   gboolean select,
                                   GtkTreeIter *out_iter);
void w_gtk_combo_box_select_or_prepend (GtkWidget *combo, const char *str);
void w_gtk_combo_box_set_w_model (GtkWidget *combo, gboolean sort);


#if GTK_MAJOR_VERSION >= 3
#define w_gtk_widget_add_alignment(widget) (widget)
#else
// GTK <= 2
GtkWidget * w_gtk_widget_add_alignment (GtkWidget *widget);
void gtk_widget_set_halign (GtkWidget *widget, GtkAlign align);
void gtk_widget_set_valign (GtkWidget *widget, GtkAlign align);
void gtk_widget_set_margin_start (GtkWidget *widget, gint margin);
void gtk_widget_set_margin_end (GtkWidget *widget, gint margin);
// deprecated since 3.12+: gtk_widget_set_margin_left() / gtk_widget_set_margin_right()
#define gtk_widget_set_margin_left gtk_widget_set_margin_start
#define gtk_widget_set_margin_right gtk_widget_set_margin_end
void gtk_widget_set_margin_bottom (GtkWidget *widget, gint margin);
void gtk_widget_set_margin_top (GtkWidget *widget, gint margin);
void gtk_grid_attach (GtkGrid *grid, GtkWidget *child, gint left, gint top, gint width, gint height);
// GTK < 2.24
#if !GTK_CHECK_VERSION(2, 24, 0)
gboolean gtk_combo_box_get_has_entry (GtkComboBox *combo_box);
#endif
#endif


// =================================================
//            GtkGrid / GtkTable
// =================================================

typedef struct _WGtkAlignmentParams
{
    GtkWidget *w;
    GtkAlign align;
    int margin_start;
    int margin_end;
    int margin_top;
    int margin_bottom;
    int width;
} WGtkAlignmentParams;

typedef struct _WGtkGridParams
{
    int cols;
    int row;
    GtkWidget *table;
    WGtkAlignmentParams c1;
    WGtkAlignmentParams c2;
    WGtkAlignmentParams c3;
    WGtkAlignmentParams c4;
} WGtkGridParams;

#define WGtkTableParams WGtkGridParams

GtkWidget * w_gtk_grid_new (GtkWidget *box);
#define w_gtk_table_new w_gtk_grid_new
void w_gtk_grid_append_row (WGtkGridParams *t);
#define w_gtk_table_append_row(WGtkTableParams) w_gtk_table_append_row(WGtkTableParams)

// =================================================

#ifdef __cplusplus
}
#endif

#endif /* __W_GTK_H__ */
