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
#include "gmpd-single-state.h"

static const GEnumValue SINGLE_STATE_VALUES[] = {
	{GMPD_SINGLE_DISABLED, "GMPD_SINGLE_DISABLED", "single-disabled"},
	{GMPD_SINGLE_ONESHOT,  "GMPD_SINGLE_ONESHOT",  "single-oneshot"},
	{GMPD_SINGLE_ENABLED,  "GMPD_SINGLE_ENABLED",  "single-enabled"},
	{0, NULL, NULL}
};

GType
gmpd_single_state_get_type(void)
{
	static gsize init = 0;
	static GType type = 0;

	if (g_once_init_enter(&init)) {
		type = g_enum_register_static("GMpdSingleState", SINGLE_STATE_VALUES);
		g_once_init_leave(&init, 1);
	}

	return type;
}

GMpdSingleState
gmpd_single_state_from_string(const gchar *s)
{
	if (g_strcmp0(s, "0") == 0)
		return GMPD_SINGLE_DISABLED;

	else if (g_strcmp0(s, "1") == 0)
		return GMPD_SINGLE_ENABLED;

	else if (g_strcmp0(s, "oneshot") == 0)
		return GMPD_SINGLE_ONESHOT;

	else
		return GMPD_SINGLE_DISABLED;
}

gchar *
gmpd_single_state_to_string(GMpdSingleState state)
{
	switch (state) {
	case GMPD_SINGLE_DISABLED:
		return g_strdup("0");

	case GMPD_SINGLE_ENABLED:
		return g_strdup("1");

	case GMPD_SINGLE_ONESHOT:
		return g_strdup("oneshot");

	default:
		g_return_val_if_reached(NULL);
	}
}

