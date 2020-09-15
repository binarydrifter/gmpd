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

static const gchar *const STRINGS[] = {"off", "track", "album", "auto", NULL};
static const gsize STRINGS_LENGTH = G_N_ELEMENTS(STRINGS) - 1;

static const GEnumValue REPLAY_GAIN_MODE_VALUES[] = {
	{GMPD_REPLAY_GAIN_OFF,   "GMPD_REPLAY_GAIN_OFF",   "replay-gain-off"},
	{GMPD_REPLAY_GAIN_TRACK, "GMPD_REPLAY_GAIN_TRACK", "replay-gain-track"},
	{GMPD_REPLAY_GAIN_ALBUM, "GMPD_REPLAY_GAIN_ALBUM", "replay-gain-album"},
	{GMPD_REPLAY_GAIN_AUTO,  "GMPD_REPLAY_GAIN_AUTO",  "replay-gain-auto"},
	{0, NULL, NULL}
};

GType
gmpd_replay_gain_mode_get_type(void)
{
	static gsize init = 0;
	static GType type = 0;

	if (g_once_init_enter(&init)) {
		type = g_enum_register_static("GMpdReplayGainMode",
		                              REPLAY_GAIN_MODE_VALUES);
		g_once_init_leave(&init, 1);
	}

	return type;
}

GMpdReplayGainMode
gmpd_replay_gain_mode_from_string(const gchar *string)
{
	gsize i;

	for (i = 0; i < STRINGS_LENGTH; i++) {
		if (g_strcmp0(string, STRINGS[i]) == 0)
			return (GMpdReplayGainMode) i;
	}

	return GMPD_REPLAY_GAIN_OFF;
}

gchar *
gmpd_replay_gain_mode_to_string(GMpdReplayGainMode mode)
{
	g_return_val_if_fail(GMPD_IS_REPLAY_GAIN_MODE(mode), NULL);
	return g_strdup(STRINGS[mode]);
}

