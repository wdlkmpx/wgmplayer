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
#include <glib.h>
#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION < 26
#include <libintl.h>
#endif

#ifndef __GM_COMMON_H__
#define __GM_COMMON_H__

#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION < 26
const gchar *g_dgettext(const gchar * domain, const gchar * msgid);
#endif

#endif
