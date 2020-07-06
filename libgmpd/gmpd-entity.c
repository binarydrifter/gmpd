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

#include <gio/gio.h>
#include "gmpd-entity.h"
#include "gmpd-entity-priv.h"

enum {
	PROP_NONE,
	PROP_PATH,
	PROP_LAST_MODIFIED,
	N_PROPERTIES,
};

G_DEFINE_ABSTRACT_TYPE(GMpdEntity, gmpd_entity, G_TYPE_OBJECT)

static GParamSpec *PROPERTIES[N_PROPERTIES] = {NULL};

static void
gmpd_entity_set_property(GObject *object,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *pspec)
{
	GMpdEntity *self = GMPD_ENTITY(object);

	switch (prop_id) {
	case PROP_PATH:
		gmpd_entity_set_path(self, g_value_get_string(value));
		break;

	case PROP_LAST_MODIFIED:
		gmpd_entity_set_last_modified(self, g_value_get_boxed(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_entity_get_property(GObject *object,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *pspec)
{
	GMpdEntity *self = GMPD_ENTITY(object);

	switch (prop_id) {
	case PROP_PATH:
		g_value_take_string(value, gmpd_entity_get_path(self));
		break;

	case PROP_LAST_MODIFIED:
		g_value_take_boxed(value, gmpd_entity_get_last_modified(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_entity_finalize(GObject *object)
{
	GMpdEntity *self = GMPD_ENTITY(object);

	g_clear_pointer(&self->path, g_free);
	g_clear_pointer(&self->last_modified, g_date_time_unref);

	G_OBJECT_CLASS(gmpd_entity_parent_class)->finalize(object);
}

static void
gmpd_entity_class_init(GMpdEntityClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = gmpd_entity_set_property;
	object_class->get_property = gmpd_entity_get_property;
	object_class->finalize = gmpd_entity_finalize;

	PROPERTIES[PROP_PATH] =
		g_param_spec_string("path",
		                    "Path",
		                    "Path of the entity within the database",
		                    NULL,
		                    G_PARAM_READWRITE |
		                    G_PARAM_EXPLICIT_NOTIFY |
		                    G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_LAST_MODIFIED] =
		g_param_spec_boxed("last-modified",
		                   "Last Modified",
		                   "Last modification time of the entity",
		                   G_TYPE_DATE_TIME,
		                   G_PARAM_READWRITE |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPERTIES, PROPERTIES);
}

static void
gmpd_entity_init(GMpdEntity *self)
{
	self->path = NULL;
	self->last_modified = NULL;
}

void
gmpd_entity_set_path(GMpdEntity *self, const gchar *path)
{
	g_return_if_fail(GMPD_IS_ENTITY(self));

	g_free(self->path);
	self->path = g_strdup(path);

	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_PATH]);
}

void
gmpd_entity_set_last_modified(GMpdEntity *self, GDateTime *last_modified)
{
	g_return_if_fail(GMPD_IS_ENTITY(self));

	g_clear_pointer(&self->last_modified, g_date_time_unref);
	self->last_modified = last_modified ? g_date_time_ref(last_modified) : NULL;

	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_LAST_MODIFIED]);
}

gchar *
gmpd_entity_get_path(GMpdEntity *self)
{
	g_return_val_if_fail(self != NULL, NULL);
	return g_strdup(self->path);
}

GDateTime *
gmpd_entity_get_last_modified(GMpdEntity *self)
{
	g_return_val_if_fail(self != NULL, NULL);
	return self->last_modified ? g_date_time_ref(self->last_modified) : NULL;
}

