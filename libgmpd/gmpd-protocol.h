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

#ifndef __GMPD_PROTOCOL_H__
#define __GMPD_PROTOCOL_H__

#if !defined(__GMPD_BUILD__)
#   error "This file is private to libgmpd and should not be included."
#endif

#include <gio/gio.h>
#include <gmpd-idle.h>
#include <gmpd-response.h>
#include <gmpd-version.h>

G_BEGIN_DECLS

typedef struct _GMpdTaskData {
	volatile gint ref_count;
	gchar *command;
	GMpdResponse *response;
} GMpdTaskData;

GMpdTaskData *gmpd_task_data_ref(GMpdTaskData *self);
void gmpd_task_data_unref(GMpdTaskData *self);

GMpdTaskData *gmpd_protocol_currentsong(void);
GMpdTaskData *gmpd_protocol_idle(GMpdIdle subsystems);
GMpdTaskData *gmpd_protocol_status(void);
GMpdTaskData *gmpd_protocol_stats(void);
GMpdTaskData *gmpd_protocol_close(void);

G_END_DECLS

#endif /* __GMPD_PROTOCOL_H__ */

