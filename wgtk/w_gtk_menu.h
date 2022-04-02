/*
 * This is free and unencumbered software released into the public domain.
 *
 * For more information, please refer to <https://unlicense.org>
 */

#ifndef __W_GTK_MENU__
#define __W_GTK_MENU__

#include <gtk/gtk.h>

//#define USE_GTK_ACTION 1
//#define USE_GTK_APPLICATION 1

#ifdef USE_GTK_APPLICATION

#define WGtkMenuBar GMenu
#define WGtkMenu    GMenu
#define w_gtk_menu_bar_new g_menu_new
#define w_gtk_menu_new     g_menu_new

typedef struct _WGtkMenuItemParams
{
    /* GSimpleAction */
    const char * action_name;
    const char * radio_id;
    GtkApplication * gtk_app;
    const char * accel_str;
    void * activate_cb;
    void * cb_data;
    void * cb_data_all;
    //--
    gboolean checkbox;
    gboolean check_state;
    /* GMenuItem */
    GMenu * parent_menu;
    GMenu * submenu;
    const char * label;
    const char * radio_group;
    const char * icon_name;
    const char * icon_alt;
    // unused
    void *accel_group;
    const char *accel_path;
} WGtkMenuItemParams;

GSimpleAction * w_gtk_menu_item_new (WGtkMenuItemParams * params);

#else /*===== GtkMenu =====*/

#define WGtkMenuBar GtkMenuBar
#define WGtkMenu    GtkMenu
#define w_gtk_menu_bar_new gtk_menu_bar_new
#define w_gtk_menu_new     gtk_menu_new

typedef struct _WGtkMenuItemParams
{
    GtkWidget * parent_menu;
    GtkWidget * submenu;
    const char * label;           /* empty = separator  */
    const char * icon_name;
    const char * icon_alt;
    void * activate_cb;     /* callback func for the activate signal */
    void * cb_data;         /* callback data      */
    void * cb_data_all;     /* callback for all signals */
    const char * accel_str;       /* i.e: "<Control>n"  */
    GtkAccelGroup * accel_group;
    const char * accel_path;
    gboolean checkbox;
    gboolean check_state;
    const char *radio_group;
    const char *radio_id;
    const char * tooltip;
//#ifdef USE_GTK_ACTION
    GtkActionGroup * gtk_app;
    const char * action_name;
//#endif
} WGtkMenuItemParams;

GtkWidget * w_gtk_recent_menu_new (const char * application, gpointer activated_cb);
GtkMenuItem * w_gtk_menu_item_new (WGtkMenuItemParams * params);
void w_gtk_action_group_destroy_action (GtkActionGroup *action_group,
                                        const char *action_name);

#endif
#endif /* __W_GTK_MENU__ */
