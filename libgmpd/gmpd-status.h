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

#ifndef __GMPD_STATUS_H__
#define __GMPD_STATUS_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gmpd-audio-format.h>
#include <gmpd-option-state.h>
#include <gmpd-playback-state.h>

G_BEGIN_DECLS

#define GMPD_TYPE_STATUS \
	(gmpd_status_get_type())

#define GMPD_STATUS(inst) \
	(G_TYPE_CHECK_INSTANCE_CAST((inst), GMPD_TYPE_STATUS, GMpdStatus))

#define GMPD_STATUS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GMPD_TYPE_STATUS, GMpdStatusClass))

#define GMPD_IS_STATUS(inst) \
	(G_TYPE_CHECK_INSTANCE_TYPE((inst), GMPD_TYPE_STATUS))

#define GMPD_IS_STATUS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GMPD_TYPE_STATUS))

#define GMPD_STATUS_GET_CLASS(inst) \
	(G_TYPE_INSTANCE_GET_CLASS((inst), GMPD_TYPE_STATUS, GMpdStatusClass))

typedef struct _GMpdStatus GMpdStatus;
typedef struct _GMpdStatusClass GMpdStatusClass;

GType gmpd_status_get_type(void);

GMpdStatus *gmpd_status_new(void);

void gmpd_status_set_partition(GMpdStatus *self, const gchar *partition);
void gmpd_status_set_volume(GMpdStatus *self, gint8 volume);
void gmpd_status_set_repeat(GMpdStatus *self, GMpdOptionState repeat);
void gmpd_status_set_random(GMpdStatus *self, GMpdOptionState random);
void gmpd_status_set_single(GMpdStatus *self, GMpdOptionState single);
void gmpd_status_set_consume(GMpdStatus *self, GMpdOptionState consume);
void gmpd_status_set_queue_version(GMpdStatus *self, guint queue_version);
void gmpd_status_set_queue_length(GMpdStatus *self, guint queue_length);
void gmpd_status_set_playback(GMpdStatus *self, GMpdPlaybackState playback);
void gmpd_status_set_current_position(GMpdStatus *self, guint current_position);
void gmpd_status_set_current_id(GMpdStatus *self, guint current_id);
void gmpd_status_set_next_position(GMpdStatus *self, guint next_position);
void gmpd_status_set_next_id(GMpdStatus *self, guint next_id);
void gmpd_status_set_current_elapsed(GMpdStatus *self, gfloat current_elapsed);
void gmpd_status_set_current_duration(GMpdStatus *self, gfloat current_duration);
void gmpd_status_set_bit_rate(GMpdStatus *self, guint bit_rate);
void gmpd_status_set_crossfade(GMpdStatus *self, guint crossfade);
void gmpd_status_set_mixramp_db(GMpdStatus *self, gfloat mixramp_db);
void gmpd_status_set_mixramp_delay(GMpdStatus *self, gfloat mixramp_delay);
void gmpd_status_set_audio_format(GMpdStatus *self, GMpdAudioFormat *audio_format);
void gmpd_status_set_db_update_job_id(GMpdStatus *self, guint db_update_job_id);
void gmpd_status_set_error(GMpdStatus *self, const gchar *error);

gchar *gmpd_status_get_partition(GMpdStatus *self);
gint8 gmpd_status_get_volume(GMpdStatus *self);
GMpdOptionState gmpd_status_get_repeat(GMpdStatus *self);
GMpdOptionState gmpd_status_get_random(GMpdStatus *self);
GMpdOptionState gmpd_status_get_single(GMpdStatus *self);
GMpdOptionState gmpd_status_get_consume(GMpdStatus *self);
guint gmpd_status_get_queue_version(GMpdStatus *self);
guint gmpd_status_get_queue_length(GMpdStatus *self);
GMpdPlaybackState gmpd_status_get_playback(GMpdStatus *self);
guint gmpd_status_get_current_position(GMpdStatus *self);
guint gmpd_status_get_current_id(GMpdStatus *self);
guint gmpd_status_get_next_position(GMpdStatus *self);
guint gmpd_status_get_next_id(GMpdStatus *self);
gfloat gmpd_status_get_current_elapsed(GMpdStatus *self);
gfloat gmpd_status_get_current_duration(GMpdStatus *self);
guint gmpd_status_get_bit_rate(GMpdStatus *self);
guint gmpd_status_get_crossfade(GMpdStatus *self);
gfloat gmpd_status_get_mixramp_db(GMpdStatus *self);
gfloat gmpd_status_get_mixramp_delay(GMpdStatus *self);
GMpdAudioFormat *gmpd_status_get_audio_format(GMpdStatus *self);
guint gmpd_status_get_db_update_job_id(GMpdStatus *self);
gchar *gmpd_status_get_error(GMpdStatus *self);

G_END_DECLS

#endif /* __GMPD_STATUS_H__ */

