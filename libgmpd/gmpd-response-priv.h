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

#ifndef __GMPD_RESPONSE_PRIV_H__
#define __GMPD_RESPONSE_PRIV_H__

#if !defined(__GMPD_BUILD__)
#   error "This file is private to libgmpd and should not be included."
#endif

#include <gio/gio.h>
#include <gmpd-response.h>
#include <gmpd-version.h>

G_BEGIN_DECLS

struct _GMpdResponseIface {
	GTypeInterface __base__;

	void (*feed_pair) (GMpdResponse *self,
	                   GMpdVersion *version,
	                   const gchar *key,
	                   const gchar *value);

	void (*feed_binary) (GMpdResponse *self,
	                     GMpdVersion *version,
	                     GBytes *binary);

	gsize (*get_remaining_binary) (GMpdResponse *self);
};

G_END_DECLS

#endif /* __GMPD_RESPONSE_PRIV_H__ */

