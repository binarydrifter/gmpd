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

#ifndef __GMPD_STATS_H__
#define __GMPD_STATS_H__

#if !defined(__GMPD_H_INSIDE__) && !defined(__GMPD_BUILD__)
#   error "Only <gmpd.h> can be included directly."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

#define GMPD_TYPE_STATS \
	(gmpd_stats_get_type())

#define GMPD_STATS(inst) \
	(G_TYPE_CHECK_INSTANCE_CAST((inst), GMPD_TYPE_STATS, GMpdStats))

#define GMPD_STATS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GMPD_TYPE_STATS, GMpdStatsClass))

#define GMPD_IS_STATS(inst) \
	(G_TYPE_CHECK_INSTANCE_TYPE((inst), GMPD_TYPE_STATS))

#define GMPD_IS_STATS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GMPD_TYPE_STATS))

#define GMPD_STATS_GET_CLASS(inst) \
	(G_TYPE_INSTANCE_GET_CLASS((inst), GMPD_TYPE_STATS, GMpdStatsClass))

typedef struct _GMpdStats      GMpdStats;
typedef struct _GMpdStatsClass GMpdStatsClass;

GType        gmpd_stats_get_type         (void);

GMpdStats *  gmpd_stats_new              (void);

void         gmpd_stats_set_artists      (GMpdStats *self,
                                          guint      artists);

void         gmpd_stats_set_albums       (GMpdStats *self,
                                          guint      albums);

void         gmpd_stats_set_songs        (GMpdStats *self,
                                          guint      songs);

void         gmpd_stats_set_uptime       (GMpdStats *self,
                                          guint64    uptime);

void         gmpd_stats_set_db_playtime  (GMpdStats *self,
                                          guint64    db_playtime);

void         gmpd_stats_set_db_update    (GMpdStats *self,
                                          GDateTime *db_update);

void         gmpd_stats_set_playtime     (GMpdStats *self,
                                          guint64    playtime);

guint        gmpd_stats_get_artists      (GMpdStats *self);
guint        gmpd_stats_get_albums       (GMpdStats *self);
guint        gmpd_stats_get_songs        (GMpdStats *self);
guint64      gmpd_stats_get_uptime       (GMpdStats *self);
guint64      gmpd_stats_get_db_playtime  (GMpdStats *self);
GDateTime *  gmpd_stats_get_db_update    (GMpdStats *self);
guint64      gmpd_stats_get_playtime     (GMpdStats *self);

G_END_DECLS

#endif /* __GMPD_STATS_H__ */

