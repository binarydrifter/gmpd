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
#include "gmpd-protocol.h"
#include "gmpd-response.h"
#include "gmpd-song.h"
#include "gmpd-status.h"

static GMpdTaskData *
gmpd_task_data_new(gchar *command, GMpdResponse *response)
{
	GMpdTaskData *self = g_slice_new(GMpdTaskData);

	self->ref_count = 1;
	self->command = command;
	self->response = response;

	return self;
}

GMpdTaskData *
gmpd_task_data_ref(GMpdTaskData *self)
{
	g_return_val_if_fail(self != NULL, NULL);
	g_return_val_if_fail(self->ref_count > 0, NULL);

	g_atomic_int_inc(&self->ref_count);

	return self;
}

void
gmpd_task_data_unref(GMpdTaskData *self)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(self->ref_count > 0);

	if (g_atomic_int_dec_and_test(&self->ref_count)) {
		g_clear_pointer(&self->command, g_free);
		g_clear_object(&self->response);

		g_slice_free(GMpdTaskData, self);
	}
}

GMpdTaskData *
gmpd_protocol_currentsong(void)
{
	return gmpd_task_data_new(g_strdup("currentsong\n"), GMPD_RESPONSE(gmpd_song_new()));
}

GMpdTaskData *
gmpd_protocol_status(void)
{
	return gmpd_task_data_new(g_strdup("status\n"), GMPD_RESPONSE(gmpd_status_new()));
}

GMpdTaskData *
gmpd_protocol_close(void)
{
	return gmpd_task_data_new(g_strdup("close\n"), NULL);
}

