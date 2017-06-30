/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gmtk_common.c
 * Copyright (C) Kevin DeKorte 2006 <kdekorte@gmail.com>
 * 
 * gmtk_common.c is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * gmtk_common.c is distributed in the hope that it will be useful,
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

#include "gmtk_common.h"
#include <libintl.h>

void gmtk_get_allocation(GtkWidget * widget, GtkAllocation * allocation)
{
#ifdef GTK2_18_ENABLED
    gtk_widget_get_allocation(widget, allocation);
#else
    allocation = &(widget->allocation);
#endif
}

GdkWindow *gmtk_get_window(GtkWidget * widget)
{
#ifdef GTK2_14_ENABLED
    return gtk_widget_get_window(widget);
#else
    return widget->window;
#endif
}

gboolean gmtk_get_visible(GtkWidget * widget)
{
#ifdef GTK2_18_ENABLED
    return gtk_widget_get_visible(widget);
#else
    return GTK_WIDGET_VISIBLE(widget);
#endif
}

gboolean gmtk_widget_get_realized(GtkWidget * widget)
{
#ifdef GTK2_20_ENABLED
    return gtk_widget_get_realized(widget);
#else
    return GTK_WIDGET_REALIZED(widget);
#endif
}

const gchar *gmtk_version()
{
#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
//    textdomain(GETTEXT_PACKAGE);
#endif
    return VERSION;
}
