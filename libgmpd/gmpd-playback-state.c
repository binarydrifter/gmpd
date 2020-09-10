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
#include "gmpd-playback-state.h"

static const GEnumValue PLAYBACK_STATE_VALUES[] = {
	{GMPD_PLAYBACK_UNKNOWN, "GMPD_PLAYBACK_UNKNOWN", "playback-unknown"},
	{GMPD_PLAYBACK_STOPPED, "GMPD_PLAYBACK_STOPPED", "playback-stopped"},
	{GMPD_PLAYBACK_PAUSED,  "GMPD_PLAYBACK_PAUSED",  "playback-paused"},
	{GMPD_PLAYBACK_PLAYING, "GMPD_PLAYBACK_PLAYING", "playback-playing"},
	{0, NULL, NULL}
};

GType
gmpd_playback_state_get_type(void)
{
	static gsize init = 0;
	static GType type = 0;

	if (g_once_init_enter(&init)) {
		type = g_enum_register_static("GMpdPlaybackState", PLAYBACK_STATE_VALUES);
		g_once_init_leave(&init, 1);
	}

	return type;
}

GMpdPlaybackState
gmpd_playback_state_from_string(const gchar *s)
{
	if (g_strcmp0(s, "stop") == 0)
		return GMPD_PLAYBACK_STOPPED;

	else if (g_strcmp0(s, "pause") == 0)
		return GMPD_PLAYBACK_PAUSED;

	else if (g_strcmp0(s, "play") == 0)
		return GMPD_PLAYBACK_PLAYING;

	else
		return GMPD_PLAYBACK_UNKNOWN;
}

gchar *
gmpd_playback_state_to_string(GMpdPlaybackState state)
{
	switch (state) {
	case GMPD_PLAYBACK_UNKNOWN:
		return NULL;

	case GMPD_PLAYBACK_STOPPED:
		return g_strdup("stop");

	case GMPD_PLAYBACK_PAUSED:
		return g_strdup("pause");

	case GMPD_PLAYBACK_PLAYING:
		return g_strdup("play");
	}

	g_return_val_if_reached(NULL);
}

