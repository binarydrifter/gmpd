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

#ifndef __GMPD_IDLE_H__
#define __GMPD_IDLE_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

#define GMPD_TYPE_IDLE  (gmpd_idle_get_type())
#define GMPD_IDLE_NONE  ((GMpdIdle) 0)
#define GMPD_IDLE_ALL   ((GMpdIdle) 0x3fff)

typedef enum _GMpdIdle {
	GMPD_IDLE_DATABASE     = 1 << 0,
	GMPD_IDLE_UPDATE       = 1 << 1,
	GMPD_IDLE_PLAYLIST     = 1 << 2,
	GMPD_IDLE_QUEUE        = 1 << 3,
	GMPD_IDLE_PLAYER       = 1 << 4,
	GMPD_IDLE_MIXER        = 1 << 5,
	GMPD_IDLE_OUTPUT       = 1 << 6,
	GMPD_IDLE_OPTIONS      = 1 << 7,
	GMPD_IDLE_PARTITION    = 1 << 8,
	GMPD_IDLE_STICKER      = 1 << 9,
	GMPD_IDLE_SUBSCRIPTION = 1 << 10,
	GMPD_IDLE_MESSAGE      = 1 << 11,
	GMPD_IDLE_NEIGHBOR     = 1 << 12,
	GMPD_IDLE_MOUNT        = 1 << 13,
} GMpdIdle;

GType     gmpd_idle_get_type     (void);
GMpdIdle  gmpd_idle_from_string  (const gchar *s);
gchar *   gmpd_idle_to_string    (GMpdIdle     idle);

G_END_DECLS

#endif /* __GMPD_IDLE_H__ */

