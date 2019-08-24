/*
 * gm_log.h
 * Copyright (C) Kevin DeKorte 2012 <kdekorte@gmail.com> and Hans Ecke
 * 
 * gm_log.h is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * gm_log.h is distributed in the hope that it will be useful,
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

#include <glib.h>

#ifdef __cplusplus
extern "C" {

#endif
    void gm_logv(gboolean force_info_to_message, GLogLevelFlags log_level, const gchar * format, va_list args);
    __attribute__ ((format(printf, 3, 4))) void gm_log(gboolean force_info_to_message, GLogLevelFlags log_level,
                                                       const gchar * format, ...);
    void gm_logs(gboolean force_info_to_message, GLogLevelFlags log_level, const gchar * msg);
    void gm_logsp(gboolean force_info_to_message, GLogLevelFlags log_level, const gchar * prefix, const gchar * msg);
    void gm_log_name_this_thread(gchar const *const name);

#ifdef __cplusplus
}
#endif
