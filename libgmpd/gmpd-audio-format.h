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

#ifndef __GMPD_AUDIO_FORMAT_H__
#define __GMPD_AUDIO_FORMAT_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

#define GMPD_TYPE_AUDIO_FORMAT \
	(gmpd_audio_format_get_type())

#define GMPD_AUDIO_FORMAT(inst) \
	(G_TYPE_CHECK_INSTANCE_CAST((inst), GMPD_TYPE_AUDIO_FORMAT, GMpdAudioFormat))

#define GMPD_AUDIO_FORMAT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GMPD_TYPE_AUDIO_FORMAT, GMpdAudioFormatClass))

#define GMPD_IS_AUDIO_FORMAT(inst) \
	(G_TYPE_CHECK_INSTANCE_TYPE((inst), GMPD_TYPE_AUDIO_FORMAT))

#define GMPD_IS_AUDIO_FORMAT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GMPD_TYPE_AUDIO_FORMAT))

#define GMPD_AUDIO_FORMAT_GET_CLASS(inst) \
	(G_TYPE_INSTANCE_GET_CLASS((inst), GMPD_TYPE_AUDIO_FORMAT, GMpdAudioFormatClass))

typedef enum _GMpdSampleFormat {
	GMPD_SAMPLE_FORMAT_UNDEFINED = 0x00,
	GMPD_SAMPLE_FORMAT_FLOAT = 0xe0,
	GMPD_SAMPLE_FORMAT_DSD = 0xe1,
} GMpdSampleFormat;

typedef struct _GMpdAudioFormat GMpdAudioFormat;
typedef struct _GMpdAudioFormatClass GMpdAudioFormatClass;

GType gmpd_audio_format_get_type(void);

GMpdAudioFormat *gmpd_audio_format_new(void);
GMpdAudioFormat *gmpd_audio_format_new_from_string(const gchar *s);

void gmpd_audio_format_set_sample_rate(GMpdAudioFormat *self, guint32 sample_rate);
void gmpd_audio_format_set_bit_depth(GMpdAudioFormat *self, guint8 bit_depth);
void gmpd_audio_format_set_channels(GMpdAudioFormat *self, guint8 bit_depth);

guint32 gmpd_audio_format_get_sample_rate(GMpdAudioFormat *self);
guint8 gmpd_audio_format_get_bit_depth(GMpdAudioFormat *self);
guint8 gmpd_audio_format_get_channels(GMpdAudioFormat *self);

G_END_DECLS

#endif /* __GMPD_AUDIO_FORMAT_H__ */

