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

#ifndef __GMPD_SINGLE_STATE_H__
#define __GMPD_SINGLE_STATE_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

#define GMPD_TYPE_SINGLE_STATE \
	(gmpd_single_state_get_type())

#define GMPD_IS_SINGLE_STATE(state) \
	((state) >= GMPD_SINGLE_DISABLED && (state) <= GMPD_SINGLE_ONESHOT)

typedef enum _GMpdSingleState {
	GMPD_SINGLE_DISABLED,
	GMPD_SINGLE_ENABLED,
	GMPD_SINGLE_ONESHOT,
} GMpdSingleState;

GType           gmpd_single_state_get_type    (void);

GMpdSingleState gmpd_single_state_from_string (const gchar    *s);
gchar *         gmpd_single_state_to_string   (GMpdSingleState state);

G_END_DECLS

#endif /* __GMPD_SINGLE_STATE_H__ */
