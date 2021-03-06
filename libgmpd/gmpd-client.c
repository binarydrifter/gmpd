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
#include <gio/gunixsocketaddress.h>

#include "gmpd-client.h"
#include "gmpd-error.h"
#include "gmpd-idle.h"
#include "gmpd-idle-response.h"
#include "gmpd-object.h"
#include "gmpd-object-priv.h"
#include "gmpd-protocol.h"
#include "gmpd-response.h"
#include "gmpd-song.h"
#include "gmpd-stats.h"
#include "gmpd-status.h"
#include "gmpd-version.h"
#include "gmpd-void-response.h"

#define LOCK(client) G_STMT_START { \
	gmpd_object_lock(GMPD_OBJECT((client))); \
} G_STMT_END

#define UNLOCK(client) G_STMT_START { \
	gmpd_object_unlock(GMPD_OBJECT((client))); \
} G_STMT_END

#define NOTIFY(client, prop_id) G_STMT_START { \
	g_object_notify_by_pspec(G_OBJECT((client)), PROPERTIES[(prop_id)]); \
} G_STMT_END

#define IS_WOULD_BLOCK(err) \
	g_error_matches(err, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)

#define IS_CANCELLED(err) \
	g_error_matches((err), G_IO_ERROR, G_IO_ERROR_CANCELLED)

#define RETURN_TASK(self, task, have_lock) G_STMT_START { \
	gmpd_object_run_in_context(GMPD_OBJECT((self)), \
	                           return_task, \
	                           task, \
	                           g_object_unref, \
	                           (have_lock)); \
} G_STMT_END

static void gmpd_client_initable_iface_init(GInitableIface *iface);
static void gmpd_client_async_initable_iface_init(GAsyncInitable *iface);

static void gmpd_client_do_set_hostname(GMpdClient  *self,
                                        const gchar *hostname,
                                        gboolean     have_lock);

static void gmpd_client_do_set_port(GMpdClient *self,
                                    guint16     port,
                                    gboolean    have_lock);

static void gmpd_client_do_set_keepalive(GMpdClient *self,
                                         gboolean    keepalive,
                                         gboolean    have_lock);

static void gmpd_client_do_set_timeout(GMpdClient *self,
                                       guint       timeout,
                                       gboolean    have_lock);

static void gmpd_client_do_set_version(GMpdClient  *self,
                                       GMpdVersion *version,
                                       gboolean     have_lock);

static void gmpd_client_set_hostname(GMpdClient  *self,
                                     const gchar *hostname);

static void gmpd_client_set_port(GMpdClient *self,
                                 guint16     port);

static void gmpd_client_set_version(GMpdClient  *self,
                                    GMpdVersion *version) G_GNUC_UNUSED;

static void gmpd_client_update_hostname(GMpdClient *self);
static void gmpd_client_update_port(GMpdClient *self);

static gboolean gmpd_client_connect_to_server(GMpdClient   *self,
                                              GCancellable *cancellable,
                                              GError      **error);

static gboolean gmpd_client_receive_version(GMpdClient   *self,
                                            GCancellable *cancellable,
                                            GError      **error);

static void gmpd_client_attach_input_source(GMpdClient *self);
static void gmpd_client_attach_output_source(GMpdClient *self);
static void gmpd_client_destroy_input_source(GMpdClient *self);
static void gmpd_client_destroy_output_source(GMpdClient *self);

static GMpdResponse *gmpd_client_run_task(GMpdClient   *self,
                                          gboolean      have_lock,
                                          GMpdTaskData *data,
                                          GCancellable *cancellable,
                                          GError      **error);

static void gmpd_client_run_task_async(GMpdClient         *self,
                                       gboolean            have_lock,
                                       GMpdTaskData       *data,
                                       GCancellable       *cancellable,
                                       GAsyncReadyCallback callback,
                                       gpointer            user_data);

static GTask *gmpd_client_start_task(GMpdClient         *self,
                                     gboolean            have_lock,
                                     GMpdTaskData       *task_data,
                                     GCancellable       *cancellable,
                                     GAsyncReadyCallback callback,
                                     gpointer            user_data);

static void gmpd_client_do_disconnect(GMpdClient *self);
static void gmpd_client_update_timeout(GMpdClient *self);
static gboolean gmpd_client_is_idle(GMpdClient *self);
static void gmpd_client_disable_timeout(GMpdClient *self);
static void gmpd_client_enable_timeout(GMpdClient *self);

static gboolean gmpd_client_do_flush(GMpdClient   *self,
                                     GCancellable *cancellable,
                                     GError      **error);

static gboolean gmpd_client_do_fill(GMpdClient   *self,
                                    GCancellable *cancellable,
                                    GError      **error);

static gboolean gmpd_client_do_sync(GMpdClient   *self,
                                    GIOCondition  condition,
                                    gboolean      blocking_io,
                                    GCancellable *cancellable,
                                    GError      **error);

static gboolean gmpd_client_on_socket_ready(GSocket     *socket,
                                            GIOCondition condition,
                                            GMpdClient  *self);

static gboolean return_task (gpointer data);

enum {
	PROP_NONE,
	PROP_HOSTNAME,
	PROP_PORT,
	PROP_KEEPALIVE,
	PROP_TIMEOUT,
	PROP_VERSION,
	N_PROPERTIES,
};

struct _GMpdClient {
	GMpdObject             __base__;

	GSocketConnection     *socket_connection;
	GDataInputStream      *input_stream;
	GBufferedOutputStream *output_stream;
	GSource               *input_source;
	GSource               *output_source;

	gchar                 *hostname;
	guint16                port;
	gboolean               keepalive;
	guint                  timeout;
	GMpdVersion           *version;

	GQueue                *task_queue;
};

struct _GMpdClientClass {
	GMpdObjectClass __base__;
};

G_DEFINE_TYPE_WITH_CODE(GMpdClient, gmpd_client, GMPD_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,
                                              gmpd_client_initable_iface_init)
                        G_IMPLEMENT_INTERFACE(G_TYPE_ASYNC_INITABLE,
                                              gmpd_client_async_initable_iface_init))

static GParamSpec *PROPERTIES[N_PROPERTIES] = {NULL};

static gboolean
gmpd_client_initable_init(GInitable    *initable,
                          GCancellable *cancellable,
                          GError      **error)
{
	GMpdClient *self = GMPD_CLIENT(initable);
	gboolean result;

	LOCK(self);

	gmpd_client_update_hostname(self);
	gmpd_client_update_port(self);

	result = gmpd_client_connect_to_server(self, cancellable, error) &&
	         gmpd_client_receive_version(self, cancellable, error);

	if (!result)
		gmpd_client_do_disconnect(self);

	UNLOCK(self);
	return result;
}

static void
gmpd_client_initable_iface_init(GInitableIface *iface)
{
	iface->init = gmpd_client_initable_init;
}

static void
gmpd_client_async_initable_iface_init(GAsyncInitable *iface G_GNUC_UNUSED)
{
	/* use defaults */
}

static void
gmpd_client_set_property(GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
	GMpdClient *self = GMPD_CLIENT(object);

	switch (prop_id) {
	case PROP_HOSTNAME:
		gmpd_client_set_hostname(self, g_value_get_string(value));
		break;

	case PROP_PORT:
		gmpd_client_set_port(self, g_value_get_uint(value));
		break;

	case PROP_KEEPALIVE:
		gmpd_client_set_keepalive(self, g_value_get_boolean(value));
		break;

	case PROP_TIMEOUT:
		gmpd_client_set_timeout(self, g_value_get_uint(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_client_get_property(GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
	GMpdClient *self = GMPD_CLIENT(object);

	switch (prop_id) {
	case PROP_HOSTNAME:
		g_value_take_string(value, gmpd_client_get_hostname(self));
		break;

	case PROP_PORT:
		g_value_set_uint(value, gmpd_client_get_port(self));
		break;

	case PROP_KEEPALIVE:
		g_value_set_boolean(value, gmpd_client_get_keepalive(self));
		break;

	case PROP_TIMEOUT:
		g_value_set_uint(value, gmpd_client_get_timeout(self));
		break;

	case PROP_VERSION:
		g_value_take_object(value, gmpd_client_get_version(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_client_finalize(GObject *object)
{
	GMpdClient *self = GMPD_CLIENT(object);
	GTask *task;

	gmpd_client_destroy_input_source(self);
	gmpd_client_destroy_output_source(self);
	g_clear_object(&self->input_stream);
	g_clear_object(&self->output_stream);
	g_clear_object(&self->socket_connection);

	g_clear_pointer(&self->hostname, g_free);
	g_clear_object(&self->version);

	while ((task = g_queue_pop_head(self->task_queue)))
		g_object_unref(task);

	g_clear_pointer(&self->task_queue, g_queue_free);

	G_OBJECT_CLASS(gmpd_client_parent_class)->finalize(object);
}

static void
gmpd_client_class_init(GMpdClientClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = gmpd_client_set_property;
	object_class->get_property = gmpd_client_get_property;
	object_class->finalize = gmpd_client_finalize;

	PROPERTIES[PROP_HOSTNAME] =
		g_param_spec_string("hostname",
		                    "Hostname",
		                    "Hostname of the MPD server",
		                    NULL,
		                    G_PARAM_READWRITE |
		                    G_PARAM_CONSTRUCT_ONLY |
		                    G_PARAM_EXPLICIT_NOTIFY |
		                    G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_PORT] =
		g_param_spec_uint("port",
		                  "Port",
		                  "Port of the MPD server",
		                  0, G_MAXUINT16, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_CONSTRUCT_ONLY |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_KEEPALIVE] =
		g_param_spec_boolean("keepalive",
		                     "Keepalive",
		                     "Send TCP Keepalive packets",
		                     FALSE,
		                     G_PARAM_READWRITE |
		                     G_PARAM_EXPLICIT_NOTIFY |
		                     G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_TIMEOUT] =
		g_param_spec_uint("timeout",
		                  "Timeout",
		                  "I/O timeout in seconds",
		                  0, G_MAXUINT, 0,
		                  G_PARAM_READWRITE |
		                  G_PARAM_EXPLICIT_NOTIFY |
		                  G_PARAM_STATIC_STRINGS);

	PROPERTIES[PROP_VERSION] =
		g_param_spec_object("version",
		                    "Version",
		                    "Protocol version understood by the MPD server",
		                    GMPD_TYPE_VERSION,
		                    G_PARAM_READABLE |
		                    G_PARAM_EXPLICIT_NOTIFY |
		                    G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPERTIES, PROPERTIES);
}

static void
gmpd_client_init(GMpdClient *self)
{
	self->socket_connection = NULL;
	self->input_stream = NULL;
	self->output_stream = NULL;
	self->input_source = NULL;
	self->output_source = NULL;

	self->hostname = NULL;
	self->port = 0;
	self->keepalive = FALSE;
	self->timeout = 0;
	self->version = NULL;

	self->task_queue = g_queue_new();
}

GMpdClient *
gmpd_client_connect(const gchar  *hostname,
                    guint16       port,
                    GCancellable *cancellable,
                    GError      **error)
{
	GMainContext *context = g_main_context_ref_thread_default();

	GMpdClient *client = g_initable_new(GMPD_TYPE_CLIENT, cancellable, error,
	                                    "context",  context,
	                                    "hostname", hostname,
	                                    "port",     port, NULL);

	if (context)
		g_main_context_unref(context);

	return client;
}

void
gmpd_client_connect_async(const gchar        *hostname,
                          guint16             port,
                          GCancellable       *cancellable,
                          GAsyncReadyCallback callback,
                          gpointer            user_data)
{
	GMainContext *context = g_main_context_ref_thread_default();

	g_async_initable_new_async(GMPD_TYPE_CLIENT, G_PRIORITY_DEFAULT,
	                           cancellable, callback, user_data,
	                           "context",  context,
	                           "hostname", hostname,
	                           "port",     port, NULL);

	if (context)
		g_main_context_unref(context);
}

GMpdClient *
gmpd_client_connect_finish(GAsyncResult *result,
                           GError      **error)
{
	GAsyncInitable *source_object;
	GMpdClient *client;

	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	source_object = G_ASYNC_INITABLE(g_async_result_get_source_object(result));

	client = GMPD_CLIENT(g_async_initable_new_finish(source_object, result, error));

	g_object_unref(source_object);

	return client;
}

void
gmpd_client_set_keepalive(GMpdClient *self,
                          gboolean    keepalive)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	gmpd_client_do_set_keepalive(self, keepalive, FALSE);
}

void
gmpd_client_set_timeout(GMpdClient *self,
                        guint       timeout)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	gmpd_client_do_set_timeout(self, timeout, FALSE);
}

gchar *
gmpd_client_get_hostname(GMpdClient *self)
{
	gchar *hostname;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);

	LOCK(self);

	hostname = g_strdup(self->hostname);

	UNLOCK(self);

	return hostname;
}

guint16
gmpd_client_get_port(GMpdClient *self)
{
	guint16 port;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), 0);

	LOCK(self);

	port = self->port;

	UNLOCK(self);

	return port;
}

gboolean
gmpd_client_get_keepalive(GMpdClient *self)
{
	gboolean keepalive;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), FALSE);

	LOCK(self);

	keepalive = self->keepalive;

	UNLOCK(self);

	return keepalive;
}

guint
gmpd_client_get_timeout(GMpdClient *self)
{
	guint timeout;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), 0);

	LOCK(self);

	timeout = self->timeout;

	UNLOCK(self);

	return timeout;
}

GMpdVersion *
gmpd_client_get_version(GMpdClient *self)
{
	GMpdVersion *version;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);

	LOCK(self);

	version = self->version ? g_object_ref(self->version) : NULL;

	UNLOCK(self);

	return version;
}

gboolean
gmpd_client_close(GMpdClient   *self,
                  GCancellable *cancellable,
                  GError      **error)
{
	GError *err = NULL;
	GMpdResponse *response;
	gboolean retval;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), FALSE);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	response = gmpd_client_run_task(self, FALSE, gmpd_protocol_close(), cancellable, &err);
	g_warn_if_fail(response == NULL);

	if (err) {
		retval = FALSE;
		g_propagate_error(error, err);
	} else {
		retval = TRUE;
	}

	return retval;
}

void
gmpd_client_close_async(GMpdClient         *self,
                        GCancellable       *cancellable,
                        GAsyncReadyCallback callback,
                        gpointer            user_data)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
	g_return_if_fail(callback != NULL || user_data == NULL);

	gmpd_client_run_task_async(self,
	                           FALSE,
	                           gmpd_protocol_close(),
	                           cancellable,
	                           callback,
	                           user_data);
}

gboolean
gmpd_client_close_finish(GMpdClient   *self,
                         GAsyncResult *result,
                         GError      **error)
{
	GTask *task;
	GError *err = NULL;
	GMpdResponse *response;
	gboolean retval;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), FALSE);
	g_return_val_if_fail(G_IS_TASK(result), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	task = G_TASK(result);
	g_return_val_if_fail(g_task_get_source_object(task) == self, FALSE);

	response = g_task_propagate_pointer(task, &err);
	g_warn_if_fail(response == NULL);

	if (err) {
		retval = FALSE;
		g_propagate_error(error, err);
	} else {
		retval = TRUE;
	}

	return retval;
}

gboolean
gmpd_client_clearerror(GMpdClient   *self,
                       GCancellable *cancellable,
                       GError      **error)
{
	GMpdResponse *response;
	GError *err = NULL;
	gboolean retval;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), FALSE);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	response = gmpd_client_run_task(self,
	                                FALSE,
	                                gmpd_protocol_clearerror(),
	                                cancellable,
	                                &err);

	g_clear_object(&response);

	if (err) {
		retval = FALSE;
		g_propagate_error(error, err);
	} else {
		retval = TRUE;
	}

	return retval;
}

void
gmpd_client_clearerror_async(GMpdClient         *self,
                             GCancellable       *cancellable,
                             GAsyncReadyCallback callback,
                             gpointer            user_data)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
	g_return_if_fail(callback != NULL || user_data == NULL);

	gmpd_client_run_task_async(self,
	                           FALSE,
	                           gmpd_protocol_clearerror(),
	                           cancellable,
	                           callback,
	                           user_data);

}

GMpdSong *
gmpd_client_currentsong(GMpdClient   *self,
                        GCancellable *cancellable,
                        GError      **error)
{
	GMpdResponse *response;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	response = gmpd_client_run_task(self,
	                                FALSE,
	                                gmpd_protocol_currentsong(),
	                                cancellable,
	                                error);

	return response ? GMPD_SONG(response) : NULL;
}

void
gmpd_client_currentsong_async(GMpdClient         *self,
                              GCancellable       *cancellable,
                              GAsyncReadyCallback callback,
                              gpointer            user_data)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
	g_return_if_fail(callback != NULL || user_data == NULL);

	gmpd_client_run_task_async(self,
	                           FALSE,
	                           gmpd_protocol_currentsong(),
	                           cancellable,
	                           callback,
	                           user_data);
}

GMpdIdle
gmpd_client_idle(GMpdClient   *self,
                 GMpdIdle      subsystems,
                 GCancellable *cancellable,
                 GError      **error)
{
	GMpdResponse *response;
	GMpdIdle idle;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), GMPD_IDLE_NONE);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), GMPD_IDLE_NONE);
	g_return_val_if_fail(error == NULL || *error == NULL, GMPD_IDLE_NONE);

	response = gmpd_client_run_task(self,
	                                FALSE,
	                                gmpd_protocol_idle(subsystems),
	                                cancellable,
	                                error);

	if (!response)
		return GMPD_IDLE_NONE;

	idle = gmpd_idle_response_get_changed(GMPD_IDLE_RESPONSE(response));

	g_object_unref(response);

	return idle;
}

void
gmpd_client_idle_async(GMpdClient         *self,
                       GMpdIdle            subsystems,
                       GCancellable       *cancellable,
                       GAsyncReadyCallback callback,
                       gpointer            user_data)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
	g_return_if_fail(callback != NULL || user_data == NULL);

	gmpd_client_run_task_async(self,
	                           FALSE,
	                           gmpd_protocol_idle(subsystems),
	                           cancellable,
	                           callback,
	                           user_data);
}

GMpdStatus *
gmpd_client_status(GMpdClient   *self,
                   GCancellable *cancellable,
                   GError      **error)
{
	GMpdResponse *response;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	response = gmpd_client_run_task(self,
	                                FALSE,
	                                gmpd_protocol_status(),
	                                cancellable,
	                                error);

	return response ? GMPD_STATUS(response) : NULL;
}

void
gmpd_client_status_async(GMpdClient         *self,
                         GCancellable       *cancellable,
                         GAsyncReadyCallback callback,
                         gpointer            user_data)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
	g_return_if_fail(callback != NULL || user_data == NULL);

	gmpd_client_run_task_async(self,
	                           FALSE,
	                           gmpd_protocol_status(),
	                           cancellable,
	                           callback,
	                           user_data);
}

GMpdStats *
gmpd_client_stats(GMpdClient   *self,
                  GCancellable *cancellable,
                  GError      **error)
{
	GMpdResponse *response;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	response = gmpd_client_run_task(self,
	                                FALSE,
	                                gmpd_protocol_stats(),
	                                cancellable,
	                                error);

	return response ? GMPD_STATS(response) : NULL;
}

void
gmpd_client_stats_async(GMpdClient         *self,
                        GCancellable       *cancellable,
                        GAsyncReadyCallback callback,
                        gpointer            user_data)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
	g_return_if_fail(callback != NULL || user_data == NULL);

	gmpd_client_run_task_async(self,
	                           FALSE,
	                           gmpd_protocol_stats(),
	                           cancellable,
	                           callback,
	                           user_data);
}

gboolean
gmpd_client_replay_gain_mode(GMpdClient        *self,
                             GMpdReplayGainMode mode,
                             GCancellable      *cancellable,
                             GError           **error)
{
	GMpdResponse *response;
	GError *err = NULL;
	gboolean retval;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), FALSE);
	g_return_val_if_fail(GMPD_IS_REPLAY_GAIN_MODE(mode), FALSE);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	response = gmpd_client_run_task(self,
	                                FALSE,
	                                gmpd_protocol_replay_gain_mode(mode),
	                                cancellable,
	                                &err);

	g_clear_object(&response);

	if (err) {
		retval = FALSE;
		g_propagate_error(error, err);
	} else {
		retval = TRUE;
	}

	return retval;
}

void
gmpd_client_replay_gain_mode_async(GMpdClient         *self,
                                   GMpdReplayGainMode  mode,
                                   GCancellable       *cancellable,
                                   GAsyncReadyCallback callback,
                                   gpointer            user_data)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	g_return_if_fail(GMPD_IS_REPLAY_GAIN_MODE(mode));
	g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
	g_return_if_fail(callback != NULL || user_data == NULL);

	gmpd_client_run_task_async(self,
	                           FALSE,
	                           gmpd_protocol_replay_gain_mode(mode),
	                           cancellable,
	                           callback,
	                           user_data);
}

GMpdReplayGainStatus *
gmpd_client_replay_gain_status(GMpdClient   *self,
                               GCancellable *cancellable,
                               GError      **error)
{
	GMpdResponse *response;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	response = gmpd_client_run_task(self,
	                                FALSE,
	                                gmpd_protocol_replay_gain_status(),
	                                cancellable,
	                                error);

	return response ? GMPD_REPLAY_GAIN_STATUS(response) : NULL;

}

void
gmpd_client_replay_gain_status_async(GMpdClient         *self,
                                     GCancellable       *cancellable,
                                     GAsyncReadyCallback callback,
                                     gpointer            user_data)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));
	g_return_if_fail(callback != NULL || user_data == NULL);

	gmpd_client_run_task_async(self,
	                           FALSE,
	                           gmpd_protocol_replay_gain_status(),
	                           cancellable,
	                           callback,
	                           user_data);

}

GMpdSong *
gmpd_client_finish_song_response(GMpdClient   *self,
                                 GAsyncResult *result,
                                 GError      **error)
{
	GTask *task;
	gpointer retval;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);
	g_return_val_if_fail(G_IS_TASK(result), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	task = G_TASK(result);
	g_return_val_if_fail(g_task_get_source_object(task) == self, NULL);

	retval = g_task_propagate_pointer(task, error);
	g_return_val_if_fail(retval == NULL || GMPD_IS_SONG(retval), NULL);

	return retval ? GMPD_SONG(retval) : NULL;
}

GMpdIdle
gmpd_client_finish_idle_response(GMpdClient   *self,
                                 GAsyncResult *result,
                                 GError      **error)
{
	GTask *task;
	gpointer idle_resp;
	GMpdIdle retval;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), GMPD_IDLE_NONE);
	g_return_val_if_fail(G_IS_TASK(result), GMPD_IDLE_NONE);
	g_return_val_if_fail(error == NULL || *error == NULL, GMPD_IDLE_NONE);

	task = G_TASK(result);
	g_return_val_if_fail(g_task_get_source_object(task) == self, GMPD_IDLE_NONE);

	idle_resp = g_task_propagate_pointer(task, error);
	g_return_val_if_fail(idle_resp == NULL || GMPD_IS_IDLE_RESPONSE(idle_resp), GMPD_IDLE_NONE);

	if (!idle_resp)
		return GMPD_IDLE_NONE;

	retval = gmpd_idle_response_get_changed(GMPD_IDLE_RESPONSE(idle_resp));

	g_object_unref(idle_resp);

	return retval;
}

GMpdStatus *
gmpd_client_finish_status_response(GMpdClient   *self,
                                   GAsyncResult *result,
                                   GError      **error)
{
	GTask *task;
	gpointer retval;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);
	g_return_val_if_fail(G_IS_TASK(result), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	task = G_TASK(result);
	g_return_val_if_fail(g_task_get_source_object(task) == self, NULL);

	retval = g_task_propagate_pointer(task, error);
	g_return_val_if_fail(retval == NULL || GMPD_IS_STATUS(retval), NULL);

	return retval ? GMPD_STATUS(retval) : NULL;
}

GMpdStats *
gmpd_client_finish_stats_response(GMpdClient   *self,
                                  GAsyncResult *result,
                                  GError      **error)
{
	GTask *task;
	gpointer retval;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);
	g_return_val_if_fail(G_IS_TASK(result), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	task = G_TASK(result);
	g_return_val_if_fail(g_task_get_source_object(task) == self, NULL);

	retval = g_task_propagate_pointer(task, error);
	g_return_val_if_fail(retval == NULL || GMPD_IS_STATS(retval), NULL);

	return retval ? GMPD_STATS(retval) : NULL;
}

gboolean
gmpd_client_finish_void_response(GMpdClient   *self,
                                 GAsyncResult *result,
                                 GError      **error)
{
	GTask *task;
	GMpdVoidResponse *response;
	GError *err = NULL;
	gboolean retval;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), FALSE);
	g_return_val_if_fail(G_IS_TASK(result), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	task = G_TASK(self);
	g_return_val_if_fail(g_task_get_source_object(task) == self, FALSE);

	response = g_task_propagate_pointer(task, &err);
	g_return_val_if_fail(response == NULL || GMPD_IS_VOID_RESPONSE(response), FALSE);

	g_clear_object(&response);

	if (err) {
		retval = FALSE;
		g_propagate_error(error, err);
	} else {
		retval = TRUE;
	}

	return retval;
}

GMpdReplayGainStatus *
gmpd_client_finish_replay_gain_status_response(GMpdClient   *self,
                                               GAsyncResult *result,
                                               GError      **error)
{
	GTask *task;
	GMpdReplayGainStatus *response;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);
	g_return_val_if_fail(G_IS_TASK(result), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	task = G_TASK(result);
	g_return_val_if_fail(g_task_get_source_object(task) == self, FALSE);

	response = g_task_propagate_pointer(task, error);
	g_return_val_if_fail(response == NULL || GMPD_IS_REPLAY_GAIN_STATUS(response), NULL);

	return response;
}

static void
gmpd_client_do_set_hostname(GMpdClient  *self,
                            const gchar *hostname,
                            gboolean     have_lock)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (!have_lock)
		LOCK(self);

	g_free(self->hostname);
	self->hostname = g_strdup(hostname);
	NOTIFY(self, PROP_HOSTNAME);

	if (!have_lock)
		UNLOCK(self);
}

static void
gmpd_client_do_set_port(GMpdClient *self,
                        guint16     port,
                        gboolean    have_lock)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (!have_lock)
		LOCK(self);

	if (self->port != port) {
		self->port = port;
		NOTIFY(self, PROP_PORT);
	}

	if (!have_lock)
		UNLOCK(self);
}

static void
gmpd_client_do_set_keepalive(GMpdClient *self,
                             gboolean    keepalive,
                             gboolean    have_lock)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (!have_lock)
		LOCK(self);

	if (self->keepalive != keepalive) {
		self->keepalive = keepalive;

		if (self->socket_connection) {
			GSocket *socket = g_socket_connection_get_socket(self->socket_connection);
			g_socket_set_keepalive(socket, self->keepalive);
		}

		NOTIFY(self, PROP_KEEPALIVE);
	}

	if (!have_lock)
		UNLOCK(self);
}

static void
gmpd_client_do_set_timeout(GMpdClient *self,
                           guint       timeout,
                           gboolean    have_lock)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (!have_lock)
		LOCK(self);

	if (self->timeout != timeout) {
		self->timeout = timeout;

		if (self->socket_connection) {
			GSocket *socket;
			gint64 ready_time;

			socket = g_socket_connection_get_socket(self->socket_connection);
			g_socket_set_timeout(socket, self->timeout);

			ready_time = g_get_monotonic_time() + (self->timeout * 1000000);

			if (self->input_source)
				g_source_set_ready_time(self->input_source, ready_time);

			if (self->output_source)
				g_source_set_ready_time(self->output_source, ready_time);
		}

		NOTIFY(self, PROP_TIMEOUT);
	}

	if (!have_lock)
		UNLOCK(self);
}

static void
gmpd_client_do_set_version(GMpdClient  *self,
                           GMpdVersion *version,
                           gboolean     have_lock)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	g_return_if_fail(version == NULL || GMPD_IS_VERSION(version));

	if (!have_lock)
		LOCK(self);

	if (self->version != version) {
		g_clear_object(&self->version);
		self->version = version ? g_object_ref(version) : NULL;
		NOTIFY(self, PROP_VERSION);
	}

	if (!have_lock)
		UNLOCK(self);
}

static void
gmpd_client_set_hostname(GMpdClient  *self,
                         const gchar *hostname)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	gmpd_client_do_set_hostname(self, hostname, FALSE);
}

static void
gmpd_client_set_port(GMpdClient *self,
                     guint16     port)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	gmpd_client_do_set_port(self, port, FALSE);
}

static void
gmpd_client_set_version(GMpdClient  *self,
                        GMpdVersion *version)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));
	g_return_if_fail(version == NULL || GMPD_IS_VERSION(version));
	gmpd_client_do_set_version(self, version, FALSE);
}

static void
gmpd_client_update_hostname(GMpdClient *self)
{
	const gchar *hostname;

	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (!self->hostname) {
		hostname = g_getenv("MPD_HOST");

		if (!hostname)
			hostname = "localhost";

		gmpd_client_do_set_hostname(self, hostname, TRUE);
	}
}

static void
gmpd_client_update_port(GMpdClient *self)
{
	const gchar *port_str;
	guint16 port;

	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (!self->port && self->hostname[0] != '/') {
		port_str = g_getenv("MPD_PORT");
		port = 0;

		if (port_str)
			port = g_ascii_strtoull(port_str, NULL, 10);

		if (!port)
			port = 6600;

		gmpd_client_do_set_port(self, port, TRUE);
	}
}

static gboolean
gmpd_client_connect_to_server(GMpdClient   *self,
                              GCancellable *cancellable,
                              GError      **error)
{
	GSocketConnectable *socket_connectable;
	GSocketClient *socket_client;
	GError *err = NULL;
	GInputStream *input_stream;
	GOutputStream *output_stream;
	GSocket *socket;

	if (self->hostname[0] == '/')
		socket_connectable = G_SOCKET_CONNECTABLE(g_unix_socket_address_new(self->hostname));
	else
		socket_connectable = g_network_address_new(self->hostname, self->port);

	socket_client = g_socket_client_new();
	self->socket_connection = g_socket_client_connect(socket_client,
	                                                  socket_connectable,
	                                                  cancellable,
	                                                  &err);

	g_object_unref(socket_connectable);
	g_object_unref(socket_client);

	if (!self->socket_connection) {
		g_propagate_error(error, err);
		return FALSE;
	}

	input_stream = g_io_stream_get_input_stream(G_IO_STREAM(self->socket_connection));
	output_stream = g_io_stream_get_output_stream(G_IO_STREAM(self->socket_connection));

	self->input_stream = g_data_input_stream_new(input_stream);
	self->output_stream = G_BUFFERED_OUTPUT_STREAM(g_buffered_output_stream_new(output_stream));

	socket = g_socket_connection_get_socket(self->socket_connection);
	g_socket_set_blocking(socket, TRUE);
	g_socket_set_keepalive(socket, self->keepalive);
	g_socket_set_timeout(socket, self->timeout);

	g_buffered_output_stream_set_auto_grow(self->output_stream, TRUE);

	return TRUE;
}

static gboolean
gmpd_client_receive_version(GMpdClient   *self,
                            GCancellable *cancellable,
                            GError      **error)
{
	GError *err = NULL;
	gchar *line;
	GMpdVersion *version;

	line = g_data_input_stream_read_line_utf8(self->input_stream, NULL, cancellable, &err);
	if (!line) {
		g_propagate_error(error, err);
		return FALSE;
	}

	version = gmpd_version_new_from_string(line);
	if (!version) {
		g_set_error(error,
		            G_IO_ERROR,
		            G_IO_ERROR_INVALID_DATA,
		            "invalid welcome string: %s", line);

		g_free(line);
		return FALSE;
	}

	gmpd_client_do_set_version(self, version, TRUE);

	g_free(line);
	g_object_unref(version);

	return TRUE;
}

static void
gmpd_client_attach_input_source(GMpdClient *self)
{
	GSocket *socket;

	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (self->input_source || !self->socket_connection || !GMPD_OBJECT(self)->context)
		return;

	socket = g_socket_connection_get_socket(self->socket_connection);
	self->input_source = g_socket_create_source(socket, G_IO_IN, NULL);

	g_source_set_callback(self->input_source,
	                      G_SOURCE_FUNC(gmpd_client_on_socket_ready),
	                      g_object_ref(self),
	                      g_object_unref);

	g_source_attach(self->input_source, GMPD_OBJECT(self)->context);
}

static void
gmpd_client_attach_output_source(GMpdClient *self)
{
	GSocket *socket;

	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (self->output_source || !self->socket_connection || !GMPD_OBJECT(self)->context)
		return;

	socket = g_socket_connection_get_socket(self->socket_connection);
	self->output_source = g_socket_create_source(socket, G_IO_OUT, NULL);

	g_source_set_callback(self->output_source,
	                      G_SOURCE_FUNC(gmpd_client_on_socket_ready),
	                      g_object_ref(self),
	                      g_object_unref);

	g_source_attach(self->output_source, GMPD_OBJECT(self)->context);
}

static void
gmpd_client_destroy_input_source(GMpdClient *self)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (self->input_source) {
		g_source_destroy(self->input_source);
		g_clear_pointer(&self->input_source, g_source_unref);
	}
}

static void
gmpd_client_destroy_output_source(GMpdClient *self)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (self->output_source) {
		g_source_destroy(self->output_source);
		g_clear_pointer(&self->output_source, g_source_unref);
	}
}

static GMpdResponse *
gmpd_client_run_task(GMpdClient   *self,
                     gboolean      have_lock,
                     GMpdTaskData *data,
                     GCancellable *cancellable,
                     GError      **error)
{
	GTask *task;
	gboolean result;
	GMpdResponse *response;

	if (!have_lock)
		LOCK(self);

	task = gmpd_client_start_task(self, TRUE, data, cancellable, NULL, NULL);

	result = gmpd_client_do_sync(self, G_IO_IN | G_IO_OUT, TRUE, cancellable, error);
	if (!result) {
		g_object_unref(task);

		if (!have_lock)
			UNLOCK(self);

		return NULL;
	}

	if (data->error) {
		if (error)
			*error = g_error_copy(data->error);

		response = NULL;
	} else {
		response = data->response ? g_object_ref(data->response) : NULL;
	}

	g_object_unref(task);

	if (!have_lock)
		UNLOCK(self);

	return response;
}

static void
gmpd_client_run_task_async(GMpdClient         *self,
                           gboolean            have_lock,
                           GMpdTaskData       *data,
                           GCancellable       *cancellable,
                           GAsyncReadyCallback callback,
                           gpointer            user_data)
{
	GTask *task = gmpd_client_start_task(self,
	                                     have_lock,
	                                     data,
	                                     cancellable,
	                                     callback,
	                                     user_data);

	g_object_unref(task);
}

static void
gmpd_client_noidle(GMpdClient *self)
{
	GTask *tail_task;
	GMpdTaskData *tail_data;

	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (!self->socket_connection)
		return;

	tail_task = g_queue_peek_tail(self->task_queue);
	tail_data = tail_task ? g_task_get_task_data(tail_task) : NULL;

	if (tail_data && GMPD_IS_IDLE_RESPONSE(tail_data->response)) {
		static const gchar noidle_command[] = "noidle\n";
		static const gsize noidle_len = G_N_ELEMENTS(noidle_command) - 1;

		g_output_stream_write(G_OUTPUT_STREAM(self->output_stream),
		                      noidle_command,
		                      noidle_len,
		                      NULL,
		                      NULL);
	}
}

static GTask *
gmpd_client_start_task(GMpdClient         *self,
                       gboolean            have_lock,
                       GMpdTaskData       *task_data,
                       GCancellable       *cancellable,
                       GAsyncReadyCallback callback,
                       gpointer            user_data)
{
	GTask *task;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), NULL);
	g_return_val_if_fail(task_data != NULL, NULL);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), NULL);
	g_return_val_if_fail(callback != NULL || user_data == NULL, NULL);

	task = g_task_new(self, cancellable, callback, user_data);
	g_task_set_task_data(task, task_data, (GDestroyNotify)gmpd_task_data_unref);

	if (!have_lock)
		LOCK(self);

	if (!self->socket_connection) {
		task_data->error = g_error_new_literal(G_IO_ERROR,
		                                       G_IO_ERROR_CLOSED,
		                                       "The client is closed");

		RETURN_TASK(self, g_object_ref(task), TRUE);

		if (!have_lock)
			UNLOCK(self);

		return task;
	}

	gmpd_client_noidle(self);

	g_queue_push_tail(self->task_queue, g_object_ref(task));

	g_output_stream_write(G_OUTPUT_STREAM(self->output_stream),
	                      task_data->command,
	                      strlen(task_data->command),
	                      NULL,
	                      NULL);

	gmpd_client_attach_output_source(self);
	gmpd_client_attach_input_source(self);

	gmpd_client_enable_timeout(self);

	if (!have_lock)
		UNLOCK(self);

	return task;
}

static void
gmpd_client_do_disconnect(GMpdClient *self)
{
	GTask *task;

	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (self->socket_connection)
		g_io_stream_close(G_IO_STREAM(self->socket_connection), NULL, NULL);

	gmpd_client_destroy_input_source(self);
	gmpd_client_destroy_output_source(self);

	g_clear_object(&self->socket_connection);
	g_clear_object(&self->input_stream);
	g_clear_object(&self->output_stream);

	gmpd_client_do_set_version(self, NULL, TRUE);

	while ((task = g_queue_pop_head(self->task_queue))) {
		GMpdTaskData *data = g_task_get_task_data(task);

		data->error = g_error_new_literal(G_IO_ERROR,
		                                  G_IO_ERROR_CLOSED,
		                                  "The client is closed");

		RETURN_TASK(self, task, TRUE);
	}
}

static void
gmpd_client_update_timeout(GMpdClient *self)
{
	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (gmpd_client_is_idle(self))
		gmpd_client_disable_timeout(self);
	else
		gmpd_client_enable_timeout(self);
}

static gboolean
gmpd_client_is_idle(GMpdClient *self)
{
	GMpdTaskData *data;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), FALSE);

	if (self->task_queue->length == 1) {
		data = g_task_get_task_data(G_TASK(g_queue_peek_tail(self->task_queue)));

		if (GMPD_IS_IDLE_RESPONSE(data->response))
			return TRUE;
	}

	return FALSE;
}

static void
gmpd_client_disable_timeout(GMpdClient *self)
{
	GSocket *socket;

	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (!self->socket_connection)
		return;

	socket = g_socket_connection_get_socket(self->socket_connection);
	g_socket_set_timeout(socket, 0);

	if (self->input_source)
		g_source_set_ready_time(self->input_source, -1);

	if (self->output_source)
		g_source_set_ready_time(self->output_source, -1);
}

static void
gmpd_client_enable_timeout(GMpdClient *self)
{
	GSocket *socket;
	gint64 ready_time;

	g_return_if_fail(GMPD_IS_CLIENT(self));

	if (!self->socket_connection)
		return;

	socket = g_socket_connection_get_socket(self->socket_connection);
	g_socket_set_timeout(socket, self->timeout);

	if (self->timeout)
		ready_time = g_get_monotonic_time() + (self->timeout * 1000000);
	else
		ready_time = -1;

	if (self->output_source)
		g_source_set_ready_time(self->output_source, ready_time);

	if (self->input_source)
		g_source_set_ready_time(self->input_source, ready_time);
}

static gboolean
gmpd_client_do_flush(GMpdClient   *self,
                     GCancellable *cancellable,
                     GError      **error)
{
	GError *err = NULL;
	gboolean result;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), FALSE);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	result = g_output_stream_flush(G_OUTPUT_STREAM(self->output_stream), cancellable, &err);
	if (!result) {
		GTask *task;
		GMpdTaskData *data;

		if (IS_WOULD_BLOCK(err) || IS_CANCELLED(err)) {
			g_propagate_error(error, err);
			return FALSE;
		}

		task = G_TASK(g_queue_pop_head(self->task_queue));
		data = g_task_get_task_data(task);

		data->error = g_error_copy(err);
		g_propagate_error(error, err);

		RETURN_TASK(self, task, TRUE);
		gmpd_client_do_disconnect(self);

		return FALSE;
	}

	gmpd_client_destroy_output_source(self);
	gmpd_client_update_timeout(self);

	return TRUE;
}

static gboolean
gmpd_client_do_fill(GMpdClient   *self,
                    GCancellable *cancellable,
                    GError      **error)
{
	GTask *task;

	g_return_val_if_fail(GMPD_IS_CLIENT(self), FALSE);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	while ((task = g_queue_peek_head(self->task_queue))) {
		GMpdTaskData *data = g_task_get_task_data(task);
		GError *err = NULL;
		gboolean result;

		if (!data->response) {
			g_queue_pop_head(self->task_queue);
			RETURN_TASK(self, task, TRUE);

			gmpd_client_do_disconnect(self);

			return TRUE;
		}

		result = gmpd_response_deserialize(data->response,
		                                   self->version,
		                                   self->input_stream,
		                                   cancellable,
		                                   &err);
		if (!result) {
			if (IS_WOULD_BLOCK(err) || IS_CANCELLED(err)) {
				g_propagate_error(error, err);
				gmpd_client_update_timeout(self);
				return FALSE;
			}

			data->error = g_error_copy(err);
			g_queue_pop_head(self->task_queue);
			RETURN_TASK(self, task, TRUE);

			if (err->domain != GMPD_ERROR) {
				g_propagate_error(error, err);
				gmpd_client_do_disconnect(self);
				return FALSE;
			}

			g_error_free(err);

			continue;
		}

		g_queue_pop_head(self->task_queue);
		RETURN_TASK(self, task, TRUE);
	}

	gmpd_client_destroy_input_source(self);
	return TRUE;
}

static gboolean
gmpd_client_do_sync(GMpdClient   *self,
                    GIOCondition  condition,
                    gboolean      blocking_io,
                    GCancellable *cancellable,
                    GError      **error)
{
	g_return_val_if_fail(GMPD_IS_CLIENT(self), FALSE);
	g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	g_socket_set_blocking(g_socket_connection_get_socket(self->socket_connection), blocking_io);

	if (condition & G_IO_OUT) {
		if (!gmpd_client_do_flush(self, cancellable, error))
			return FALSE;
	}

	if (condition & G_IO_IN) {
		if (!gmpd_client_do_fill(self, cancellable, error))
			return FALSE;
	}

	return TRUE;
}

static gboolean
gmpd_client_on_socket_ready(GSocket     *socket,
                            GIOCondition condition,
                            GMpdClient  *self)
{
	g_return_val_if_fail(G_IS_SOCKET(socket), G_SOURCE_REMOVE);
	g_return_val_if_fail(GMPD_IS_CLIENT(self), G_SOURCE_REMOVE);

	LOCK(self);

	gmpd_client_do_sync(self, condition, FALSE, NULL, NULL);

	UNLOCK(self);

	return G_SOURCE_CONTINUE;
}

static gboolean
return_task(gpointer data)
{
	GTask *task;
	GMpdTaskData *task_data;

	g_return_val_if_fail(G_IS_TASK(data), G_SOURCE_REMOVE);

	task = G_TASK(data);
	task_data = g_task_get_task_data(task);

	if (task_data->error)
		g_task_return_error(task, g_error_copy(task_data->error));

	else if (task_data->response)
		g_task_return_pointer(task, g_object_ref(task_data->response), g_object_unref);

	else
		g_task_return_pointer(task, NULL, NULL);

	return G_SOURCE_REMOVE;
}

