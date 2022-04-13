/*
 * This is free and unencumbered software released into the public domain.
 *
 * For more information, please refer to <https://unlicense.org>
 */

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "w_gtk.h"
#include <stdio.h>
#include <string.h>

GtkWidget * w_gtk_window_new (const char * title,
                              GtkWindow * parent,
                              GtkApplication * app, // gtkcompat.h < 3 = `void * app`
                              gboolean resizable)
{
    GtkWidget * window;
    if (app) {
#if GTK_CHECK_VERSION (3, 4, 0)
        window = gtk_application_window_new (app);
#else
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
#endif
    } else {
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    }
    gtk_window_set_title (GTK_WINDOW (window), title);
    gtk_container_set_border_width (GTK_CONTAINER (window), 4); /* padding */
    if (parent) {
        gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (parent));
        gtk_window_set_modal (GTK_WINDOW (window), TRUE);
        gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
        gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);
        gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
        gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    }
    if (!resizable) {
        // no need to call this if TRUE, unexpected behavior in GTK3 IIRC
        gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    }

    return window;
}


GtkWidget * w_gtk_dialog_new (const char * title,
                              GtkWindow * parent,
                              gboolean resizable,
                              GtkWidget ** main_vbox) /* out */
{
    GtkWidget * dialog;
    dialog = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (dialog), title);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 4); /* padding */
    if (parent) {
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
        gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
        gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
        gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
        gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
    }
#if GTK_MAJOR_VERSION >= 3 // always need a parent window
    else {
        GtkWidget * parent = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));
        gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
    }
#endif
    if (!resizable) {
        // no need to call this if TRUE, unexpected behavior in GTK3 IIRC
        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
    }

    if (main_vbox) {
        *main_vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
        /* padding */
        gtk_container_set_border_width (GTK_CONTAINER (*main_vbox), 4);
    }

    return dialog;
}


GtkWidget * w_gtk_frame_vbox_new (char * label,
                                  GtkWidget * parent_box,
                                  int children_spacing,
                                  int box_padding)
{
    // returns a vbox inside a frame
    GtkWidget * frame = gtk_frame_new (label);
    gtk_box_pack_start (GTK_BOX (parent_box), frame, FALSE, FALSE, 0);

    GtkWidget * frame_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, children_spacing);
    gtk_container_add (GTK_CONTAINER (frame), frame_vbox);
    if (box_padding) {
        /* padding */
        gtk_container_set_border_width (GTK_CONTAINER (frame_vbox), box_padding);
    }
    return frame_vbox;
}


GtkWidget * w_gtk_image_new_from_icon_name (const char *icon_name, GtkIconSize size)
{
    GtkWidget *img = NULL;
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
    if (gtk_icon_theme_has_icon (icon_theme, icon_name)) {
        img = gtk_image_new_from_icon_name (icon_name, size);
    } else if (strncmp (icon_name, "gtk-", 4) == 0) {
        img = gtk_image_new_from_stock (icon_name, size);
    } else {
        // get blank/invalid image
        fprintf (stderr, "(GtkImage) %s was not found in icon theme\n", icon_name);
        img = gtk_image_new_from_icon_name (icon_name, size);
    }
    return img;
}


void w_gtk_image_set_from_icon_name (GtkImage *img, const char *icon_name, GtkIconSize size)
{
    if (!img) {
        return;
    }
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
    if (gtk_icon_theme_has_icon (icon_theme, icon_name)) {
        gtk_image_set_from_icon_name (img, icon_name, size);
    } else if (strncmp (icon_name, "gtk-", 4) == 0) {
        gtk_image_set_from_stock (img, icon_name, size);
    } else {
        // set blank/invalid image
        fprintf (stderr, "%s was not found in icon theme\n", icon_name);
        gtk_image_set_from_icon_name (img, icon_name, size);
    }
}


void w_gtk_button_set_icon_name (GtkButton *button, const char *icon_name)
{
    GtkWidget *img;
    img = w_gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image (button, img);
}


GtkWidget * w_gtk_button_new (const char * label,
                              const char * icon_name,
                              gpointer clicked_cb,
                              gpointer cdata)
{
    GtkWidget * button;
    if (label) {
        button = gtk_button_new_with_mnemonic (label);
    } else {
        button = gtk_button_new ();
    }
    if (icon_name) {
        w_gtk_button_set_icon_name (GTK_BUTTON(button), icon_name);
    }
    if (clicked_cb) {
        // void button_cb (GtkButton *button, gpointer user_data)
        g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (clicked_cb), cdata);
    }
    gtk_widget_set_can_default (button, TRUE);
    return button;
}


GtkWidget * w_gtk_notebook_add_tab (GtkWidget * notebook, char * label_str)
{
    // returns GtkGrid / GtkTable
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *table;
    vbox  = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    label = gtk_label_new_with_mnemonic (label_str);
    table = w_gtk_grid_new (vbox);
    gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, label);
    return table;
}

/* ================================================== */
/*               GtkTable / GtkGrid                   */
/* ================================================== */

GtkWidget * w_gtk_grid_new (GtkWidget *box)
{
    // new GtkGrid or a GtkTable that must be resized
    GtkWidget * table;
    //table = gtkcompat_grid_new (1, 1);
#if GTK_MAJOR_VERSION >= 3
    table = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID(table), 5);
    //gtk_grid_set_row_spacing (GTK_GRID(table), 3);
#else
    table = gtk_table_new (1, 1, FALSE);
    gtk_table_set_col_spacings (GTK_TABLE(table), 5);
    //gtk_table_set_row_spacings (GTK_TABLE(table), 3);
#endif
    if (box) {
        //gtk_box_set_homogeneous (GTK_BOX(box), TRUE);
        gtk_container_set_border_width (GTK_CONTAINER (box), 5);
        // FALSE, FALSE is required for GtkTable, otherwise
        // the widgets height may be incorrect
        gtk_box_pack_start (GTK_BOX(box), table, FALSE, FALSE, 0);
    }
    return table;
}


static void set_walignment (WGtkAlignmentParams *col)
{
    GtkAlign align = col->align;
    if (!col->w) {
        return;
    }
#if GTK_MAJOR_VERSION >= 3
    // GtkGrid: columns don't have a proper width, this doesn't happen with GtkTable
    //   specially noticeable in GtkNotebook tabs
    gtk_widget_set_hexpand (col->w, TRUE);
#endif
    gtk_widget_set_halign (col->w, col->align);
    if (col->margin_start)  gtk_widget_set_margin_start (col->w, col->margin_start);
    if (col->margin_end)    gtk_widget_set_margin_end   (col->w, col->margin_end);
    if (col->margin_top)    gtk_widget_set_margin_top   (col->w, col->margin_top);
    if (col->margin_bottom) gtk_widget_set_margin_bottom(col->w, col->margin_bottom);
    memset (col, 0, sizeof(*col));
    // keep align
    col->align = align;
}

void w_gtk_grid_append_row (WGtkTableParams *t)
{
    int ncols;
#if GTK_MAJOR_VERSION <= 2
    // resize GtkTable, it should be created with 1_row 1_col
    gtk_table_resize (GTK_TABLE(t->table), t->row+1, t->cols);
    // use GtkAlignment to emulate some gtk3 functions (gtk_widget_set_halign)
    t->c1.w = w_gtk_widget_add_alignment (t->c1.w);
    t->c2.w = w_gtk_widget_add_alignment (t->c2.w);
    t->c3.w = w_gtk_widget_add_alignment (t->c3.w);
    t->c4.w = w_gtk_widget_add_alignment (t->c4.w);
#endif
    if (!t->c1.w && !t->c2.w && !t->c3.w && !t->c4.w) {
        t->c1.w = gtk_label_new ("");
    }
    if (t->cols == 1)
    {
        gtk_grid_attach (GTK_GRID(t->table), t->c1.w, 0, t->row, 1, 1);
    }
    else if (t->cols == 2)
    {
        if (t->c1.w && t->c2.w) {
            gtk_grid_attach (GTK_GRID(t->table), t->c1.w, 0, t->row, 1, 1);
            gtk_grid_attach (GTK_GRID(t->table), t->c2.w, 1, t->row, 1, 1);
        } else if (t->c1.w && !t->c2.w) {
            ncols = t->c1.width ? t->c1.width : 2;
            gtk_grid_attach (GTK_GRID(t->table), t->c1.w, 0, t->row, ncols, 1);
        } else if (t->c2.w) {
            // only 2nd column
            gtk_grid_attach (GTK_GRID(t->table), t->c2.w, 1, t->row, 1, 1);
        }
    }
    else if (t->cols == 3 || t->cols == 4)
    {
        if (t->c1.w) {
            gtk_grid_attach (GTK_GRID(t->table), t->c1.w, 0, t->row, 1, 1);
        }
        if (t->c2.w) {
            gtk_grid_attach (GTK_GRID(t->table), t->c2.w, 1, t->row, 1, 1);
        }
        if (t->c3.w) {
            gtk_grid_attach (GTK_GRID(t->table), t->c3.w, 2, t->row, 1, 1);
        }
        if (t->cols == 4) {
            if (t->c4.w) {
                gtk_grid_attach (GTK_GRID(t->table), t->c4.w, 3, t->row, 1, 1);
            }
        }
    }
    else
    {
        fprintf (stderr, "w_gtk_grid_append_row(): no more than 4 rows are supported");
    }
    set_walignment (&(t->c1));
    set_walignment (&(t->c2));
    set_walignment (&(t->c3));
    set_walignment (&(t->c4));
    // increase current row
    t->row = t->row + 1;
}


void w_gtk_widget_change_tooltip (GtkWidget *widget, const char *new_text)
{ /* changes widget tooltip only if the new_text is different */
    char *tip;
    tip = gtk_widget_get_tooltip_text (widget);
    if (!tip || g_strcmp0(tip,new_text) != 0) {
        gtk_widget_set_tooltip_text (widget, new_text);
    }
    g_free (tip);
}


/* ================================================== */
/*                  TREE VIEW                         */
/* ================================================== */

int w_gtk_tree_view_get_num_selected (GtkWidget *tv)
{
    GtkTreeView      *tree = GTK_TREE_VIEW (tv);
    GtkTreeSelection *tsel = gtk_tree_view_get_selection (tree);
    int              count = gtk_tree_selection_count_selected_rows (tsel);
    return count;
}


void w_gtk_tree_view_clear_list (GtkWidget *tv)
{
    GtkTreeView  *tree  = GTK_TREE_VIEW (tv);
    GtkListStore *store = GTK_LIST_STORE (gtk_tree_view_get_model (tree));
    gtk_list_store_clear (store);
}


void w_gtk_tree_view_clear_tree (GtkWidget *tv)
{
    GtkTreeView  *tree  = GTK_TREE_VIEW (tv);
    GtkTreeStore *store = GTK_TREE_STORE (gtk_tree_view_get_model (tree));
    gtk_tree_store_clear (store);
}


void w_gtk_tree_view_select_all (GtkWidget *tv)
{
    GtkTreeView      *tree = GTK_TREE_VIEW (tv);
    GtkTreeSelection *tsel = gtk_tree_view_get_selection (tree);
    gtk_tree_selection_select_all (tsel);
}


void w_gtk_tree_view_deselect_all (GtkWidget *tv)
{
    GtkTreeView      *tree = GTK_TREE_VIEW (tv);
    GtkTreeSelection *tsel = gtk_tree_view_get_selection (tree);
    gtk_tree_selection_unselect_all (tsel);
}


void w_gtk_tree_view_select_row (GtkWidget *tv, int n)
{
    GtkTreeView      *tree  = GTK_TREE_VIEW (tv);
    GtkTreeSelection *tsel  = gtk_tree_view_get_selection (tree);
    GtkTreePath      *tpath = gtk_tree_path_new_from_indices (n, -1);
    gtk_tree_selection_select_path (tsel, tpath);
    gtk_tree_path_free (tpath);
}

/* ================================================== */
/*                  COMBO BOX                         */
/* ================================================== */


void w_gtk_glist_to_combo (GtkComboBox *combo, GList *strings, int default_index)
{
    GList * list;
    char * text;
    int len = 0;
    gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (combo));
    if (!strings) {
        return; // nothing to add
    }
    for (list = strings;  list;  list = list->next)
    {
        text = (char *) list->data;
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), text);
        len++;
    }
    if (default_index >= len || default_index < 0) {
        default_index = 0;
    }
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), default_index);
}


void w_gtk_strv_to_combo (GtkComboBox *combo, char **strv, int default_index)
{
    int i;
    GtkListStore * store;
    GtkTreeIter iter;
    store = GTK_LIST_STORE (gtk_combo_box_get_model (combo));
    gtk_list_store_clear (store);
    //gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (combo));
    if (!strv || !*strv) {
        return; /* nothing to add */
    }
    for (i = 0; strv[i]; i++) {
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, strv[i], -1);
        //gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), strv[i]);
    }
    if (default_index >= i || default_index < 0) {
        default_index = 0;
    }
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), default_index);
}


int w_gtk_combo_box_get_count (GtkComboBox *combo)
{
    GtkTreeModel * model;
    GtkTreeIter iter;
    gboolean valid;
    int count = 0;
    model = gtk_combo_box_get_model (combo);
    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid) {
        count++;
        valid = gtk_tree_model_iter_next (model, &iter);
    }
    return count;
}


void w_gtk_combo_box_find_and_select (GtkComboBox *combo, char *str)
{
    GtkTreeModel * model;
    GtkTreeIter iter;
    gboolean valid;
    char *value;
    if (!str) {
        return;
    }
    model = gtk_combo_box_get_model (combo);
    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid) {
        gtk_tree_model_get (model, &iter, 0, &value, -1);
        if (g_strcmp0 (value, str) == 0) {
            gtk_combo_box_set_active_iter (combo, &iter);
            break;
        }
        valid = gtk_tree_model_iter_next (model, &iter);
    }
}


/* ================================================== */
/*                   GTK < 3.0                        */
/* ================================================== */

#if ! GTK_CHECK_VERSION (3, 0, 0)

GtkWidget * w_gtk_widget_add_alignment (GtkWidget *widget)
{
    // this will be handy to emulate some gtk3 functions (gtk_widget_set_halign, etc)
    // use g_object_get_data(widget,"alignment") to get the alignment to apply properties
    if (!widget) {
        return NULL;
    }
    GtkWidget *alignment; /* GtkAlignment */
    alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
    gtk_container_add (GTK_CONTAINER(alignment), widget);
    g_object_set_data (G_OBJECT(widget), "alignment", alignment);
    return alignment;
}


static void misc_set_alignment (GtkWidget *widget, GtkAlign align, gboolean horizontal)
{
    // only GtkLabel/GtkArrow/GtkImage/GtkPixmap support this feature
    GtkMisc *misc = GTK_MISC (widget);
    gfloat *misc_align = horizontal ? &(misc->xalign) : &(misc->yalign);
    char *align_str    = horizontal ? "xalign" : "yalign";
    //--
    gfloat walign = 0.0;
    if   (align == GTK_ALIGN_CENTER) walign = 0.5;
    else if (align == GTK_ALIGN_END) walign = 1.0;
    if (walign != *misc_align) {
        g_object_freeze_notify (G_OBJECT (misc));
        g_object_notify (G_OBJECT (misc), align_str);
        *misc_align = walign;
        if (gtk_widget_is_drawable (widget)) {
            gtk_widget_queue_draw (widget);
        }
        g_object_thaw_notify (G_OBJECT (misc));
    }
}

static void misc_set_padding (GtkWidget *widget, gint margin, gboolean horizontal)
{
    // only GtkLabel/GtkArrow/GtkImage/GtkPixmap support this feature
    GtkMisc *misc = GTK_MISC (widget);
    guint16 *misc_pad = horizontal ? &(misc->xpad) : &(misc->ypad);
    char *pad_str = horizontal ? "xpad" : "ypad";
    if (margin < 0) margin = 0;
    if (margin != *misc_pad) {
        g_object_freeze_notify (G_OBJECT (misc));
        g_object_notify (G_OBJECT (misc), pad_str);
        *misc_pad = margin;
        if (gtk_widget_is_drawable (widget)) {
            gtk_widget_queue_resize (GTK_WIDGET (misc));
        }
        g_object_thaw_notify (G_OBJECT (misc));
    }
}


static int alignment_set_align (GtkWidget *widget, GtkAlign align, char *property)
{
    GtkWidget *alignment = NULL;
    char *p2;
    if (GTK_IS_ALIGNMENT(widget)) {
        alignment = widget;
    } else {
        alignment = g_object_get_data (G_OBJECT(widget), "alignment");
    }
    if (!alignment) {
        return -1;
    }
    if (*property == 'x') {
        p2 = "xscale";
    } else {
        p2 = "yscale";
    }
    gfloat walign = 0.0;
    if   (align == GTK_ALIGN_CENTER) walign = 0.5;
    else if (align == GTK_ALIGN_END) walign = 1.0;
    if (align == GTK_ALIGN_FILL) {
        /* fill GtkAlignment widget */
        g_object_set (G_OBJECT(alignment), p2, 1.0, NULL);
    } else {
        g_object_set (G_OBJECT(alignment), property, walign, NULL);
    }
    return 0;
}

static int alignment_set_padding (GtkWidget *widget, int margin, char *property)
{
    GtkWidget *alignment = NULL;
    if (GTK_IS_ALIGNMENT(widget)) {
        alignment = widget;
    } else {
        alignment = g_object_get_data (G_OBJECT(widget), "alignment");
    }
    if (!alignment) {
        return -1;
    }
    g_object_set (G_OBJECT(alignment), property, margin, NULL);
    return 0;
}


void gtk_widget_set_halign (GtkWidget *widget, GtkAlign align)
{
    if (alignment_set_align (widget, align, "xalign") != -1) {
        return;
    }
    if (GTK_IS_MISC(widget)) {
        misc_set_alignment (widget, align, TRUE);
        return;
    }
}

void gtk_widget_set_valign (GtkWidget *widget, GtkAlign align)
{
    if (alignment_set_align (widget, align, "yalign") != -1) {
        return;
    }
    if (GTK_IS_MISC(widget)) {
        misc_set_alignment (widget, align, FALSE);
        return;
    }
}

void gtk_widget_set_margin_start  (GtkWidget *widget, gint margin)
{
    if (alignment_set_padding (widget, margin, "left-padding") != -1) {
        return;
    }
    if (GTK_IS_MISC(widget)) {
        misc_set_padding (widget, margin, TRUE);
        return;
    }
}

void gtk_widget_set_margin_end (GtkWidget *widget, gint margin)
{
    alignment_set_padding (widget, margin, "right-padding");
}

void gtk_widget_set_margin_bottom (GtkWidget *widget, gint margin)
{
    alignment_set_padding (widget, margin, "bottom-padding");
}

void gtk_widget_set_margin_top (GtkWidget *widget, gint margin)
{
    if (alignment_set_padding (widget, margin, "top-padding") != -1) {
        return;
    }
    if (GTK_IS_MISC(widget)) {
        misc_set_padding (widget, margin, FALSE);
        return;
    }
}

void gtk_grid_attach (GtkGrid *grid, GtkWidget *child, gint left, gint top, gint width, gint height)
{
    if (GTK_IS_ALIGNMENT(child)) {
        gtk_table_attach (grid, child, left, left+width, top, top+height,
                          GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);
    } else {
        gtk_table_attach_defaults (grid, child, left, left+width, top, top+height);
    }
}

#endif


