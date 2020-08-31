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
#include "gmpd-object.h"
#include "gmpd-object-priv.h"

typedef struct _DispatchNotifyData DispatchNotifyData;

static void gmpd_object_set_context(GMpdObject *self, GMainContext *context);
static DispatchNotifyData *dispatch_notify_data_new(GObject *object, guint n_pspecs, GParamSpec **pspecs);
static void dispatch_notify_data_free(DispatchNotifyData *data);
static gboolean dispatch_notify_source_func(DispatchNotifyData *data);

enum {
	PROP_NONE,
	PROP_CONTEXT,
	N_PROPERTIES,
};

struct _DispatchNotifyData {
	GObject     *object;
	guint        n_pspecs;
	GParamSpec **pspecs;
};

G_DEFINE_ABSTRACT_TYPE(GMpdObject, gmpd_object, G_TYPE_OBJECT)

static GParamSpec *PROPERTIES[N_PROPERTIES] = {NULL};

static void
gmpd_object_set_property(GObject *object,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *pspec)
{
	GMpdObject *self = GMPD_OBJECT(object);

	switch (prop_id) {
	case PROP_CONTEXT:
		gmpd_object_set_context(self, g_value_get_boxed(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_object_get_property(GObject *object,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *pspec)
{
	GMpdObject *self = GMPD_OBJECT(object);

	switch (prop_id) {
	case PROP_CONTEXT:
		g_value_take_boxed(value, gmpd_object_get_context(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
gmpd_object_finalize(GObject *object)
{
	GMpdObject *self = GMPD_OBJECT(object);

	g_mutex_clear(&self->mutex);
	g_clear_pointer(&self->context, g_main_context_unref);

	G_OBJECT_CLASS(gmpd_object_parent_class)->finalize(object);
}

static void
gmpd_object_dispatch_properties_changed(GObject *object,
                                        guint n_pspecs,
                                        GParamSpec **pspecs)
{
	GMpdObject *self = GMPD_OBJECT(object);
	GSource *source = g_idle_source_new();

	g_source_set_callback(source,
	                      (GSourceFunc)dispatch_notify_source_func,
	                      dispatch_notify_data_new(object, n_pspecs, pspecs),
	                      (GDestroyNotify)dispatch_notify_data_free);

	g_source_attach(source, self->context);
}

static void
gmpd_object_class_init(GMpdObjectClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = gmpd_object_set_property;
	object_class->get_property = gmpd_object_get_property;
	object_class->finalize = gmpd_object_finalize;
	object_class->dispatch_properties_changed = gmpd_object_dispatch_properties_changed;

	PROPERTIES[PROP_CONTEXT] =
		g_param_spec_boxed("context",
		                   "Context",
		                   "The GMainContext in which the object will run",
		                   G_TYPE_MAIN_CONTEXT,
		                   G_PARAM_READWRITE |
		                   G_PARAM_CONSTRUCT_ONLY |
		                   G_PARAM_EXPLICIT_NOTIFY |
		                   G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPERTIES, PROPERTIES);
}

static void
gmpd_object_init(GMpdObject *self)
{
	g_mutex_init(&self->mutex);
}

void
gmpd_object_lock(GMpdObject *self)
{
	g_return_if_fail(GMPD_IS_OBJECT(self));

	g_mutex_lock(&self->mutex);
	g_object_freeze_notify(G_OBJECT(self));
}

void
gmpd_object_unlock(GMpdObject *self)
{
	g_return_if_fail(GMPD_IS_OBJECT(self));

	g_mutex_unlock(&self->mutex);
	g_object_thaw_notify(G_OBJECT(self));
}

guint
gmpd_object_run_in_context(GMpdObject *self,
                           GSourceFunc callback,
                           gpointer data,
                           GDestroyNotify destroy,
                           gboolean have_lock)
{
	GSource *source;
	guint source_id;

	g_return_val_if_fail(GMPD_IS_OBJECT(self), 0);
	g_return_val_if_fail(callback != NULL, 0);

	if (!have_lock)
		gmpd_object_lock(self);

	if (!self->context) {
		if (data && destroy)
			destroy(data);

		if (!have_lock)
			gmpd_object_unlock(self);

		return 0;
	}

	source = g_idle_source_new();
	g_source_set_callback(source, callback, data, destroy);

	source_id = g_source_attach(source, self->context);
	g_source_unref(source);

	if (!have_lock)
		gmpd_object_unlock(self);

	return source_id;
}

GMainContext *
gmpd_object_get_context(GMpdObject *self)
{
	g_return_val_if_fail(GMPD_IS_OBJECT(self), NULL);
	return self->context ? g_main_context_ref(self->context) : NULL;
}

static void
gmpd_object_set_context(GMpdObject *self, GMainContext *context)
{
	g_return_if_fail(GMPD_IS_OBJECT(self));

	g_clear_pointer(&self->context, g_main_context_unref);
	self->context = g_main_context_ref(context);

	g_object_notify_by_pspec(G_OBJECT(self), PROPERTIES[PROP_CONTEXT]);
}

static DispatchNotifyData *
dispatch_notify_data_new(GObject *object, guint n_pspecs, GParamSpec **pspecs)
{
	DispatchNotifyData *data;
	guint i;

	data = g_slice_new(DispatchNotifyData);
	data->object = g_object_ref(object);
	data->n_pspecs = n_pspecs;
	data->pspecs = g_malloc((n_pspecs + 1) * sizeof(GParamSpec *));

	for (i = 0; i < n_pspecs; i++)
		data->pspecs[i] = g_param_spec_ref(pspecs[i]);

	data->pspecs[n_pspecs] = NULL;

	return data;
}

static void
dispatch_notify_data_free(DispatchNotifyData *data)
{
	guint i;

	for (i = 0; i < data->n_pspecs; i++)
		g_param_spec_unref(data->pspecs[i]);

	g_free(data->pspecs);
	g_object_unref(data->object);

	g_slice_free(DispatchNotifyData, data);
}

static gboolean
dispatch_notify_source_func(DispatchNotifyData *data)
{
	GObjectClass *parent_class = G_OBJECT_CLASS(gmpd_object_parent_class);

	parent_class->dispatch_properties_changed(data->object, data->n_pspecs, data->pspecs);

	return FALSE;
}

