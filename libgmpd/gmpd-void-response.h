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

#ifndef __GMPD_VOID_RESPONSE_H__
#define __GMPD_VOID_RESPONSE_H__

#if !defined(__GMPD_BUILD__)
#   error "This file is private to libgmpd and should not be included."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

#define GMPD_TYPE_VOID_RESPONSE \
	(gmpd_void_response_get_type())

#define GMPD_VOID_RESPONSE(inst) \
	(G_TYPE_CHECK_INSTANCE_CAST((inst), GMPD_TYPE_VOID_RESPONSE, GMpdVoidResponse))

#define GMPD_VOID_RESPONSE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GMPD_TYPE_VOID_RESPONSE, GMpdVoidResponseClass))

#define GMPD_IS_VOID_RESPONSE(inst) \
	(G_TYPE_CHECK_INSTANCE_TYPE((inst), GMPD_TYPE_VOID_RESPONSE))

#define GMPD_IS_VOID_RESPONSE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GMPD_TYPE_VOID_RESPONSE))

#define GMPD_VOID_RESPONSE_GET_CLASS(inst) \
	(G_TYPE_INSTANCE_GET_CLASS((inst), GMPD_TYPE_VOID_RESPONSE, GMpdVoidResponseClass))

typedef struct _GMpdVoidResponse GMpdVoidResponse;
typedef struct _GMpdVoidResponseClass GMpdVoidResponseClass;

struct _GMpdVoidResponse {
	GObject __base__;
};

struct _GMpdVoidResponseClass {
	GObjectClass __base__;
};

GType gmpd_void_response_get_type(void);

GMpdVoidResponse *gmpd_void_response_new(void);

G_END_DECLS

#endif /* __GMPD_VOID_RESPONSE_H__ */

