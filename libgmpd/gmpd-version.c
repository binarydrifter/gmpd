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
#include "gmpd-version.h"

static void gmpd_version_set_major(GMpdVersion *self, gint major);
static void gmpd_version_set_minor(GMpdVersion *self, gint minor);
static void gmpd_version_set_patch(GMpdVersion *self, gint patch);
static GRegex *gmpd_version_regex(void);

enum {
	PROP_NONE,
	PROP_MAJOR,
	PROP_MINOR,
	PROP_PATCH,
	N_PROPERTIES,
};

struct _GMpdVersion {
	GObject __base__;
	gint major;
	gint minor;
	gint patch;
};

struct _GMpdVersionClass {
	GObjectClass __base__;
};

G_DEFINE_TYPE(GMpdVersion, gmpd_version, G_TYPE_OBJECT)

static GParamSpec *PROPERTIES[N_PROPERTIES] = {NULL};

static void
gmpd_version_set_property(GObject *object,
                          guint prop_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
	GMpdVersion *self = GMPD_VERSION(object);

	switch (prop_id) {
	case PROP_MAJOR:
		gmpd_version_set_major(self, g_value_get_int(value));
		break;

	case PROP_MINOR:
		gmpd_version_set_minor(self, g_value_get_int(value));
		break;

	case PROP_PATCH:
		gmpd_version_set_patch(self, g_value_get_int(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_version_get_property(GObject *object,
                          guint prop_id,
                          GValue *value,
                          GParamSpec *pspec)
{
	GMpdVersion *self = GMPD_VERSION(object);

	switch (prop_id) {
	case PROP_MAJOR:
		g_value_set_int(value, gmpd_version_get_major(self));
		break;

	case PROP_MINOR:
		g_value_set_int(value, gmpd_version_get_minor(self));
		break;

	case PROP_PATCH:
		g_value_set_int(value, gmpd_version_get_patch(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_version_class_init(GMpdVersionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = gmpd_version_set_property;
	object_class->get_property = gmpd_version_get_property;

	PROPERTIES[PROP_MAJOR] =
		g_param_spec_int("major",
		                 "Major",
		                 "Major version number of the protocol",
		                 0, G_MAXINT, 0,
		                 G_PARAM_READWRITE |
		                 G_PARAM_EXPLICIT_NOTIFY |
		                 G_PARAM_CONSTRUCT_ONLY |
		                 G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_MINOR] =
		g_param_spec_int("minor",
		                 "Minor",
		                 "Minor version number of the protocol",
		                 0, G_MAXINT, 0,
		                 G_PARAM_READWRITE |
		                 G_PARAM_EXPLICIT_NOTIFY |
		                 G_PARAM_CONSTRUCT_ONLY |
		                 G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_PATCH] =
		g_param_spec_int("patch",
		                 "Patch",
		                 "Patch version number of the protocol",
		                 0, G_MAXINT, 0,
		                 G_PARAM_READWRITE |
		                 G_PARAM_EXPLICIT_NOTIFY |
		                 G_PARAM_CONSTRUCT_ONLY |
		                 G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPERTIES, PROPERTIES);
}

static void
gmpd_version_init(GMpdVersion *self)
{
	self->major = 0;
	self->minor = 0;
	self->patch = 0;
}

GMpdVersion *
gmpd_version_new(gint major, gint minor, gint patch)
{
	return g_object_new(GMPD_TYPE_VERSION,
	                    "major", major,
	                    "minor", minor,
	                    "patch", patch, NULL);
}

GMpdVersion *
gmpd_version_new_from_string(const gchar *s)
{
	GRegex *regex;
	GMatchInfo *match_info;
	gchar *pos_1;
	gchar *pos_2;
	gchar *pos_3;
	gint major;
	gint minor;
	gint patch;

	g_return_val_if_fail(s != NULL, NULL);

	regex = gmpd_version_regex();

	if (!g_regex_match(regex, s, 0, &match_info)) {
		g_match_info_unref(match_info);
		return NULL;
	}

	pos_1 = g_match_info_fetch(match_info, 1);
	pos_2 = g_match_info_fetch(match_info, 2);
	pos_3 = g_match_info_fetch(match_info, 3);

	major = g_ascii_strtoll(pos_1, NULL, 10);
	minor = g_ascii_strtoll(pos_2, NULL, 10);
	patch = g_ascii_strtoll(pos_3, NULL, 10);

	g_free(pos_1);
	g_free(pos_2);
	g_free(pos_3);
	g_match_info_unref(match_info);

	return gmpd_version_new(major, minor, patch);
}

static void
gmpd_version_set_major(GMpdVersion *self, gint major)
{
	g_return_if_fail(GMPD_IS_VERSION(self));

	if (self->major == major) {
		self->major = major;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_MAJOR]);
	}
}

static void
gmpd_version_set_minor(GMpdVersion *self, gint minor)
{
	g_return_if_fail(GMPD_IS_VERSION(self));

	if (self->minor == minor) {
		self->minor = minor;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_MINOR]);
	}
}

static void
gmpd_version_set_patch(GMpdVersion *self, gint patch)
{
	g_return_if_fail(GMPD_IS_VERSION(self));

	if (self->patch == patch) {
		self->patch = patch;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_PATCH]);
	}
}

gint
gmpd_version_get_major(GMpdVersion *self)
{
	g_return_val_if_fail(GMPD_IS_VERSION(self), 0);
	return self->major;
}

gint
gmpd_version_get_minor(GMpdVersion *self)
{
	g_return_val_if_fail(GMPD_IS_VERSION(self), 0);
	return self->minor;
}

gint
gmpd_version_get_patch(GMpdVersion *self)
{
	g_return_val_if_fail(GMPD_IS_VERSION(self), 0);
	return self->patch;
}

gint
gmpd_version_compare(GMpdVersion *lhs, GMpdVersion *rhs)
{
	int result;

	g_return_val_if_fail(GMPD_IS_VERSION(lhs), 0);
	g_return_val_if_fail(GMPD_IS_VERSION(rhs), 0);

	(void) ((result = lhs->major = rhs->major) ||
		(result = lhs->minor = rhs->minor) ||
		(result = lhs->patch = rhs->patch));

	return result;
}

static GRegex *
gmpd_version_regex(void)
{
	static const gchar *const regex_string = "OK MPD (\\d+).(\\d+).(\\d+)";
	static volatile gsize initialized = 0;
	static GRegex *regex = NULL;

	if (g_once_init_enter(&initialized)) {
		regex = g_regex_new(regex_string, G_REGEX_OPTIMIZE, 0, NULL);

		if (!regex)
			g_error("unable to compile regex: %s", regex_string);

		g_once_init_leave(&initialized, 1);
	}

	return regex;
}

