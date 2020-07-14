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
#include "gmpd-error.h"
#include "gmpd-response.h"
#include "gmpd-response-priv.h"
#include "gmpd-version.h"

G_DEFINE_INTERFACE(GMpdResponse, gmpd_response, G_TYPE_OBJECT)

static void gmpd_response_default_init(GMpdResponseIface *iface G_GNUC_UNUSED);

static void gmpd_response_feed_pair(GMpdResponse *self,
                                    GMpdVersion *version,
                                    const gchar *key,
                                    const gchar *value);

static void gmpd_response_feed_binary(GMpdResponse *self, GMpdVersion *version, GBytes *binary);
static gsize gmpd_response_get_remaining_binary(GMpdResponse *self);

static gboolean deserialize_binary(GMpdResponse *self,
                                   GMpdVersion *version,
                                   GInputStream *input_stream,
                                   GCancellable *cancellable,
                                   GError **error);

static void
gmpd_response_default_feed_pair(GMpdResponse *self,
                                GMpdVersion *version G_GNUC_UNUSED,
                                const gchar *key G_GNUC_UNUSED,
                                const gchar *value G_GNUC_UNUSED)
{
	g_critical("%s does not implement GMpdResponse::feed_pair()", G_OBJECT_TYPE_NAME(self));
}

static void
gmpd_response_default_feed_binary(GMpdResponse *self,
                                  GMpdVersion *version G_GNUC_UNUSED,
                                  GBytes *binary G_GNUC_UNUSED)
{
	g_critical("%s does not implement GMpdResponse::feed_binary()", G_OBJECT_TYPE_NAME(self));
}

static gsize
gmpd_response_default_get_remaining_binary(GMpdResponse *self G_GNUC_UNUSED)
{
	return 0;
}

static void
gmpd_response_default_init(GMpdResponseIface *iface)
{
	iface->feed_pair = gmpd_response_default_feed_pair;
	iface->feed_binary = gmpd_response_default_feed_binary;
	iface->get_remaining_binary = gmpd_response_default_get_remaining_binary;
}

static void
gmpd_response_feed_pair(GMpdResponse *self,
                        GMpdVersion *version,
                        const gchar *key,
                        const gchar *value)
{
	GMpdResponseIface *iface;

	g_return_if_fail(GMPD_IS_RESPONSE(self));
	g_return_if_fail(GMPD_IS_VERSION(version));
	g_return_if_fail(key != NULL);
	g_return_if_fail(value != NULL);

	iface = GMPD_RESPONSE_GET_IFACE(self);

	g_return_if_fail(iface->feed_pair != NULL);

	iface->feed_pair(self, version, key, value);
}

static void
gmpd_response_feed_binary(GMpdResponse *self, GMpdVersion *version, GBytes *binary)
{
	GMpdResponseIface *iface;

	g_return_if_fail(GMPD_IS_RESPONSE(self));
	g_return_if_fail(GMPD_IS_VERSION(version));
	g_return_if_fail(binary != NULL);

	iface = GMPD_RESPONSE_GET_IFACE(self);

	g_return_if_fail(iface->feed_binary != NULL);

	iface->feed_binary(self, version, binary);
}

static gsize
gmpd_response_get_remaining_binary(GMpdResponse *self)
{
	GMpdResponseIface *iface;

	g_return_val_if_fail(GMPD_IS_RESPONSE(self), 0);

	iface = GMPD_RESPONSE_GET_IFACE(self);

	g_return_val_if_fail(iface->get_remaining_binary != NULL, 0);

	return iface->get_remaining_binary(self);
}

static gboolean
deserialize_binary(GMpdResponse *self,
                   GMpdVersion *version,
                   GInputStream *input_stream,
                   GCancellable *cancellable,
                   GError **error)
{
	gsize remaining;

	g_return_val_if_fail(GMPD_IS_RESPONSE(self), FALSE);
	g_return_val_if_fail(GMPD_IS_VERSION(version), FALSE);
	g_return_val_if_fail(G_IS_INPUT_STREAM(input_stream), FALSE);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	remaining = gmpd_response_get_remaining_binary(self);
	while (remaining) {
		GBytes *bytes = g_input_stream_read_bytes(input_stream,
		                                          remaining,
		                                          cancellable,
		                                          error);
		if (!bytes)
			return FALSE;

		gmpd_response_feed_binary(self, version, bytes);
		g_bytes_unref(bytes);

		remaining = gmpd_response_get_remaining_binary(self);
	}

	return TRUE;
}

gboolean
gmpd_response_deserialize(GMpdResponse *self,
                          GMpdVersion *version,
                          GDataInputStream *data_stream,
                          GCancellable *cancellable,
                          GError **error)
{
	g_return_val_if_fail(GMPD_IS_RESPONSE(self), FALSE);
	g_return_val_if_fail(GMPD_IS_VERSION(version), FALSE);
	g_return_val_if_fail(G_IS_DATA_INPUT_STREAM(data_stream), FALSE);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	while (TRUE) {
		gboolean result;
		gchar *line;
		gchar **parts;

		/* On the start of the first loop, we may be reentering the
		 * function after a G_IO_ERROR_WOULD_BLOCK. On all subsequent
		 * loops, the pair received in the prior loop may have had
		 * a key of 'binary', In either case, we need to receive
		 * the binary data before continuing reading utf8 encoded data.
		 */
		result = deserialize_binary(self,
		                            version,
		                            G_INPUT_STREAM(data_stream),
		                            cancellable,
		                            error);
		if (!result)
			return FALSE;

		line = g_data_input_stream_read_line_utf8(data_stream, NULL, cancellable, error);

		if (!line)
			return FALSE;

		/* check if the command has complete successfully */
		if (!g_strcmp0(line, "OK") || !g_strcmp0(line, "list_OK")) {
			g_free(line);
			return TRUE;
		}

		/* check if the server has returned an ack error */
		if (g_str_has_prefix(line, "ACK")) {
			if (error)
				*error = gmpd_error_from_string(line);

			g_free(line);
			return FALSE;
		}

		/* line should be a pair */
		parts = g_strsplit(line, ": ", 2);
		if (!parts || g_strv_length(parts) != 2) {
			g_set_error(error,
			            G_IO_ERROR,
			            G_IO_ERROR_INVALID_DATA,
			            "invalid pair: %s",
			            line);

			g_strfreev(parts);
			g_free(line);

			return FALSE;
		}

		gmpd_response_feed_pair(self, version, parts[0], parts[1]);

		g_strfreev(parts);
		g_free(line);
	}

	g_return_val_if_reached(FALSE);
}

