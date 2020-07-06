/* libgmpd: MPD protocol implementation for GLib
 * Copyright (C) 2020 Patrick Keating <binarydrifter@protonmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GMPD_ERROR_H__
#define __GMPD_ERROR_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define GMPD_IS_ERROR_ENUM(e) ( \
	((e) == GMPD_ERROR_UNKNOWN) || \
	((e) >= GMPD_ERROR_NOT_LIST && (e) <= GMPD_ERROR_COMMAND) || \
	((e) >= GMPD_ERROR_DOES_NOT_EXIST && (e) <= GMPD_ERROR_EXISTS))

#define GMPD_TYPE_ERROR \
	(gmpd_error_get_type())

#define GMPD_ERROR \
	(gmpd_error_quark())

typedef enum _GMpdErrorEnum {
	GMPD_ERROR_UNKNOWN    = -1,

	GMPD_ERROR_NOT_LIST   = 1,
	GMPD_ERROR_ARGUMENTS  = 2,
	GMPD_ERROR_PASSWORD   = 3,
	GMPD_ERROR_PERMISSION = 4,
	GMPD_ERROR_COMMAND    = 5,

	GMPD_ERROR_DOES_NOT_EXIST     = 50,
	GMPD_ERROR_PLAYLIST_MAX       = 51,
	GMPD_ERROR_SYSTEM             = 52,
	GMPD_ERROR_PLAYLIST_LOAD      = 53,
	GMPD_ERROR_UPDATE_IN_PROGRESS = 54,
	GMPD_ERROR_OUT_OF_SYNC        = 55,
	GMPD_ERROR_EXISTS             = 56,
} GMpdErrorEnum;

GType gmpd_error_enum_get_type(void);

GQuark gmpd_error_quark(void);

GError *gmpd_error_from_string(const gchar *s);

G_END_DECLS

#endif /* __GMPD_ERROR_H__ */

