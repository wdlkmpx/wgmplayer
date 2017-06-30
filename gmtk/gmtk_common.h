/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gmtk_common.h
 * Copyright (C) Kevin DeKorte 2006 <kdekorte@gmail.com>
 * 
 * gmtk_common.h is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * gmtk_common.h is distributed in the hope that it will be useful,
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#ifdef X11_ENABLED
#include <gdk/gdkx.h>
#if GTK_MAJOR_VERSION == 3
#include <gtk/gtkx.h>
#endif
#endif
#if GTK_MAJOR_VERSION == 3
#include <gdk/gdkkeysyms-compat.h>
#else
#include <gdk/gdkkeysyms.h>
#endif
#include "gm_common.h"

#ifndef __GMTK_COMMON_H__
#define __GMTK_COMMON_H__

void gmtk_get_allocation(GtkWidget * widget, GtkAllocation * allocation);
GdkWindow *gmtk_get_window(GtkWidget * widget);
gboolean gmtk_get_visible(GtkWidget * widget);
gboolean gmtk_widget_get_realized(GtkWidget * widget);
const gchar *gmtk_version();

#endif
