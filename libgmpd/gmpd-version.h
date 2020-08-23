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

#ifndef __GMPD_VERSION_H__
#define __GMPD_VERSION_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define GMPD_TYPE_VERSION \
	(gmpd_version_get_type())

#define GMPD_VERSION(inst) \
	(G_TYPE_CHECK_INSTANCE_CAST((inst), GMPD_TYPE_VERSION, GMpdVersion))

#define GMPD_VERSION_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GMPD_TYPE_VERSION, GMpdVersionClass))

#define GMPD_IS_VERSION(inst) \
	(G_TYPE_CHECK_INSTANCE_TYPE((inst), GMPD_TYPE_VERSION))

#define GMPD_IS_VERSION_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GMPD_TYPE_VERSION))

#define GMPD_VERSION_GET_CLASS(inst) \
	(G_TYPE_INSTANCE_GET_CLASS((inst), GMPD_TYPE_VERSION, GMpdVersionClass))

typedef struct _GMpdVersion      GMpdVersion;
typedef struct _GMpdVersionClass GMpdVersionClass;

GType          gmpd_version_get_type         (void);

GMpdVersion *  gmpd_version_new              (gint         major,
                                              gint         minor,
                                              gint         patch);

GMpdVersion *  gmpd_version_new_from_string  (const gchar *s);

gint           gmpd_version_get_major        (GMpdVersion *self);
gint           gmpd_version_get_minor        (GMpdVersion *self);
gint           gmpd_version_get_patch        (GMpdVersion *self);

gint           gmpd_version_compare          (GMpdVersion *lhs,
                                              GMpdVersion *rhs);

G_END_DECLS

#endif /* __GMPD_VERSION_H__ */

