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
#include "gmpd-response.h"
#include "gmpd-void-response.h"

static void gmpd_void_response_iface_init(GMpdResponseIface *iface G_GNUC_UNUSED);

G_DEFINE_TYPE_WITH_CODE(GMpdVoidResponse, gmpd_void_response, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(GMPD_TYPE_RESPONSE,
                                              gmpd_void_response_iface_init))

static void
gmpd_void_response_iface_init(GMpdResponseIface *iface G_GNUC_UNUSED)
{
	/* use defaults */
}

static GObject *
gmpd_void_response_constructor(GType type,
                               guint n_properties,
                               GObjectConstructParam *properties)
{
	static gsize init = 0;
	static gpointer inst = NULL;

	g_return_val_if_fail(type == GMPD_TYPE_VOID_RESPONSE, NULL);

	if (g_once_init_enter(&init)) {
		GObjectClass *parent_class = G_OBJECT_CLASS(gmpd_void_response_parent_class);

		inst = parent_class->constructor(type, n_properties, properties);

		g_once_init_leave(&init, 1);
	}

	return g_object_ref(inst);
}

static void
gmpd_void_response_class_init(GMpdVoidResponseClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->constructor = gmpd_void_response_constructor;
}

static void
gmpd_void_response_init(GMpdVoidResponse *self G_GNUC_UNUSED)
{
}

GMpdVoidResponse *
gmpd_void_response_new(void)
{
	return g_object_new(GMPD_TYPE_VOID_RESPONSE, NULL);
}

