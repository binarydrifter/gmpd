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
#include "gmpd-error.h"

G_DEFINE_QUARK(GMPD_ERROR, gmpd_error)

static GRegex *gmpd_error_regex(void);

static const GEnumValue GMPD_ERROR_ENUM_VALUES[] = {
	{GMPD_ERROR_UNKNOWN,
		"GMPD_ERROR_UNKNOWN",
		"gmpd-error-unknown"},

	{GMPD_ERROR_NOT_LIST,
		"GMPD_ERROR_NOT_LIST",
		"gmpd-error-not-list"},

	{GMPD_ERROR_ARGUMENTS,
		"GMPD_ERROR_ARGUMENTS",
		"gmpd-error-arguments"},

	{GMPD_ERROR_PASSWORD,
		"GMPD_ERROR_PASSWORD",
		"gmpd-error-password"},

	{GMPD_ERROR_PERMISSION,
		"GMPD_ERROR_PERMISSION",
		"gmpd-error-permission"},

	{GMPD_ERROR_COMMAND,
		"GMPD_ERROR_COMMAND",
		"gmpd-error-command"},

	{GMPD_ERROR_DOES_NOT_EXIST,
		"GMPD_ERROR_DOES_NOT_EXIST",
		"gmpd-error-does-not-exist"},

	{GMPD_ERROR_PLAYLIST_MAX,
		"GMPD_ERROR_PLAYLIST_MAX",
		"gmpd-error-playlist-max"},

	{GMPD_ERROR_SYSTEM,
		"GMPD_ERROR_SYSTEM",
		"gmpd-error-system"},

	{GMPD_ERROR_PLAYLIST_LOAD,
		"GMPD_ERROR_PLAYLIST_LOAD",
		"gmpd-error-playlist-load"},

	{GMPD_ERROR_UPDATE_IN_PROGRESS,
		"GMPD_ERROR_UPDATE_IN_PROGRESS",
		"gmpd-error-update-in-progress"},

	{GMPD_ERROR_OUT_OF_SYNC,
		"GMPD_ERROR_OUT_OF_SYNC",
		"gmpd-error-out-of-sync"},

	{GMPD_ERROR_EXISTS,
		"GMPD_ERROR_EXISTS",
		"gmpd-error-exists"},

	{0, NULL, NULL},
};

GType
gmpd_error_enum_get_type(void)
{
	static volatile gsize initialized = 0;
	static GType gtype = 0;

	if (g_once_init_enter(&initialized)) {
		gtype = g_enum_register_static("GMpdErrorEnum", GMPD_ERROR_ENUM_VALUES);
		g_once_init_leave(&initialized, 1);
	}

	return gtype;
}

GError *
gmpd_error_from_string(const gchar *s)
{
	GRegex *regex;
	GMatchInfo *match_info;
	gchar *pos_1;
	gchar *pos_3;
	gchar *pos_4;
	gint code;
	GError *error;

	g_return_val_if_fail(s != NULL, NULL);

	regex = gmpd_error_regex();

	if (!g_regex_match(regex, s, 0, &match_info)) {
		g_match_info_unref(match_info);

		return g_error_new(GMPD_ERROR,
		                   GMPD_ERROR_UNKNOWN,
		                   "invalid error string: %s", s);
	}

	pos_1 = g_match_info_fetch(match_info, 1);
	pos_3 = g_match_info_fetch(match_info, 3);
	pos_4 = g_match_info_fetch(match_info, 4);

	code = g_ascii_strtoll(pos_1, NULL, 10);
	if (!GMPD_IS_ERROR_ENUM(code)) {
		g_warning("unknown error code returned from MPD: %d", code);
		code = GMPD_ERROR_UNKNOWN;
	}

	error = g_error_new(GMPD_ERROR, code, "%s: %s", pos_3, pos_4);

	g_free(pos_1);
	g_free(pos_3);
	g_free(pos_4);
	g_match_info_unref(match_info);

	return error;
}

static GRegex *
gmpd_error_regex(void)
{
	static const gchar *const regex_str = "ACK \\[(\\d+)@(\\d+)\\] {([^}]+)} (.*)";
	static GRegex *regex = NULL;
	static gsize initialized = 0;

	if (g_once_init_enter(&initialized)) {
		regex = g_regex_new(regex_str, G_REGEX_OPTIMIZE, 0, NULL);
		if (!regex) {
			g_error("unable to compile regex string: %s", regex_str);
		}

		g_once_init_leave(&initialized, 1);
	}

	return regex;
}

