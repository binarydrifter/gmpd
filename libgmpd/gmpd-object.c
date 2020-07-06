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

typedef struct _DispatchNotifyData DispatchNotifyData;

static DispatchNotifyData *dispatch_notify_data_new(GObject *object, guint n_pspecs, GParamSpec **pspecs);
static void dispatch_notify_data_free(DispatchNotifyData *data);
static gboolean dispatch_notify_source_func(DispatchNotifyData *data);

struct _DispatchNotifyData {
	GObject *object;
	guint n_pspecs;
	GParamSpec **pspecs;
};

G_DEFINE_TYPE(GMpdObject, gmpd_object, G_TYPE_OBJECT)

static void
gmpd_object_finalize(GObject *object)
{
	GMpdObject *self = GMPD_OBJECT(object);

	g_mutex_clear(&self->mutex);

	G_OBJECT_CLASS(gmpd_object_parent_class)->finalize(object);
}

static void
gmpd_object_dispatch_properties_changed(GObject *object,
                                        guint n_pspecs,
                                        GParamSpec **pspecs)
{
	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
	                (GSourceFunc)dispatch_notify_source_func,
	                dispatch_notify_data_new(object, n_pspecs, pspecs),
	                (GDestroyNotify)dispatch_notify_data_free);
}

static void
gmpd_object_class_init(GMpdObjectClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize = gmpd_object_finalize;
	object_class->dispatch_properties_changed = gmpd_object_dispatch_properties_changed;
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

