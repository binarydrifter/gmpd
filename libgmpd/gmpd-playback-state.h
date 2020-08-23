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

#ifndef __GMPD_PLAYBACK_STATE_H__
#define __GMPD_PLAYBACK_STATE_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

#define GMPD_TYPE_PLAYBACK_STATE \
	(gmpd_playback_state_get_type())

#define GMPD_IS_PLAYBACK_STATE(state) \
	((state) >= GMPD_PLAYBACK_UNKNOWN || (state) <= GMPD_PLAYBACK_PLAYING)

typedef enum _GMpdPlaybackState {
	GMPD_PLAYBACK_UNKNOWN = -1,
	GMPD_PLAYBACK_STOPPED,
	GMPD_PLAYBACK_PAUSED,
	GMPD_PLAYBACK_PLAYING,
} GMpdPlaybackState;

GType              gmpd_playback_state_get_type     (void);

GMpdPlaybackState  gmpd_playback_state_from_string  (const gchar      *s);
gchar *            gmpd_playback_state_to_string    (GMpdPlaybackState state);

G_END_DECLS

#endif /* __GMPD_PLAYBACK_STATE_H__ */

