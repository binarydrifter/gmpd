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
#include "gmpd-idle.h"
#include "gmpd-idle-response.h"
#include "gmpd-response.h"

static void gmpd_idle_response_iface_init(GMpdResponseIface *iface);

enum {
	PROP_NONE,
	PROP_CHANGED,
	N_PROPERTIES,
};

G_DEFINE_TYPE_WITH_CODE(GMpdIdleResponse, gmpd_idle_response, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(GMPD_TYPE_RESPONSE,
                                              gmpd_idle_response_iface_init))

static GParamSpec *PROPERTIES[N_PROPERTIES] = {NULL};

static void
gmpd_idle_response_feed_pair(GMpdResponse *response,
                             GMpdVersion *version,
                             const gchar *key,
                             const gchar *value)
{
	GMpdIdleResponse *self;
	GMpdIdle idle;

	g_return_if_fail(GMPD_IS_IDLE_RESPONSE(response));
	g_return_if_fail(GMPD_IS_VERSION(version));
	g_return_if_fail(key != NULL);
	g_return_if_fail(value != NULL);

	self = GMPD_IDLE_RESPONSE(response);

	if (g_strcmp0(key, "changed") != 0) {
		g_warning("invalid key: %s", key);
		return;
	}

	idle = gmpd_idle_from_string(value);
	if (idle == GMPD_IDLE_NONE) {
		g_warning("unknown idle event: %s", value);
		return;
	}

	gmpd_idle_response_set_changed(self, self->changed | idle);
}


static void
gmpd_idle_response_iface_init(GMpdResponseIface *iface)
{
	iface->feed_pair = gmpd_idle_response_feed_pair;
}

static void
gmpd_idle_response_set_property(GObject *object,
                                guint prop_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
	GMpdIdleResponse *self = GMPD_IDLE_RESPONSE(object);

	switch (prop_id) {
	case PROP_CHANGED:
		gmpd_idle_response_set_changed(self, g_value_get_flags(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_idle_response_get_property(GObject *object,
                                guint prop_id,
                                GValue *value,
                                GParamSpec *pspec)
{
	GMpdIdleResponse *self = GMPD_IDLE_RESPONSE(object);

	switch (prop_id) {
	case PROP_CHANGED:
		g_value_set_flags(value, gmpd_idle_response_get_changed(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_idle_response_class_init(GMpdIdleResponseClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = gmpd_idle_response_set_property;
	object_class->get_property = gmpd_idle_response_get_property;

	PROPERTIES[PROP_CHANGED] =
		g_param_spec_flags("changed",
		                   "Changed",
		                   "The changed subsystems of the server",
		                   GMPD_TYPE_IDLE,
		                   GMPD_IDLE_NONE,
		                   G_PARAM_READWRITE |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPERTIES, PROPERTIES);
}

static void
gmpd_idle_response_init(GMpdIdleResponse *self)
{
	self->changed = GMPD_IDLE_NONE;
}

GMpdIdleResponse *
gmpd_idle_response_new(void)
{
	return g_object_new(GMPD_TYPE_IDLE_RESPONSE, NULL);
}

void
gmpd_idle_response_set_changed(GMpdIdleResponse *self, GMpdIdle changed)
{
	g_return_if_fail(GMPD_IS_IDLE_RESPONSE(self));

	if (self->changed != changed) {
		self->changed = changed;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_CHANGED]);
	}
}

GMpdIdle
gmpd_idle_response_get_changed(GMpdIdleResponse *self)
{
	g_return_val_if_fail(GMPD_IS_IDLE_RESPONSE(self), GMPD_IDLE_NONE);
	return self->changed;
}

