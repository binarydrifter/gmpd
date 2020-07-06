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

#ifndef __GMPD_ENTITY_H__
#define __GMPD_ENTITY_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

#define GMPD_TYPE_ENTITY \
	(gmpd_entity_get_type())

#define GMPD_ENTITY(inst) \
	(G_TYPE_CHECK_INSTANCE_CAST((inst), GMPD_TYPE_ENTITY, GMpdEntity))

#define GMPD_ENTITY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GMPD_TYPE_ENTITY, GMpdEntityClass))

#define GMPD_IS_ENTITY(inst) \
	(G_TYPE_CHECK_INSTANCE_TYPE((inst), GMPD_TYPE_ENTITY))

#define GMPD_IS_ENTITY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GMPD_TYPE_ENTITY))

#define GMPD_ENTITY_GET_CLASS(inst) \
	(G_TYPE_INSTANCE_GET_CLASS((inst), GMPD_TYPE_ENTITY, GMpdEntityClass))

typedef struct _GMpdEntity GMpdEntity;
typedef struct _GMpdEntityClass GMpdEntityClass;

GType gmpd_entity_get_type(void);

void gmpd_entity_set_path(GMpdEntity *self, const gchar *path);
void gmpd_entity_set_last_modified(GMpdEntity *self, GDateTime *last_modified);

gchar *gmpd_entity_get_path(GMpdEntity *self);
GDateTime *gmpd_entity_get_last_modified(GMpdEntity *self);

G_END_DECLS

#endif /* __GMPD_ENTITY_H__ */

