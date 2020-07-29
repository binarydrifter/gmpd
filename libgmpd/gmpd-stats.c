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
#include "gmpd-response.h"
#include "gmpd-stats.h"

static void gmpd_stats_response_iface_init(GMpdResponseIface *iface);

enum {
	PROP_NONE,
	PROP_ARTISTS,
	PROP_ALBUMS,
	PROP_SONGS,
	PROP_UPTIME,
	PROP_DB_PLAYTIME,
	PROP_DB_UPDATE,
	PROP_PLAYTIME,
	N_PROPERTIES,
};

struct _GMpdStats {
	GObject __base__;
	guint artists;
	guint albums;
	guint songs;
	guint64 uptime;
	guint64 db_playtime;
	GDateTime *db_update;
	guint64 playtime;
};

struct _GMpdStatsClass {
	GObjectClass __base__;
};

G_DEFINE_TYPE_WITH_CODE(GMpdStats, gmpd_stats, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(GMPD_TYPE_RESPONSE,
                                              gmpd_stats_response_iface_init))

static GParamSpec *PROPERTIES[N_PROPERTIES] = {NULL};

static void
gmpd_stats_response_feed_pair(GMpdResponse *response,
                              GMpdVersion *version,
                              const gchar *key,
                              const gchar *value)
{
	GMpdStats *self;

	g_return_if_fail(GMPD_IS_STATS(response));
	g_return_if_fail(GMPD_IS_VERSION(version));
	g_return_if_fail(key != NULL);
	g_return_if_fail(value != NULL);

	self = GMPD_STATS(response);

	if (g_strcmp0(key, "artists") == 0) {
		gmpd_stats_set_artists(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "albums") == 0) {
		gmpd_stats_set_albums(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "songs") == 0) {
		gmpd_stats_set_songs(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "uptime") == 0) {
		gmpd_stats_set_uptime(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "db_playtime") == 0) {
		gmpd_stats_set_db_playtime(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "db_update") == 0) {
		gint64 unix_utc = g_ascii_strtoll(value, NULL, 10);
		GDateTime *db_update = g_date_time_new_from_unix_utc(unix_utc);

		gmpd_stats_set_db_update(self, db_update);

		g_clear_pointer(&db_update, g_date_time_unref);

	} else if (g_strcmp0(key, "playtime") == 0) {
		gmpd_stats_set_playtime(self, g_ascii_strtoull(value, NULL, 10));

	} else {
		g_warning("invalid key: %s", key);
	}
}

static void
gmpd_stats_response_iface_init(GMpdResponseIface *iface)
{
	iface->feed_pair = gmpd_stats_response_feed_pair;
}

static void
gmpd_stats_set_property(GObject *object,
                        guint prop_id,
                        const GValue *value,
                        GParamSpec *pspec)
{
	GMpdStats *self = GMPD_STATS(object);

	switch (prop_id) {
	case PROP_ARTISTS:
		gmpd_stats_set_artists(self, g_value_get_uint(value));
		break;

	case PROP_ALBUMS:
		gmpd_stats_set_albums(self, g_value_get_uint(value));
		break;

	case PROP_SONGS:
		gmpd_stats_set_songs(self, g_value_get_uint(value));
		break;

	case PROP_UPTIME:
		gmpd_stats_set_uptime(self, g_value_get_uint64(value));
		break;

	case PROP_DB_PLAYTIME:
		gmpd_stats_set_db_playtime(self, g_value_get_uint64(value));
		break;

	case PROP_DB_UPDATE:
		gmpd_stats_set_db_update(self, g_value_get_boxed(value));
		break;

	case PROP_PLAYTIME:
		gmpd_stats_set_playtime(self, g_value_get_uint64(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_stats_get_property(GObject *object,
                        guint prop_id,
                        GValue *value,
                        GParamSpec *pspec)
{
	GMpdStats *self = GMPD_STATS(object);

	switch (prop_id) {
	case PROP_ARTISTS:
		g_value_set_uint(value, gmpd_stats_get_artists(self));
		break;

	case PROP_ALBUMS:
		g_value_set_uint(value, gmpd_stats_get_albums(self));
		break;

	case PROP_SONGS:
		g_value_set_uint(value, gmpd_stats_get_songs(self));
		break;

	case PROP_UPTIME:
		g_value_set_uint64(value, gmpd_stats_get_uptime(self));
		break;

	case PROP_DB_PLAYTIME:
		g_value_set_uint64(value, gmpd_stats_get_db_playtime(self));
		break;

	case PROP_DB_UPDATE:
		g_value_take_boxed(value, gmpd_stats_get_db_update(self));
		break;

	case PROP_PLAYTIME:
		g_value_set_uint64(value, gmpd_stats_get_playtime(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_stats_finalize(GObject *object)
{
	GMpdStats *self = GMPD_STATS(object);

	g_clear_pointer(&self->db_update, g_date_time_unref);

	G_OBJECT_CLASS(gmpd_stats_parent_class)->finalize(object);
}

static void
gmpd_stats_class_init(GMpdStatsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = gmpd_stats_set_property;
	object_class->get_property = gmpd_stats_get_property;
	object_class->finalize = gmpd_stats_finalize;

	PROPERTIES[PROP_ARTISTS] =
		g_param_spec_uint("artists",
		                  "Artists",
		                  "Number of artists in the database",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_ALBUMS] =
		g_param_spec_uint("albums",
		                  "Albums",
		                  "Number of albums in the database",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_SONGS] =
		g_param_spec_uint("songs",
		                  "Songs",
		                  "Number of songs in the database",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_UPTIME] =
		g_param_spec_uint64("uptime",
		                    "Uptime",
		                    "Daemon uptime in seconds",
		                    0, G_MAXUINT64, 0,
		                    G_PARAM_READWRITE |
		                    G_PARAM_EXPLICIT_NOTIFY |
		                    G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_DB_PLAYTIME] =
		g_param_spec_uint64("db-playtime",
		                    "Db Playtime",
		                    "Total playtime of all songs in the database in seconds",
		                    0, G_MAXUINT64, 0,
		                    G_PARAM_READWRITE |
		                    G_PARAM_EXPLICIT_NOTIFY |
		                    G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_DB_UPDATE] =
		g_param_spec_boxed("db-update",
		                   "Db Update",
		                   "Timestamp of the last database update",
		                   G_TYPE_DATE_TIME,
		                   G_PARAM_READWRITE |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_PLAYTIME] =
		g_param_spec_uint64("playtime",
		                    "Playtime",
		                    "Total playtime of the daemon in seconds",
		                    0, G_MAXUINT64, 0,
		                    G_PARAM_READWRITE |
		                    G_PARAM_EXPLICIT_NOTIFY |
		                    G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPERTIES, PROPERTIES);
}

static void
gmpd_stats_init(GMpdStats *self)
{
	self->artists = 0;
	self->albums = 0;
	self->songs = 0;
	self->uptime = 0;
	self->db_playtime = 0;
	self->db_update = NULL;
	self->playtime = 0;
}

GMpdStats *
gmpd_stats_new(void)
{
	return g_object_new(GMPD_TYPE_STATS, NULL);
}

void
gmpd_stats_set_artists(GMpdStats *self, guint artists)
{
	g_return_if_fail(GMPD_IS_STATS(self));

	if (self->artists != artists) {
		self->artists = artists;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_ARTISTS]);
	}
}

void
gmpd_stats_set_albums(GMpdStats *self, guint albums)
{
	g_return_if_fail(GMPD_IS_STATS(self));

	if (self->albums != albums) {
		self->albums = albums;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_ALBUMS]);
	}
}

void
gmpd_stats_set_songs(GMpdStats *self, guint songs)
{
	g_return_if_fail(GMPD_IS_STATS(self));

	if (self->songs != songs) {
		self->songs = songs;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_SONGS]);
	}
}

void
gmpd_stats_set_uptime(GMpdStats *self, guint64 uptime)
{
	g_return_if_fail(GMPD_IS_STATS(self));

	if (self->uptime != uptime) {
		self->uptime = uptime;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_UPTIME]);
	}
}

void
gmpd_stats_set_db_playtime(GMpdStats *self, guint64 db_playtime)
{
	g_return_if_fail(GMPD_IS_STATS(self));

	if (self->db_playtime != db_playtime) {
		self->db_playtime = db_playtime;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_DB_PLAYTIME]);
	}
}

void
gmpd_stats_set_db_update(GMpdStats *self, GDateTime *db_update)
{
	g_return_if_fail(GMPD_IS_STATS(self));

	if (self->db_update != db_update) {
		g_clear_pointer(&self->db_update, g_date_time_unref);
		self->db_update = db_update ? g_date_time_ref(db_update) : NULL;

		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_DB_UPDATE]);
	}
}

void
gmpd_stats_set_playtime(GMpdStats *self, guint64 playtime)
{
	g_return_if_fail(GMPD_IS_STATS(self));

	if (self->playtime != playtime) {
		self->playtime = playtime;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_PLAYTIME]);
	}
}

guint
gmpd_stats_get_artists(GMpdStats *self)
{
	g_return_val_if_fail(GMPD_IS_STATS(self), 0);
	return self->artists;
}

guint
gmpd_stats_get_albums(GMpdStats *self)
{
	g_return_val_if_fail(GMPD_IS_STATS(self), 0);
	return self->albums;
}

guint
gmpd_stats_get_songs(GMpdStats *self)
{
	g_return_val_if_fail(GMPD_IS_STATS(self), 0);
	return self->songs;
}

guint64
gmpd_stats_get_uptime(GMpdStats *self)
{
	g_return_val_if_fail(GMPD_IS_STATS(self), 0);
	return self->uptime;
}

guint64
gmpd_stats_get_db_playtime(GMpdStats *self)
{
	g_return_val_if_fail(GMPD_IS_STATS(self), 0);
	return self->db_playtime;
}

GDateTime *
gmpd_stats_get_db_update(GMpdStats *self)
{
	g_return_val_if_fail(GMPD_IS_STATS(self), NULL);
	return self->db_update;
}

guint64
gmpd_stats_get_playtime(GMpdStats *self)
{
	g_return_val_if_fail(GMPD_IS_STATS(self), 0);
	return self->playtime;
}

