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
#include "gmpd-tag.h"

static const gchar *const GMPD_TAG_NAMES[] =
	{ "Artist"
	, "ArtistSort"
	, "Album"
	, "AlbumSort"
	, "AlbumArtist"
	, "AlbumArtistSort"
	, "Title"
	, "Track"
	, "Name"
	, "Genre"
	, "Date"
	, "OriginalDate"
	, "Composer"
	, "Performer"
	, "Conductor"
	, "Work"
	, "Grouping"
	, "Comment"
	, "Disc"
	, "Label"
	, "MUSICBRAINZ_ARTISTID"
	, "MUSICBRAINZ_ALBUMID"
	, "MUSICBRAINZ_ALBUMARTISTID"
	, "MUSICBRAINZ_TRACKID"
	, "MUSICBRAINZ_RELEASETRACKID"
	, "MUSICBRAINZ_WORKID"
	, NULL
	};

G_STATIC_ASSERT(G_N_ELEMENTS(GMPD_TAG_NAMES) - 1 == GMPD_N_TAGS);

static const GEnumValue GMPD_TAG_VALUES[] = {
	{ GMPD_TAG_UNKNOWN
	, "GMPD_TAG_UNKNOWN"
	, "gmpd-tag-unknown" },

	{ GMPD_TAG_ARTIST
	,  "GMPD_TAG_ARTIST"
	,  "gmpd-tag-artist" },

	{ GMPD_TAG_ARTIST_SORT
	, "GMPD_TAG_ARTIST_SORT"
	, "gmpd-tag-artist-sort" },

	{ GMPD_TAG_ALBUM
	, "GMPD_TAG_ALBUM"
	, "gmpd-tag-album" },

	{ GMPD_TAG_ALBUM_SORT
	, "GMPD_TAG_ALBUM_SORT"
	, "gmpd-tag-album-sort" },

	{ GMPD_TAG_ALBUM_ARTIST
	, "GMPD_TAG_ALBUM_ARTIST"
	, "gmpd-tag-album-artist" },

	{ GMPD_TAG_ALBUM_ARTIST_SORT
	, "GMPD_TAG_ALBUM_ARTIST_SORT"
	, "gmpd-tag-album-artist-sort" },

	{ GMPD_TAG_TITLE
	, "GMPD_TAG_TITLE"
	, "gmpd-tag-title" },

	{ GMPD_TAG_TRACK
	, "GMPD_TAG_TRACK"
	, "gmpd-tag-track" },

	{ GMPD_TAG_NAME
	, "GMPD_TAG_NAME"
	, "gmpd-tag-name" },

	{ GMPD_TAG_GENRE
	, "GMPD_TAG_GENRE"
	, "gmpd-tag-genre" },

	{ GMPD_TAG_DATE
	, "GMPD_TAG_DATE"
	, "gmpd-tag-date" },

	{ GMPD_TAG_ORIGINAL_DATE
	, "GMPD_TAG_ORIGINAL_DATE"
	, "gmpd-tag-original-date" },

	{ GMPD_TAG_COMPOSER
	, "GMPD_TAG_COMPOSER"
	, "gmpd-tag-composer" },

	{ GMPD_TAG_PERFORMER
	, "GMPD_TAG_PERFORMER"
	, "gmpd-tag-performer" },

	{ GMPD_TAG_CONDUCTOR
	, "GMPD_TAG_CONDUCTOR"
	, "gmpd-tag-conductor" },

	{ GMPD_TAG_WORK
	, "GMPD_TAG_WORK"
	, "gmpd-tag-work" },

	{ GMPD_TAG_GROUPING
	, "GMPD_TAG_GROUPING"
	, "gmpd-tag-grouping" },

	{ GMPD_TAG_COMMENT
	, "GMPD_TAG_COMMENT"
	, "gmpd-tag-comment" },

	{ GMPD_TAG_DISC
	, "GMPD_TAG_DISC"
	, "gmpd-tag-disc" },

	{ GMPD_TAG_LABEL
	, "GMPD_TAG_LABEL"
	, "gmpd-tag-label" },

	{ GMPD_TAG_MUSICBRAINZ_ARTIST_ID
	, "GMPD_TAG_MUSICBRAINZ_ARTIST_ID"
	, "gmpd-tag-musicbrainz-artist-id" },

	{ GMPD_TAG_MUSICBRAINZ_ALBUM_ID
	, "GMPD_TAG_MUSICBRAINZ_ALBUM_ID"
	, "gmpd-tag-musicbrainz-album-id" },

	{ GMPD_TAG_MUSICBRAINZ_ALBUM_ARTIST_ID
	, "GMPD_TAG_MUSICBRAINZ_ALBUM_ARTIST_ID"
	, "gmpd-tag-musicbrainz-album-artist-id" },

	{ GMPD_TAG_MUSICBRAINZ_TRACK_ID
	, "GMPD_TAG_MUSICBRAINZ_TRACK_ID"
	, "gmpd-tag-musicbrainz-track-id" },

	{ GMPD_TAG_MUSICBRAINZ_RELEASE_TRACK_ID
	, "GMPD_TAG_MUSICBRAINZ_RELEASE_TRACK_ID"
	, "gmpd-tag-musicbrainz-release-track-id" },

	{ GMPD_TAG_MUSICBRAINZ_WORK_ID
	, "GMPD_TAG_MUSICBRAINZ_WORK_ID"
	, "gmpd_tag_musicbrainz_work_id" },

	{ 0, NULL, NULL },
};

GType
gmpd_tag_get_type(void)
{
	static volatile gsize initialized = 0;
	static GType gtype = 0;

	if (g_once_init_enter(&initialized)) {
		gtype = g_enum_register_static("GMpdTag", GMPD_TAG_VALUES);
		g_once_init_leave(&initialized, 1);
	}

	return gtype;
}

GMpdTag
gmpd_tag_from_string(const gchar *s)
{
	gsize i;

	g_return_val_if_fail(s != NULL, GMPD_TAG_UNKNOWN);

	for (i = 0; i < GMPD_N_TAGS; i++) {
		if (g_strcmp0(GMPD_TAG_NAMES[i], s) == 0)
			return (GMpdTag) i;
	}

	return GMPD_TAG_UNKNOWN;
}

gchar *
gmpd_tag_to_string(GMpdTag tag)
{
	g_return_val_if_fail(GMPD_TAG_IS_VALID(tag), NULL);
	return g_strdup(GMPD_TAG_NAMES[tag]);
}

GQuark
gmpd_tag_to_quark(GMpdTag tag)
{
	g_return_val_if_fail(GMPD_TAG_IS_VALID(tag), 0);
	return g_quark_from_static_string(GMPD_TAG_NAMES[tag]);
}

