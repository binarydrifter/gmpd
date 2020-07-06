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

#include "gmpd-audio-format.h"
#include "gmpd-entity.h"
#include "gmpd-entity-priv.h"
#include "gmpd-response.h"
#include "gmpd-response-priv.h"
#include "gmpd-song.h"
#include "gmpd-tag.h"
#include "gmpd-version.h"

static void gmpd_song_response_iface_init(GMpdResponseIface *iface);
static void gmpd_song_tag_changed(GMpdSong *self, GMpdTag tag);

enum {
	PROP_NONE,
	PROP_POSITION,
	PROP_ID,
	PROP_PRIORITY,
	PROP_DURATION,
	PROP_RANGE_START,
	PROP_RANGE_END,
	PROP_FORMAT,
	N_PROPERTIES,
};

enum {
	SIGNAL_TAG_CHANGED,
	N_SIGNALS,
};

struct _GMpdSong {
	GMpdEntity __base__;
	guint position;
	guint id;
	guint8 priority;
	float duration;
	float range_start;
	float range_end;
	GMpdAudioFormat *format;
	GPtrArray *tags[GMPD_N_TAGS];
};

struct _GMpdSongClass {
	GMpdEntityClass __base__;
	void (*tag_changed) (GMpdSong *self, GMpdTag tag);
};

G_DEFINE_TYPE_WITH_CODE(GMpdSong, gmpd_song, GMPD_TYPE_ENTITY,
                        G_IMPLEMENT_INTERFACE(GMPD_TYPE_RESPONSE,
                                              gmpd_song_response_iface_init))

static GParamSpec *PROPERTIES[N_PROPERTIES] = {NULL};
static guint SIGNALS[N_SIGNALS] = {0};

static void
gmpd_song_response_feed_pair(GMpdResponse *_self,
                             GMpdVersion *version,
                             const gchar *key,
                             const gchar *value)
{
	GMpdSong *self;
	GMpdTag tag;

	g_return_if_fail(GMPD_IS_SONG(_self));
	g_return_if_fail(GMPD_IS_VERSION(version));
	g_return_if_fail(key != NULL);
	g_return_if_fail(value != NULL);

	self = GMPD_SONG(_self);

	if (!g_strcmp0(key, "file")) {
		gmpd_entity_set_path(GMPD_ENTITY(self), value);

	} else if (!g_strcmp0(key, "Last-Modified")) {
		GDateTime *last_modified = g_date_time_new_from_iso8601(value, NULL);
		gmpd_entity_set_last_modified(GMPD_ENTITY(self), last_modified);
		g_clear_pointer(&last_modified, g_date_time_unref);

	} else if (!g_strcmp0(key, "Pos")) {
		gmpd_song_set_position(self, g_ascii_strtoull(value, NULL, 10));

	} else if (!g_strcmp0(key, "Id")) {
		gmpd_song_set_id(self, g_ascii_strtoull(value, NULL, 10));

	} else if (!g_strcmp0(key, "Prio")) {
		gmpd_song_set_priority(self, g_ascii_strtoull(value, NULL, 10));

	} else if (!g_strcmp0(key, "Time") && !self->duration) {
		gmpd_song_set_duration(self, g_ascii_strtod(value, NULL));

	} else if (!g_strcmp0(key, "duration")) {
		gmpd_song_set_duration(self, g_ascii_strtod(value, NULL));

	} else if (!g_strcmp0(key, "Range")) {
		gchar **parts = g_strsplit(value, "-", 2);
		if (!parts || g_strv_length(parts) != 2) {
			gmpd_song_set_range_start(self, 0);
			gmpd_song_set_range_end(self, 0);

		} else {
			gmpd_song_set_range_start(self, g_ascii_strtod(parts[0], NULL));
			gmpd_song_set_range_end(self, g_ascii_strtod(parts[1], NULL));
		}

		g_strfreev(parts);

	} else if (!g_strcmp0(key, "Format")) {
		GMpdAudioFormat *format = gmpd_audio_format_new_from_string(value);
		gmpd_song_set_format(self, format);
		g_clear_object(&format);

	} else if ((tag = gmpd_tag_from_string(key)) != GMPD_TAG_UNKNOWN) {
		if (!self->tags[tag]) {
			self->tags[tag] = g_ptr_array_new_full(2, g_free);
			g_ptr_array_add(self->tags[tag], NULL);
		}

		g_ptr_array_remove(self->tags[tag], NULL);
		g_ptr_array_add(self->tags[tag], g_strdup(value));
		g_ptr_array_add(self->tags[tag], NULL);

		gmpd_song_tag_changed(self, tag);

	} else {
		g_warning("%s: unknown key: %s", __func__, key);
	}
}

static void
gmpd_song_response_feed_binary(GMpdResponse *self G_GNUC_UNUSED,
                               GMpdVersion *version G_GNUC_UNUSED,
                               GBytes *bytes G_GNUC_UNUSED)
{
	g_critical("%s: GMpdSong does not receive binary data", __func__);
}

static gsize
gmpd_song_response_get_remaining_binary(GMpdResponse *self G_GNUC_UNUSED)
{
	return 0;
}

static void
gmpd_song_response_iface_init(GMpdResponseIface *iface)
{
	iface->feed_pair = gmpd_song_response_feed_pair;
	iface->feed_binary = gmpd_song_response_feed_binary;
	iface->get_remaining_binary = gmpd_song_response_get_remaining_binary;
}

static void
gmpd_song_tag_changed(GMpdSong *self, GMpdTag tag)
{
	GQuark detail;

	g_return_if_fail(GMPD_IS_SONG(self));
	g_return_if_fail(GMPD_TAG_IS_VALID(tag));

	detail = gmpd_tag_to_quark(tag);
	g_signal_emit(self, SIGNALS[SIGNAL_TAG_CHANGED], detail, tag);
}

static void
gmpd_song_set_property(GObject *object,
                       guint prop_id,
                       const GValue *value,
                       GParamSpec *pspec)
{
	GMpdSong *self = GMPD_SONG(object);

	switch (prop_id) {
	case PROP_POSITION:
		gmpd_song_set_position(self, g_value_get_uint(value));
		break;

	case PROP_ID:
		gmpd_song_set_id(self, g_value_get_uint(value));
		break;

	case PROP_PRIORITY:
		gmpd_song_set_priority(self, g_value_get_uint(value));
		break;

	case PROP_DURATION:
		gmpd_song_set_duration(self, g_value_get_float(value));
		break;

	case PROP_RANGE_START:
		gmpd_song_set_range_start(self, g_value_get_float(value));
		break;

	case PROP_RANGE_END:
		gmpd_song_set_range_end(self, g_value_get_float(value));
		break;

	case PROP_FORMAT:
		gmpd_song_set_format(self, g_value_get_object(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_song_get_property(GObject *object,
                       guint prop_id,
                       GValue *value,
                       GParamSpec *pspec)
{
	GMpdSong *self = GMPD_SONG(object);

	switch (prop_id) {
	case PROP_POSITION:
		g_value_set_uint(value, gmpd_song_get_position(self));
		break;

	case PROP_ID:
		g_value_set_uint(value, gmpd_song_get_id(self));
		break;

	case PROP_PRIORITY:
		g_value_set_uint(value, gmpd_song_get_priority(self));
		break;

	case PROP_DURATION:
		g_value_set_float(value, gmpd_song_get_duration(self));
		break;

	case PROP_RANGE_START:
		g_value_set_float(value, gmpd_song_get_range_start(self));
		break;

	case PROP_RANGE_END:
		g_value_set_float(value, gmpd_song_get_range_end(self));
		break;

	case PROP_FORMAT:
		g_value_take_object(value, gmpd_song_get_format(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_song_finalize(GObject *object)
{
	GMpdSong *self = GMPD_SONG(object);
	gsize i;

	g_clear_object(&self->format);

	for (i = 0; i < GMPD_N_TAGS; i++)
		g_clear_pointer(&self->tags[i], g_ptr_array_unref);

	G_OBJECT_CLASS(gmpd_song_parent_class)->finalize(object);
}

static void
gmpd_song_class_init(GMpdSongClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	klass->tag_changed = NULL;
	object_class->set_property = gmpd_song_set_property;
	object_class->get_property = gmpd_song_get_property;
	object_class->finalize = gmpd_song_finalize;

	PROPERTIES[PROP_POSITION] =
		g_param_spec_uint("position",
		                  "Position",
		                  "Position of the song within the queue",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_ID] =
		g_param_spec_uint("id",
		                  "Id",
		                  "Id of the song within the queue",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_PRIORITY] =
		g_param_spec_uint("priority",
		                  "Priority",
		                  "Priority of the song within the queue",
		                  0, G_MAXUINT8, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_DURATION] =
		g_param_spec_float("duration",
		                   "Duration",
		                   "Duration of the song in seconds",
		                   0, G_MAXFLOAT, 0,
		                   G_PARAM_READWRITE |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_RANGE_START] =
		g_param_spec_float("range-start",
		                   "Range Start",
		                   "Start of the virtual song in seconds",
		                   0, G_MAXFLOAT, 0,
		                   G_PARAM_READWRITE |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_RANGE_END] =
		g_param_spec_float("range-end",
		                   "Range End",
		                   "End of the virtual song in seconds",
		                   0, G_MAXFLOAT, 0,
		                   G_PARAM_READWRITE |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_FORMAT] =
		g_param_spec_object("format",
		                    "Format",
		                    "Audio format of the song",
		                    GMPD_TYPE_AUDIO_FORMAT,
		                    G_PARAM_READWRITE |
		                    G_PARAM_EXPLICIT_NOTIFY |
		                    G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPERTIES, PROPERTIES);

#define TAG_CHANGED_SIGNAL_FLAGS \
	(G_SIGNAL_RUN_FIRST | \
	 G_SIGNAL_NO_RECURSE | \
	 G_SIGNAL_DETAILED | \
	 G_SIGNAL_ACTION | \
	 G_SIGNAL_NO_HOOKS)

	SIGNALS[SIGNAL_TAG_CHANGED] =
		g_signal_new("tag-changed",
		             GMPD_TYPE_SONG,
		             TAG_CHANGED_SIGNAL_FLAGS,
		             G_STRUCT_OFFSET(GMpdSongClass, tag_changed),
		             NULL, NULL,
		             NULL,
		             G_TYPE_NONE,
		             1, GMPD_TYPE_TAG);

#undef TAG_CHANGED_SIGNAL_FLAGS
}

static void
gmpd_song_init(GMpdSong *self)
{
	size_t i;

	self->position = 0;
	self->id = 0;
	self->priority = 0;
	self->duration = 0;
	self->range_start = 0;
	self->range_end = 0;
	self->format = NULL;

	for (i = 0; i < GMPD_N_TAGS; i++)
		self->tags[i] = NULL;
}

GMpdSong *
gmpd_song_new(void)
{
	return g_object_new(GMPD_TYPE_SONG, NULL);
}

void
gmpd_song_set_position(GMpdSong *self, guint position)
{
	g_return_if_fail(GMPD_IS_SONG(self));

	if (self->position != position) {
		self->position = position;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_POSITION]);
	}
}

void
gmpd_song_set_id(GMpdSong *self, guint id)
{
	g_return_if_fail(GMPD_IS_SONG(self));

	if (self->id != id) {
		self->id = id;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_ID]);
	}
}

void
gmpd_song_set_priority(GMpdSong *self, guint8 priority)
{
	g_return_if_fail(GMPD_IS_SONG(self));

	if (self->priority != priority) {
		self->priority = priority;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_PRIORITY]);
	}
}

void
gmpd_song_set_duration(GMpdSong *self, float duration)
{
	g_return_if_fail(GMPD_IS_SONG(self));

	if (self->duration != duration) {
		self->duration = duration;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_DURATION]);
	}
}

void
gmpd_song_set_range_start(GMpdSong *self, float range_start)
{
	g_return_if_fail(GMPD_IS_SONG(self));

	if (self->range_start != range_start) {
		self->range_start = range_start;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_RANGE_START]);
	}
}

void
gmpd_song_set_range_end(GMpdSong *self, float range_end)
{
	g_return_if_fail(GMPD_IS_SONG(self));

	if (self->range_end != range_end) {
		self->range_end = range_end;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_RANGE_END]);
	}
}

void
gmpd_song_set_format(GMpdSong *self, GMpdAudioFormat *format)
{
	g_return_if_fail(GMPD_IS_SONG(self));
	g_return_if_fail(format == NULL || GMPD_IS_AUDIO_FORMAT(format));


	g_clear_object(&self->format);
	self->format = format ? g_object_ref(format) : NULL;

	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_FORMAT]);
}

void
gmpd_song_set_tag(GMpdSong *self, GMpdTag tag, const gchar *const *values)
{
	gsize values_len;
	GPtrArray *new_tag;
	gsize i;

	g_return_if_fail(GMPD_IS_SONG(self));
	g_return_if_fail(GMPD_TAG_IS_VALID(tag));

	if (!values || !values[0]) {
		g_clear_pointer(&self->tags[tag], g_ptr_array_unref);
		gmpd_song_tag_changed(self, tag);
		return;
	}

	values_len = g_strv_length((gchar **)values);
	new_tag = g_ptr_array_new_full(values_len, g_free);

	for (i = 0; i < values_len; i++) {
		gchar *s = g_strdup(values[i]);
		g_ptr_array_add(new_tag, s);
	}

	g_ptr_array_add(new_tag, NULL);

	g_clear_pointer(&self->tags[tag], g_ptr_array_unref);
	self->tags[tag] = new_tag;

	gmpd_song_tag_changed(self, tag);
}

guint
gmpd_song_get_position(GMpdSong *self)
{
	g_return_val_if_fail(GMPD_IS_SONG(self), 0);
	return self->position;
}

guint
gmpd_song_get_id(GMpdSong *self)
{
	g_return_val_if_fail(GMPD_IS_SONG(self), 0);
	return self->id;
}

guint8
gmpd_song_get_priority(GMpdSong *self)
{
	g_return_val_if_fail(GMPD_IS_SONG(self), 0);
	return self->priority;
}

float
gmpd_song_get_duration(GMpdSong *self)
{
	g_return_val_if_fail(GMPD_IS_SONG(self), 0);
	return self->duration;
}

float
gmpd_song_get_range_start(GMpdSong *self)
{
	g_return_val_if_fail(GMPD_IS_SONG(self), 0);
	return self->range_start;
}

float
gmpd_song_get_range_end(GMpdSong *self)
{
	g_return_val_if_fail(GMPD_IS_SONG(self), 0);
	return self->range_end;
}

GMpdAudioFormat *
gmpd_song_get_format(GMpdSong *self)
{
	g_return_val_if_fail(GMPD_IS_SONG(self), NULL);
	return self->format ? g_object_ref(self->format) : NULL;
}

gchar **
gmpd_song_get_tag(GMpdSong *self, GMpdTag tag)
{
	g_return_val_if_fail(GMPD_IS_SONG(self), NULL);
	g_return_val_if_fail(GMPD_TAG_IS_VALID(tag), NULL);

	if (!self->tags[tag] || !self->tags[tag]->len)
		return NULL;

	return g_strdupv((gchar **)self->tags[tag]->pdata);
}

