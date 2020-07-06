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

#ifndef __GMPD_SONG_H__
#define __GMPD_SONG_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <gio/gio.h>

#include <gmpd-audio-format.h>
#include <gmpd-entity.h>
#include <gmpd-tag.h>

G_BEGIN_DECLS

#define GMPD_TYPE_SONG \
	(gmpd_song_get_type())

#define GMPD_SONG(inst) \
	(G_TYPE_CHECK_INSTANCE_CAST((inst), GMPD_TYPE_SONG, GMpdSong))

#define GMPD_SONG_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GMPD_TYPE_SONG, GMpdSongClass))

#define GMPD_IS_SONG(inst) \
	(G_TYPE_CHECK_INSTANCE_TYPE((inst), GMPD_TYPE_SONG))

#define GMPD_IS_SONG_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GMPD_TYPE_SONG))

#define GMPD_SONG_GET_CLASS(inst) \
	(G_TYPE_INSTANCE_GET_CLASS((inst), GMPD_TYPE_SONG, GMpdSongClass))

typedef struct _GMpdSong GMpdSong;
typedef struct _GMpdSongClass GMpdSongClass;

GType gmpd_song_get_type(void);

GMpdSong *gmpd_song_new(void);

void gmpd_song_set_position(GMpdSong *self, guint position);
void gmpd_song_set_id(GMpdSong *self, guint id);
void gmpd_song_set_priority(GMpdSong *self, guint8 priority);
void gmpd_song_set_duration(GMpdSong *self, float duration);
void gmpd_song_set_range_start(GMpdSong *self, float range_start);
void gmpd_song_set_range_end(GMpdSong *self, float range_end);
void gmpd_song_set_format(GMpdSong *self, GMpdAudioFormat *afmt);
void gmpd_song_set_tag(GMpdSong *self, GMpdTag tag, const gchar *const *values);

guint gmpd_song_get_position(GMpdSong *self);
guint gmpd_song_get_id(GMpdSong *self);
guint8 gmpd_song_get_priority(GMpdSong *self);
float gmpd_song_get_duration(GMpdSong *self);
float gmpd_song_get_range_start(GMpdSong *self);
float gmpd_song_get_range_end(GMpdSong *self);
GMpdAudioFormat *gmpd_song_get_format(GMpdSong *self);
gchar **gmpd_song_get_tag(GMpdSong *self, GMpdTag tag);

G_END_DECLS

#endif /* __GMPD_SONG_H__ */

