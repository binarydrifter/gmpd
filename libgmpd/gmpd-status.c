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
#include "gmpd-option-state.h"
#include "gmpd-playback-state.h"
#include "gmpd-response.h"
#include "gmpd-status.h"
#include "gmpd-version.h"


static void gmpd_status_response_iface_init(GMpdResponseIface *iface);

enum {
	PROP_NONE,
	PROP_PARTITION,
	PROP_VOLUME,
	PROP_REPEAT,
	PROP_RANDOM,
	PROP_SINGLE,
	PROP_CONSUME,
	PROP_QUEUE_VERSION,
	PROP_QUEUE_LENGTH,
	PROP_PLAYBACK,
	PROP_CURRENT_POSITION,
	PROP_CURRENT_ID,
	PROP_NEXT_POSITION,
	PROP_NEXT_ID,
	PROP_CURRENT_ELAPSED,
	PROP_CURRENT_DURATION,
	PROP_BIT_RATE,
	PROP_CROSSFADE,
	PROP_MIXRAMP_DB,
	PROP_MIXRAMP_DELAY,
	PROP_AUDIO_FORMAT,
	PROP_DB_UPDATE_JOB_ID,
	PROP_ERROR,
	N_PROPERTIES,
};

struct _GMpdStatus {
	GObject           __base__;
	gchar            *partition;
	gint8             volume;
	GMpdOptionState   repeat;
	GMpdOptionState   random;
	GMpdOptionState   single;
	GMpdOptionState   consume;
	guint             queue_version;
	guint             queue_length;
	GMpdPlaybackState playback;
	guint             current_position;
	guint             current_id;
	guint             next_position;
	guint             next_id;
	gfloat            current_elapsed;
	gfloat            current_duration;
	guint             bit_rate;
	guint             crossfade;
	gfloat            mixramp_db;
	gfloat            mixramp_delay;
	GMpdAudioFormat  *audio_format;
	guint             db_update_job_id;
	gchar            *error;
};

struct _GMpdStatusClass {
	GObjectClass __base__;
};

G_DEFINE_TYPE_WITH_CODE(GMpdStatus, gmpd_status, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(GMPD_TYPE_RESPONSE,
                                              gmpd_status_response_iface_init))


static GParamSpec *PROPERTIES[N_PROPERTIES] = {NULL};

static void
gmpd_status_response_feed_pair(GMpdResponse *response,
                               GMpdVersion  *version,
                               const gchar  *key,
                               const gchar  *value)
{
	GMpdStatus *self;

	g_return_if_fail(GMPD_IS_STATUS(response));
	g_return_if_fail(GMPD_IS_VERSION(version));
	g_return_if_fail(key != NULL);
	g_return_if_fail(value != NULL);

	self = GMPD_STATUS(response);

	if (g_strcmp0(key, "partition") == 0) {
		gmpd_status_set_partition(self, value);

	} else if (g_strcmp0(key, "volume") == 0) {
		gmpd_status_set_volume(self, g_ascii_strtoll(value, NULL, 10));

	} else if (g_strcmp0(key, "repeat") == 0) {
		gmpd_status_set_repeat(self, gmpd_option_state_from_string(value));

	} else if (g_strcmp0(key, "random") == 0) {
		gmpd_status_set_random(self, gmpd_option_state_from_string(value));

	} else if (g_strcmp0(key, "single") == 0) {
		gmpd_status_set_single(self, gmpd_option_state_from_string(value));

	} else if (g_strcmp0(key, "consume") == 0) {
		gmpd_status_set_consume(self, gmpd_option_state_from_string(value));

	} else if (g_strcmp0(key, "playlist") == 0) {
		gmpd_status_set_queue_version(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "playlistlength") == 0) {
		gmpd_status_set_queue_length(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "state") == 0) {
		gmpd_status_set_playback(self, gmpd_playback_state_from_string(value));

	} else if (g_strcmp0(key, "song") == 0) {
		gmpd_status_set_current_position(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "songid") == 0) {
		gmpd_status_set_current_id(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "nextsong") == 0) {
		gmpd_status_set_next_position(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "nextsongid") == 0) {
		gmpd_status_set_next_id(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "time") == 0) {
		/* TODO */

	} else if (g_strcmp0(key, "elapsed") == 0) {
		gmpd_status_set_current_elapsed(self, g_ascii_strtod(value, NULL));

	} else if (g_strcmp0(key, "duration") == 0) {
		gmpd_status_set_current_duration(self, g_ascii_strtod(value, NULL));

	} else if (g_strcmp0(key, "bitrate") == 0) {
		gmpd_status_set_bit_rate(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "xfade") == 0) {
		gmpd_status_set_crossfade(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "mixrampdb") == 0) {
		gmpd_status_set_mixramp_db(self, g_ascii_strtod(value, NULL));

	} else if (g_strcmp0(key, "mixrampdelay") == 0) {
		gmpd_status_set_mixramp_delay(self, g_ascii_strtod(value, NULL));

	} else if (g_strcmp0(key, "audio") == 0) {
		gmpd_status_set_audio_format(self, gmpd_audio_format_new_from_string(value));

	} else if (g_strcmp0(key, "updating_db") == 0) {
		gmpd_status_set_db_update_job_id(self, g_ascii_strtoull(value, NULL, 10));

	} else if (g_strcmp0(key, "error") == 0) {
		gmpd_status_set_error(self, value);

	} else {
		g_warning("invalid key: %s", key);
	}
}

static void
gmpd_status_response_iface_init(GMpdResponseIface *iface)
{
	iface->feed_pair = gmpd_status_response_feed_pair;
}

static void
gmpd_status_set_property(GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
	GMpdStatus *self = GMPD_STATUS(object);

	switch (prop_id) {
	case PROP_PARTITION:
		gmpd_status_set_partition(self, g_value_get_string(value));
		break;

	case PROP_VOLUME:
		gmpd_status_set_volume(self, g_value_get_int(value));
		break;

	case PROP_REPEAT:
		gmpd_status_set_repeat(self, g_value_get_enum(value));
		break;

	case PROP_RANDOM:
		gmpd_status_set_random(self, g_value_get_enum(value));
		break;

	case PROP_SINGLE:
		gmpd_status_set_single(self, g_value_get_enum(value));
		break;

	case PROP_CONSUME:
		gmpd_status_set_consume(self, g_value_get_enum(value));
		break;

	case PROP_QUEUE_VERSION:
		gmpd_status_set_queue_version(self, g_value_get_uint(value));
		break;

	case PROP_QUEUE_LENGTH:
		gmpd_status_set_queue_length(self, g_value_get_uint(value));
		break;

	case PROP_PLAYBACK:
		gmpd_status_set_playback(self, g_value_get_enum(value));
		break;

	case PROP_CURRENT_POSITION:
		gmpd_status_set_current_position(self, g_value_get_uint(value));
		break;

	case PROP_CURRENT_ID:
		gmpd_status_set_current_id(self, g_value_get_uint(value));
		break;

	case PROP_NEXT_POSITION:
		gmpd_status_set_next_position(self, g_value_get_uint(value));
		break;

	case PROP_NEXT_ID:
		gmpd_status_set_next_id(self, g_value_get_uint(value));
		break;

	case PROP_CURRENT_ELAPSED:
		gmpd_status_set_current_elapsed(self, g_value_get_float(value));
		break;

	case PROP_CURRENT_DURATION:
		gmpd_status_set_current_duration(self, g_value_get_float(value));
		break;

	case PROP_BIT_RATE:
		gmpd_status_set_bit_rate(self, g_value_get_uint(value));
		break;

	case PROP_CROSSFADE:
		gmpd_status_set_crossfade(self, g_value_get_uint(value));
		break;

	case PROP_MIXRAMP_DB:
		gmpd_status_set_mixramp_db(self, g_value_get_float(value));
		break;

	case PROP_MIXRAMP_DELAY:
		gmpd_status_set_mixramp_delay(self, g_value_get_float(value));
		break;

	case PROP_AUDIO_FORMAT:
		gmpd_status_set_audio_format(self, g_value_get_object(value));
		break;

	case PROP_DB_UPDATE_JOB_ID:
		gmpd_status_set_db_update_job_id(self, g_value_get_uint(value));
		break;

	case PROP_ERROR:
		gmpd_status_set_error(self, g_value_get_string(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_status_get_property(GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
	GMpdStatus *self = GMPD_STATUS(object);

	switch (prop_id) {
	case PROP_PARTITION:
		g_value_take_string(value, gmpd_status_get_partition(self));
		break;

	case PROP_VOLUME:
		g_value_set_int(value, gmpd_status_get_volume(self));
		break;

	case PROP_REPEAT:
		g_value_set_enum(value, gmpd_status_get_repeat(self));
		break;

	case PROP_RANDOM:
		g_value_set_enum(value, gmpd_status_get_random(self));
		break;

	case PROP_SINGLE:
		g_value_set_enum(value, gmpd_status_get_single(self));
		break;

	case PROP_CONSUME:
		g_value_set_enum(value, gmpd_status_get_consume(self));
		break;

	case PROP_QUEUE_VERSION:
		g_value_set_uint(value, gmpd_status_get_queue_version(self));
		break;

	case PROP_QUEUE_LENGTH:
		g_value_set_uint(value, gmpd_status_get_queue_length(self));
		break;

	case PROP_PLAYBACK:
		g_value_set_enum(value, gmpd_status_get_playback(self));
		break;

	case PROP_CURRENT_POSITION:
		g_value_set_uint(value, gmpd_status_get_current_position(self));
		break;

	case PROP_CURRENT_ID:
		g_value_set_uint(value, gmpd_status_get_current_id(self));
		break;

	case PROP_NEXT_POSITION:
		g_value_set_uint(value, gmpd_status_get_next_position(self));
		break;

	case PROP_NEXT_ID:
		g_value_set_uint(value, gmpd_status_get_next_id(self));
		break;

	case PROP_CURRENT_ELAPSED:
		g_value_set_float(value, gmpd_status_get_current_elapsed(self));
		break;

	case PROP_CURRENT_DURATION:
		g_value_set_float(value, gmpd_status_get_current_duration(self));
		break;

	case PROP_BIT_RATE:
		g_value_set_uint(value, gmpd_status_get_bit_rate(self));
		break;

	case PROP_CROSSFADE:
		g_value_set_uint(value, gmpd_status_get_crossfade(self));
		break;

	case PROP_MIXRAMP_DB:
		g_value_set_float(value, gmpd_status_get_mixramp_db(self));
		break;

	case PROP_MIXRAMP_DELAY:
		g_value_set_float(value, gmpd_status_get_mixramp_delay(self));
		break;

	case PROP_AUDIO_FORMAT:
		g_value_take_object(value, gmpd_status_get_audio_format(self));
		break;

	case PROP_DB_UPDATE_JOB_ID:
		g_value_set_uint(value, gmpd_status_get_db_update_job_id(self));
		break;

	case PROP_ERROR:
		g_value_take_string(value, gmpd_status_get_error(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_status_finalize(GObject *object)
{
	GMpdStatus *self = GMPD_STATUS(object);

	g_free(self->partition);

	if (self->audio_format)
		g_object_unref(self->audio_format);

	g_free(self->error);

	G_OBJECT_CLASS(gmpd_status_parent_class)->finalize(object);
}

static void
gmpd_status_class_init(GMpdStatusClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = gmpd_status_set_property;
	object_class->get_property = gmpd_status_get_property;
	object_class->finalize = gmpd_status_finalize;

	PROPERTIES[PROP_PARTITION] =
		g_param_spec_string("partition",
		                    "Partition",
		                    "Name of the current partition",
		                    NULL,
		                    G_PARAM_READWRITE |
		                    G_PARAM_EXPLICIT_NOTIFY |
		                    G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_VOLUME] =
		g_param_spec_int("volume",
		                 "Volume",
		                 "Volume of the audio playback",
		                 -1, 100, -1,
		                 G_PARAM_READWRITE |
		                 G_PARAM_EXPLICIT_NOTIFY |
		                 G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_REPEAT] =
		g_param_spec_enum("repeat",
		                  "Repeat",
		                  "State of the repeat option",
		                  GMPD_TYPE_OPTION_STATE,
		                  GMPD_OPTION_UNKNOWN,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_RANDOM] =
		g_param_spec_enum("random",
		                  "Random",
		                  "State of the random option",
		                  GMPD_TYPE_OPTION_STATE,
		                  GMPD_OPTION_UNKNOWN,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_SINGLE] =
		g_param_spec_enum("single",
		                  "Single",
		                  "State of the single option",
		                  GMPD_TYPE_OPTION_STATE,
		                  GMPD_OPTION_UNKNOWN,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_CONSUME] =
		g_param_spec_enum("consume",
		                  "Consume",
		                  "State of the consume option",
		                  GMPD_TYPE_OPTION_STATE,
		                  GMPD_OPTION_UNKNOWN,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_QUEUE_VERSION] =
		g_param_spec_uint("queue-version",
		                  "Queue Version",
		                  "The queue version number",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_QUEUE_LENGTH] =
		g_param_spec_uint("queue-length",
		                  "Queue Length",
		                  "Number of songs in the queue",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_PLAYBACK] =
		g_param_spec_enum("playback",
		                  "Playback",
		                  "Current playback state",
		                  GMPD_TYPE_PLAYBACK_STATE,
		                  GMPD_PLAYBACK_UNKNOWN,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_CURRENT_POSITION] =
		g_param_spec_uint("current-position",
		                  "Current Position",
		                  "Position of the currently playing song",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_CURRENT_ID] =
		g_param_spec_uint("current-id",
		                  "Current Id",
		                  "Id of the currently playing song",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);


	PROPERTIES[PROP_NEXT_POSITION] =
		g_param_spec_uint("next-position",
		                  "Next Position",
		                  "Position of the next song in the queue",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_NEXT_ID] =
		g_param_spec_uint("next-id",
		                  "Next Id",
		                  "Id of the next song in the queue",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_CURRENT_ELAPSED] =
		g_param_spec_float("current-elapsed",
		                   "Current Elapsed",
		                   "Duration of the current song in seconds",
		                   0, G_MAXFLOAT, 0,
		                   G_PARAM_READWRITE |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_CURRENT_DURATION] =
		g_param_spec_float("current-duration",
		                   "Current Duration",
		                   "Duration of the current song in seconds",
		                   0, G_MAXFLOAT, 0,
		                   G_PARAM_READWRITE |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_BIT_RATE] =
		g_param_spec_uint("bit-rate",
		                  "Bit Rate",
		                  "Bit rate in Kbps",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_CROSSFADE] =
		g_param_spec_uint("crossfade",
		                  "Crossfade",
		                  "Crossfade in seconds",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_MIXRAMP_DB] =
		g_param_spec_float("mixramp-db",
		                   "MixRamp Db",
		                   "MixRamp threshold in dB",
		                   -G_MAXFLOAT, G_MAXFLOAT, 0,
		                   G_PARAM_READWRITE |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_MIXRAMP_DELAY] =
		g_param_spec_float("mixramp-delay",
		                   "MixRamp Delay",
		                   "MixRamp delay in seconds",
		                   0, G_MAXFLOAT, 0,
		                   G_PARAM_READWRITE |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_AUDIO_FORMAT] =
		g_param_spec_object("audio-format",
		                    "Audio Format",
		                    "Audio format of the decoder plugin",
		                    GMPD_TYPE_AUDIO_FORMAT,
		                    G_PARAM_READWRITE |
		                    G_PARAM_EXPLICIT_NOTIFY |
		                    G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_DB_UPDATE_JOB_ID] =
		g_param_spec_uint("db-update-job-id",
		                  "Db Update Job Id",
		                  "Job Id of the current database update",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_ERROR] =
		g_param_spec_string("error",
		                    "Error",
		                    "Error string returned from the server",
		                    NULL,
		                    G_PARAM_READWRITE |
		                    G_PARAM_EXPLICIT_NOTIFY |
		                    G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPERTIES, PROPERTIES);
}

static void
gmpd_status_init(GMpdStatus *self)
{
	self->partition = NULL;
	self->volume = -1;
	self->repeat = GMPD_OPTION_UNKNOWN;
	self->random = GMPD_OPTION_UNKNOWN;
	self->single = GMPD_OPTION_UNKNOWN;
	self->consume = GMPD_OPTION_UNKNOWN;
	self->queue_version = 0;
	self->queue_length = 0;
	self->playback = GMPD_PLAYBACK_UNKNOWN;
	self->current_position = 0;
	self->current_id = 0;
	self->next_position = 0;
	self->next_id = 0;
	self->current_elapsed = 0;
	self->current_duration = 0;
	self->bit_rate = 0;
	self->crossfade = 0;
	self->mixramp_db = 0;
	self->mixramp_delay = 0;
	self->audio_format = NULL;
	self->db_update_job_id = 0;
	self->error = NULL;
}

GMpdStatus *
gmpd_status_new(void)
{
	return g_object_new(GMPD_TYPE_STATUS, NULL);
}

void
gmpd_status_set_partition(GMpdStatus  *self,
                          const gchar *partition)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->partition = g_strdup(partition);
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_PARTITION]);
}

void
gmpd_status_set_volume(GMpdStatus *self,
                       gint8       volume)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	if (volume < -1 || volume > 100)
		volume = -1;

	self->volume = volume;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_VOLUME]);
}

void
gmpd_status_set_repeat(GMpdStatus     *self,
                       GMpdOptionState repeat)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	if (!GMPD_IS_OPTION_STATE(repeat))
		repeat = GMPD_OPTION_UNKNOWN;

	self->repeat = repeat;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_REPEAT]);
}

void
gmpd_status_set_random(GMpdStatus     *self,
                       GMpdOptionState random)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	if (!GMPD_IS_OPTION_STATE(random))
		random = GMPD_OPTION_UNKNOWN;

	self->random = random;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_RANDOM]);
}

void
gmpd_status_set_single(GMpdStatus     *self,
                       GMpdOptionState single)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	if (!GMPD_IS_OPTION_STATE(single))
		single = GMPD_OPTION_UNKNOWN;

	self->single = single;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_SINGLE]);
}

void
gmpd_status_set_consume(GMpdStatus     *self,
                        GMpdOptionState consume)
{
	g_return_if_fail(GMPD_IS_STATUS(self));
	g_return_if_fail(GMPD_IS_OPTION_STATE(consume));

	if (!GMPD_IS_OPTION_STATE(consume))
		consume = GMPD_OPTION_UNKNOWN;

	self->consume = consume;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_CONSUME]);
}

void
gmpd_status_set_queue_version(GMpdStatus *self,
                              guint       queue_version)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->queue_version = queue_version;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_QUEUE_VERSION]);
}

void
gmpd_status_set_queue_length(GMpdStatus *self,
                             guint       queue_length)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->queue_length = queue_length;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_QUEUE_LENGTH]);
}

void
gmpd_status_set_playback(GMpdStatus       *self,
                         GMpdPlaybackState playback)
{
	g_return_if_fail(GMPD_IS_STATUS(self));
	g_return_if_fail(GMPD_IS_PLAYBACK_STATE(playback));

	if (!GMPD_IS_PLAYBACK_STATE(playback))
		playback = GMPD_PLAYBACK_UNKNOWN;

	self->playback = playback;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_PLAYBACK]);
}

void
gmpd_status_set_current_position(GMpdStatus *self,
                                 guint       current_position)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->current_position = current_position;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_CURRENT_POSITION]);
}

void
gmpd_status_set_current_id(GMpdStatus *self,
                           guint       current_id)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->current_id = current_id;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_CURRENT_ID]);
}

void
gmpd_status_set_next_position(GMpdStatus *self,
                              guint       next_position)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->next_position = next_position;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_NEXT_POSITION]);
}

void
gmpd_status_set_next_id(GMpdStatus *self,
                        guint       next_id)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->next_id = next_id;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_NEXT_ID]);
}

void
gmpd_status_set_current_elapsed(GMpdStatus *self,
                                gfloat      current_elapsed)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	if (current_elapsed < 0)
		current_elapsed = 0;

	self->current_elapsed = current_elapsed;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_CURRENT_ELAPSED]);
}

void
gmpd_status_set_current_duration(GMpdStatus *self,
                                 gfloat      current_duration)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	if (current_duration < 0)
		 current_duration = 0;

	self->current_duration = current_duration;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_CURRENT_DURATION]);
}

void
gmpd_status_set_bit_rate(GMpdStatus *self,
                         guint       bit_rate)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->bit_rate = bit_rate;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_BIT_RATE]);
}

void
gmpd_status_set_crossfade(GMpdStatus *self,
                          guint       crossfade)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->crossfade = crossfade;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_CROSSFADE]);
}

void
gmpd_status_set_mixramp_db(GMpdStatus *self,
                           gfloat      mixramp_db)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->mixramp_db = mixramp_db;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_MIXRAMP_DB]);
}

void
gmpd_status_set_mixramp_delay(GMpdStatus *self,
                              gfloat      mixramp_delay)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	if (mixramp_delay < 0)
		mixramp_delay = 0;

	self->mixramp_delay = mixramp_delay;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_MIXRAMP_DELAY]);
}

void
gmpd_status_set_audio_format(GMpdStatus      *self,
                             GMpdAudioFormat *audio_format)
{
	g_return_if_fail(GMPD_IS_STATUS(self));
	g_return_if_fail(audio_format == NULL || GMPD_IS_AUDIO_FORMAT(audio_format));

	g_clear_object(&self->audio_format);
	self->audio_format = audio_format ? g_object_ref(audio_format) : NULL;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_AUDIO_FORMAT]);
}

void
gmpd_status_set_db_update_job_id(GMpdStatus *self,
                                 guint       db_update_job_id)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	self->db_update_job_id = db_update_job_id;
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_DB_UPDATE_JOB_ID]);
}

void
gmpd_status_set_error(GMpdStatus  *self,
                      const gchar *error)
{
	g_return_if_fail(GMPD_IS_STATUS(self));

	g_free(self->error);
	self->error = g_strdup(error);
	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_ERROR]);
}

gchar *
gmpd_status_get_partition(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), NULL);
	return g_strdup(self->partition);
}

gint8
gmpd_status_get_volume(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->volume;
}

GMpdOptionState
gmpd_status_get_repeat(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), GMPD_OPTION_UNKNOWN);
	return self->repeat;
}

GMpdOptionState
gmpd_status_get_random(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), GMPD_OPTION_UNKNOWN);
	return self->random;
}

GMpdOptionState
gmpd_status_get_single(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), GMPD_OPTION_UNKNOWN);
	return self->single;
}

GMpdOptionState
gmpd_status_get_consume(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), GMPD_OPTION_UNKNOWN);
	return self->consume;
}

guint
gmpd_status_get_queue_version(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->queue_version;
}

guint
gmpd_status_get_queue_length(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->queue_length;
}

GMpdPlaybackState
gmpd_status_get_playback(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), GMPD_PLAYBACK_UNKNOWN);
	return self->playback;
}

guint
gmpd_status_get_current_position(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->current_position;
}

guint
gmpd_status_get_current_id(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->current_id;
}

guint
gmpd_status_get_next_position(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->next_position;
}

guint
gmpd_status_get_next_id(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->next_id;
}

gfloat
gmpd_status_get_current_elapsed(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->current_elapsed;
}

gfloat
gmpd_status_get_current_duration(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->current_duration;
}

guint
gmpd_status_get_bit_rate(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->bit_rate;
}

guint
gmpd_status_get_crossfade(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->crossfade;
}

gfloat
gmpd_status_get_mixramp_db(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->mixramp_db;
}

gfloat
gmpd_status_get_mixramp_delay(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->mixramp_delay;
}

GMpdAudioFormat *
gmpd_status_get_audio_format(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), NULL);
	return self->audio_format ? g_object_ref(self->audio_format) : NULL;
}

guint
gmpd_status_get_db_update_job_id(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), 0);
	return self->db_update_job_id;
}

gchar *
gmpd_status_get_error(GMpdStatus *self)
{
	g_return_val_if_fail(GMPD_IS_STATUS(self), NULL);
	return g_strdup(self->error);
}

