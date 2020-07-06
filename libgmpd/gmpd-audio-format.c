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

static GRegex *gmpd_audio_format_get_regex(void);

enum {
	PROP_NONE,
	PROP_SAMPLE_RATE,
	PROP_BIT_DEPTH,
	PROP_CHANNELS,
	N_PROPERTIES,
};

struct _GMpdAudioFormat {
	GObject __base__;
	guint32 sample_rate;
	guint8 bit_depth;
	guint8 channels;
};

struct _GMpdAudioFormatClass {
	GObjectClass __base__;
};

G_DEFINE_TYPE(GMpdAudioFormat, gmpd_audio_format, G_TYPE_OBJECT)

static GParamSpec *PROPERTIES[N_PROPERTIES] = { NULL };

static void
gmpd_audio_format_set_property(GObject *object,
                              guint prop_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
	GMpdAudioFormat *self = GMPD_AUDIO_FORMAT(object);

	switch (prop_id) {
	case PROP_SAMPLE_RATE:
		gmpd_audio_format_set_sample_rate(self, g_value_get_uint(value));
		break;

	case PROP_BIT_DEPTH:
		gmpd_audio_format_set_bit_depth(self, g_value_get_uint(value));
		break;

	case PROP_CHANNELS:
		gmpd_audio_format_set_channels(self, g_value_get_uint(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_audio_format_get_property(GObject *object,
                              guint prop_id,
                              GValue *value,
                              GParamSpec *pspec)
{
	GMpdAudioFormat *self = GMPD_AUDIO_FORMAT(object);

	switch (prop_id) {
	case PROP_SAMPLE_RATE:
		g_value_set_uint(value, gmpd_audio_format_get_sample_rate(self));
		break;

	case PROP_BIT_DEPTH:
		g_value_set_uint(value, gmpd_audio_format_get_bit_depth(self));
		break;

	case PROP_CHANNELS:
		g_value_set_uint(value, gmpd_audio_format_get_channels(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_audio_format_class_init(GMpdAudioFormatClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = gmpd_audio_format_set_property;
	object_class->get_property = gmpd_audio_format_get_property;

	PROPERTIES[PROP_SAMPLE_RATE] =
		g_param_spec_uint("sample-rate",
		                  "Sample Rate",
		                  "The sample rate in Hz",
		                  0, G_MAXUINT32, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_BIT_DEPTH] =
		g_param_spec_uint("bit-depth",
		                  "Bit Depth",
		                  "The number of bits per sample",
		                  0, G_MAXUINT8, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_CHANNELS] =
		g_param_spec_uint("channels",
		                  "Channels",
		                  "The number of channels",
		                  0, G_MAXUINT8, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPERTIES, PROPERTIES);
}

static void
gmpd_audio_format_init(GMpdAudioFormat *self)
{
	self->sample_rate = 0;
	self->bit_depth = 0;
	self->channels = 0;
}

GMpdAudioFormat *
gmpd_audio_format_new(void)
{
	return g_object_new(GMPD_TYPE_AUDIO_FORMAT, NULL);
}

GMpdAudioFormat *
gmpd_audio_format_new_from_string(const gchar *s)
{
	GRegex *regex;
	GMatchInfo *match_info;

	GMpdAudioFormat *afmt;
	gchar *pos_1;
	gchar *pos_2;
	gchar *pos_3;

	g_return_val_if_fail(s != NULL, NULL);

	regex = gmpd_audio_format_get_regex();

	if (!g_regex_match(regex, s, 0, &match_info)) {
		g_match_info_unref(match_info);
		return NULL;
	}

	afmt = gmpd_audio_format_new();
	pos_1 = g_match_info_fetch(match_info, 1);
	pos_2 = g_match_info_fetch(match_info, 2);
	pos_3 = g_match_info_fetch(match_info, 3);

	afmt->sample_rate = g_ascii_strtoull(pos_1, NULL, 10);
	afmt->bit_depth = g_ascii_strtoull(pos_2, NULL, 10);
	afmt->channels = g_ascii_strtoull(pos_3, NULL, 10);

	g_free(pos_1);
	g_free(pos_2);
	g_free(pos_3);
	g_match_info_unref(match_info);

	return afmt;
}

void
gmpd_audio_format_set_sample_rate(GMpdAudioFormat *self, guint32 sample_rate)
{
	g_return_if_fail(self != NULL);

	if (self->sample_rate != sample_rate) {
		self->sample_rate = sample_rate;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_SAMPLE_RATE]);
	}
}

void
gmpd_audio_format_set_bit_depth(GMpdAudioFormat *self, guint8 bit_depth)
{
	g_return_if_fail(self != NULL);

	if (self->bit_depth != bit_depth) {
		self->bit_depth = bit_depth;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_BIT_DEPTH]);
	}
}

void
gmpd_audio_format_set_channels(GMpdAudioFormat *self, guint8 channels)
{
	g_return_if_fail(self != NULL);

	if (self->channels != channels) {
		self->channels = channels;
		g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_CHANNELS]);
	}
}

guint32
gmpd_audio_format_get_sample_rate(GMpdAudioFormat *self)
{
	g_return_val_if_fail(self != NULL, 0);
	return self->sample_rate;
}

guint8
gmpd_audio_format_get_bit_depth(GMpdAudioFormat *self)
{
	g_return_val_if_fail(self != NULL, 0);
	return self->bit_depth;
}

guint8
gmpd_audio_format_get_channels(GMpdAudioFormat *self)
{
	g_return_val_if_fail(self != NULL, 0);
	return self->channels;
}

static GRegex *
gmpd_audio_format_get_regex(void)
{
	static const gchar *const regex_string = "(\\d+):(\\d+):(\\d+)";
	static GRegex *regex = NULL;
	static volatile gsize initialized = 0;

	if (g_once_init_enter(&initialized)) {
		GError *err = NULL;

		regex = g_regex_new(regex_string, G_REGEX_OPTIMIZE, 0, &err);
		if (!regex)
			g_error("failed to compile regex: %s", err->message);

		g_once_init_leave(&initialized, 1);
	}

	g_assert(regex != NULL);
	return regex;
}

