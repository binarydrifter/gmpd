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
#include "gmpd-replay-gain-mode.h"
#include "gmpd-replay-gain-status.h"
#include "gmpd-response.h"

static void gmpd_replay_gain_status_response_iface_init(GMpdResponseIface *iface);

enum {
	PROP_NONE,
	PROP_MODE,
	N_PROPERTIES,
};

struct _GMpdReplayGainStatus {
	GObject            __base__;
	GMpdReplayGainMode mode;
};

struct _GMpdReplayGainStatusClass {
	GObjectClass __base__;
};

G_DEFINE_TYPE_WITH_CODE(GMpdReplayGainStatus, gmpd_replay_gain_status, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(GMPD_TYPE_RESPONSE,
                                              gmpd_replay_gain_status_response_iface_init))

static GParamSpec *PROPERTIES[N_PROPERTIES] = {NULL};

static void
gmpd_replay_gain_status_feed_pair(GMpdResponse *response,
                                  GMpdVersion  *version G_GNUC_UNUSED,
                                  const gchar  *key,
                                  const gchar  *value)
{
	GMpdReplayGainStatus *self = GMPD_REPLAY_GAIN_STATUS(response);

	if (g_strcmp0(key, "replay_gain_mode") == 0) {
		gmpd_replay_gain_status_set_mode(self, gmpd_replay_gain_mode_from_string(value));
	} else {
		g_warning("invalid key: %s", key);
	}
}

static void
gmpd_replay_gain_status_response_iface_init(GMpdResponseIface *iface)
{
	iface->feed_pair = gmpd_replay_gain_status_feed_pair;
}

static void
gmpd_replay_gain_status_set_property(GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
	GMpdReplayGainStatus *self = GMPD_REPLAY_GAIN_STATUS(object);

	switch (prop_id) {
	case PROP_MODE:
		gmpd_replay_gain_status_set_mode(self, g_value_get_enum(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_replay_gain_status_get_property(GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
	GMpdReplayGainStatus *self = GMPD_REPLAY_GAIN_STATUS(object);

	switch (prop_id) {
	case PROP_MODE:
		g_value_set_enum(value, gmpd_replay_gain_status_get_mode(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_replay_gain_status_class_init(GMpdReplayGainStatusClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = gmpd_replay_gain_status_set_property;
	object_class->get_property = gmpd_replay_gain_status_get_property;

	PROPERTIES[PROP_MODE] =
		g_param_spec_enum("mode",
		                  "Mode",
		                  "Replay gain mode of the server",
		                  GMPD_TYPE_REPLAY_GAIN_MODE,
		                  GMPD_REPLAY_GAIN_OFF,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPERTIES, PROPERTIES);
}

static void
gmpd_replay_gain_status_init(GMpdReplayGainStatus *self)
{
	self->mode = GMPD_REPLAY_GAIN_OFF;
}

GMpdReplayGainStatus *
gmpd_replay_gain_status_new(void)
{
	return g_object_new(GMPD_TYPE_REPLAY_GAIN_STATUS, NULL);
}

void
gmpd_replay_gain_status_set_mode(GMpdReplayGainStatus *self,
                                 GMpdReplayGainMode    mode)
{
	g_return_if_fail(GMPD_IS_REPLAY_GAIN_STATUS(self));

	if (!GMPD_IS_REPLAY_GAIN_MODE(mode)) {
		g_warn_if_fail(GMPD_IS_REPLAY_GAIN_MODE(mode));
		mode = GMPD_REPLAY_GAIN_OFF;
	}

	self->mode = mode;

	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_MODE]);
}

GMpdReplayGainMode
gmpd_replay_gain_status_get_mode(GMpdReplayGainStatus *self)
{
	g_return_val_if_fail(GMPD_IS_REPLAY_GAIN_STATUS(self), GMPD_REPLAY_GAIN_OFF);
	return self->mode;
}

