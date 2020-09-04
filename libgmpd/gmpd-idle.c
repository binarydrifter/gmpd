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

static const GFlagsValue IDLE_VALUES[] = {
	{GMPD_IDLE_DATABASE,     "GMPD_IDLE_DATABASE",     "idle-database"},
	{GMPD_IDLE_UPDATE,       "GMPD_IDLE_UPDATE",       "idle-update"},
	{GMPD_IDLE_PLAYLIST,     "GMPD_IDLE_PLAYLIST",     "idle-playlist"},
	{GMPD_IDLE_QUEUE,        "GMPD_IDLE_QUEUE",        "idle-queue"},
	{GMPD_IDLE_PLAYER,       "GMPD_IDLE_PLAYER",       "idle-player"},
	{GMPD_IDLE_MIXER,        "GMPD_IDLE_MIXER",        "idle-mixer"},
	{GMPD_IDLE_OUTPUT,       "GMPD_IDLE_OUTPUT",       "idle-output"},
	{GMPD_IDLE_OPTIONS,      "GMPD_IDLE_OPTIONS",      "idle-options"},
	{GMPD_IDLE_PARTITION,    "GMPD_IDLE_PARTITION",    "idle-partition"},
	{GMPD_IDLE_STICKER,      "GMPD_IDLE_STICKER",      "idle-sticker"},
	{GMPD_IDLE_SUBSCRIPTION, "GMPD_IDLE_SUBSCRIPTION", "idle-subscription"},
	{GMPD_IDLE_MESSAGE,      "GMPD_IDLE_MESSAGE",      "idle-message"},
	{GMPD_IDLE_NEIGHBOR,     "GMPD_IDLE_NEIGHBOR",     "idle-neighbor"},
	{GMPD_IDLE_MOUNT,        "GMPD_IDLE_MOUNT",        "idle-mount"},
	{0, NULL, NULL},
};

static const gchar *IDLE_STRINGS[] = {
	"database",
	"update",
	"stored_playlist",
	"playlist",
	"player",
	"mixer",
	"output",
	"options",
	"partition",
	"sticker",
	"subscription",
	"message",
	"neighbor",
	"mount",
	NULL,
};

static const size_t IDLE_STRINGS_LEN = G_N_ELEMENTS(IDLE_STRINGS) - 1;

GType
gmpd_idle_get_type(void)
{
	static gsize init = 0;
	static GType type = 0;

	if (g_once_init_enter(&init)) {
		type = g_flags_register_static("GMpdIdle", IDLE_VALUES);
		g_once_init_leave(&init, 1);
	}

	return type;
}

GMpdIdle
gmpd_idle_from_string(const gchar *s)
{
	gsize i;

	for (i = 0; i < IDLE_STRINGS_LEN; i++) {
		if (g_strcmp0(s, IDLE_STRINGS[i]) == 0)
			return (GMpdIdle) 1 << i;
	}

	return GMPD_IDLE_NONE;
}

gchar *
gmpd_idle_to_string(GMpdIdle idle)
{
	GString *buffer = g_string_new(NULL);
	gsize i;

	for (i = 0; i < IDLE_STRINGS_LEN; i++) {
		if (idle & (1 << i)) {
			if (buffer->len)
				g_string_append_printf(buffer, " %s", IDLE_STRINGS[i]);
			else
				g_string_append_printf(buffer, "%s", IDLE_STRINGS[i]);
		}
	}

	return g_string_free(buffer, FALSE);
}

