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

#ifndef __GMPD_CLIENT_H__
#define __GMPD_CLIENT_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <gio/gio.h>
#include <gmpd-idle.h>
#include <gmpd-replay-gain-mode.h>
#include <gmpd-replay-gain-status.h>
#include <gmpd-song.h>
#include <gmpd-stats.h>
#include <gmpd-status.h>
#include <gmpd-version.h>

G_BEGIN_DECLS

#define GMPD_TYPE_CLIENT \
	(gmpd_client_get_type())

#define GMPD_CLIENT(inst) \
	(G_TYPE_CHECK_INSTANCE_CAST((inst), GMPD_TYPE_CLIENT, GMpdClient))

#define GMPD_CLIENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GMPD_TYPE_CLIENT, GMpdClientClass))

#define GMPD_IS_CLIENT(inst) \
	(G_TYPE_CHECK_INSTANCE_TYPE((inst), GMPD_TYPE_CLIENT))

#define GMPD_IS_CLIENT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GMPD_TYPE_CLIENT))

#define GMPD_CLIENT_GET_CLASS(inst) \
	(G_TYPE_INSTANCE_GET_CLASS((inst), GMPD_TYPE_CLIENT, GMpdClientClass))

typedef struct _GMpdClient      GMpdClient;
typedef struct _GMpdClientClass GMpdClientClass;

GType           gmpd_client_get_type                (void);

GMpdClient *    gmpd_client_connect                 (const gchar         *hostname,
                                                     guint16              port,
                                                     GCancellable        *cancellable,
                                                     GError             **error);

void            gmpd_client_connect_async           (const gchar         *hostname,
                                                     guint16              port,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);

GMpdClient *    gmpd_client_connect_finish          (GAsyncResult        *result,
                                                     GError             **error);

gboolean        gmpd_client_close                   (GMpdClient          *self,
                                                     GCancellable        *cancellable,
                                                     GError             **error);

void            gmpd_client_close_async             (GMpdClient          *self,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);

gboolean        gmpd_client_close_finish            (GMpdClient          *self,
                                                     GAsyncResult        *callback,
                                                     GError             **error);

/*
 * Properties
 */
void            gmpd_client_set_keepalive           (GMpdClient          *self,
                                                     gboolean             keepalive);

void            gmpd_client_set_timeout             (GMpdClient          *self,
                                                     guint                timeout);

GMainContext *  gmpd_client_get_context             (GMpdClient          *self);
gchar *         gmpd_client_get_hostname            (GMpdClient          *self);
guint16         gmpd_client_get_port                (GMpdClient          *self);
gboolean        gmpd_client_get_keepalive           (GMpdClient          *self);
guint           gmpd_client_get_timeout             (GMpdClient          *self);
GMpdVersion *   gmpd_client_get_version             (GMpdClient          *self);


/*
 * Querying MPDs status
 */
gboolean        gmpd_client_clearerror              (GMpdClient          *self,
                                                     GCancellable        *cancellable,
                                                     GError             **error);

void            gmpd_client_clearerror_async        (GMpdClient          *self,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);

GMpdSong *      gmpd_client_currentsong             (GMpdClient          *self,
                                                     GCancellable        *cancellable,
                                                     GError             **error);

void            gmpd_client_currentsong_async       (GMpdClient          *self,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);


GMpdIdle        gmpd_client_idle                    (GMpdClient          *self,
                                                     GMpdIdle             subsystems,
                                                     GCancellable        *cancellable,
                                                     GError             **error);

void            gmpd_client_idle_async              (GMpdClient          *self,
                                                     GMpdIdle             subsystems,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);

GMpdStatus *    gmpd_client_status                  (GMpdClient          *self,
                                                     GCancellable        *cancellable,
                                                     GError             **error);

void            gmpd_client_status_async            (GMpdClient          *self,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);

GMpdStats *     gmpd_client_stats                   (GMpdClient          *self,
                                                     GCancellable        *cancellable,
                                                     GError             **error);

void            gmpd_client_stats_async             (GMpdClient          *self,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);

/*
 * Playback Options
 */

gboolean        gmpd_client_replay_gain_mode        (GMpdClient          *self,
                                                     GMpdReplayGainMode   mode,
                                                     GCancellable        *cancellable,
                                                     GError             **error);

void            gmpd_client_replay_gain_mode_async  (GMpdClient          *self,
                                                     GMpdReplayGainMode   mode,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);

GMpdReplayGainStatus * gmpd_client_replay_gain_status       (GMpdClient          *self,
                                                             GCancellable        *cancellable,
                                                             GError             **error);

void                   gmpd_client_replay_gain_status_async (GMpdClient          *self,
                                                             GCancellable        *cancellable,
                                                             GAsyncReadyCallback  callback,
                                                             gpointer             user_data);

/*
 * Responses
 */
GMpdSong *      gmpd_client_finish_song_response    (GMpdClient          *self,
                                                     GAsyncResult        *result,
                                                     GError             **error);

GMpdIdle        gmpd_client_finish_idle_response    (GMpdClient          *self,
                                                     GAsyncResult        *result,
                                                     GError             **error);

GMpdStatus *    gmpd_client_finish_status_response  (GMpdClient          *self,
                                                     GAsyncResult        *result,
                                                     GError             **error);

GMpdStats *     gmpd_client_finish_stats_response   (GMpdClient          *self,
                                                     GAsyncResult        *result,
                                                     GError             **error);

gboolean        gmpd_client_finish_void_response    (GMpdClient          *self,
                                                     GAsyncResult        *result,
                                                     GError             **error);

GMpdReplayGainStatus * gmpd_client_finish_replay_gain_status_response (GMpdClient    *self,
                                                                       GAsyncResult  *result,
                                                                       GError       **error);

G_END_DECLS

#endif /* __GMPD_CLIENT_H__ */

