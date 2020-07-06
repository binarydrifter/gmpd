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

#ifndef __GMPD_TAG_H__
#define __GMPD_TAG_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define GMPD_TYPE_TAG \
	(gmpd_tag_get_type())

#define GMPD_TAG_IS_VALID(tag) \
	((tag) > GMPD_TAG_UNKNOWN && (tag) < GMPD_N_TAGS)

typedef enum _GMpdTag {
	GMPD_TAG_UNKNOWN = -1,
	GMPD_TAG_ARTIST,
	GMPD_TAG_ARTIST_SORT,
	GMPD_TAG_ALBUM,
	GMPD_TAG_ALBUM_SORT,
	GMPD_TAG_ALBUM_ARTIST,
	GMPD_TAG_ALBUM_ARTIST_SORT,
	GMPD_TAG_TITLE,
	GMPD_TAG_TRACK,
	GMPD_TAG_NAME,
	GMPD_TAG_GENRE,
	GMPD_TAG_DATE,
	GMPD_TAG_ORIGINAL_DATE,
	GMPD_TAG_COMPOSER,
	GMPD_TAG_PERFORMER,
	GMPD_TAG_CONDUCTOR,
	GMPD_TAG_WORK,
	GMPD_TAG_GROUPING,
	GMPD_TAG_COMMENT,
	GMPD_TAG_DISC,
	GMPD_TAG_LABEL,
	GMPD_TAG_MUSICBRAINZ_ARTIST_ID,
	GMPD_TAG_MUSICBRAINZ_ALBUM_ID,
	GMPD_TAG_MUSICBRAINZ_ALBUM_ARTIST_ID,
	GMPD_TAG_MUSICBRAINZ_TRACK_ID,
	GMPD_TAG_MUSICBRAINZ_RELEASE_TRACK_ID,
	GMPD_TAG_MUSICBRAINZ_WORK_ID,
	GMPD_N_TAGS,
} GMpdTag;

GType gmpd_tag_get_type(void);

GMpdTag gmpd_tag_from_string(const gchar *s);
gchar *gmpd_tag_to_string(GMpdTag tag);

GQuark gmpd_tag_to_quark(GMpdTag tag);

G_END_DECLS

#endif /* __GMPD_TAG_H__ */

