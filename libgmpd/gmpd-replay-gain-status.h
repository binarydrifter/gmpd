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

#ifndef __GMPD_REPLAY_GAIN_STATUS_H__
#define __GMPD_REPLAY_GAIN_STATUS_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gmpd-replay-gain-mode.h>

G_BEGIN_DECLS

#define GMPD_TYPE_REPLAY_GAIN_STATUS \
	(gmpd_replay_gain_status_get_type())

#define GMPD_REPLAY_GAIN_STATUS(inst) \
	(G_TYPE_CHECK_INSTANCE_CAST((inst), GMPD_TYPE_REPLAY_GAIN_STATUS, GMpdReplayGainStatus))

#define GMPD_REPLAY_GAIN_STATUS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GMPD_TYPE_REPLAY_GAIN_STATUS, GMpdReplayGainStatusClass))

#define GMPD_IS_REPLAY_GAIN_STATUS(inst) \
	(G_TYPE_CHECK_INSTANCE_TYPE((inst), GMPD_TYPE_REPLAY_GAIN_STATUS))

#define GMPD_IS_REPLAY_GAIN_STATUS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GMPD_TYPE_REPLAY_GAIN_STATUS))

#define GMPD_REPLAY_GAIN_STATUS_GET_CLASS(inst) \
	(G_TYPE_INSTANCE_GET_CLASS((inst), GMPD_TYPE_REPLAY_GAIN_STATUS, GMpdReplayGainStatusClass))

typedef struct _GMpdReplayGainStatus      GMpdReplayGainStatus;
typedef struct _GMpdReplayGainStatusClass GMpdReplayGainStatusClass;

GType                  gmpd_replay_gain_status_get_type (void);
GMpdReplayGainStatus * gmpd_replay_gain_status_new      (void);

void                   gmpd_replay_gain_status_set_mode (GMpdReplayGainStatus *self,
                                                         GMpdReplayGainMode    mode);

GMpdReplayGainMode     gmpd_replay_gain_status_get_mode (GMpdReplayGainStatus *self);

G_END_DECLS

#endif /* __GMPD_REPLAY_GAIN_STATUS_H__ */

