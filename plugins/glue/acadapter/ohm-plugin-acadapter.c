/*
 * Copyright (C) 2007 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU Lesser General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <gmodule.h>
#include <glib.h>
#include <string.h>
#include <ohm-plugin.h>

/**
 * plugin_preload:
 * @plugin: This class instance
 *
 * Called before the plugin is coldplg.
 * Define any modules that the plugin depends on, but do not do coldplug here
 * as some of the modules may not have loaded yet.
 */
static gboolean
plugin_preload (OhmPlugin *plugin)
{
	ohm_plugin_conf_provide (plugin, "acadapter.state");
	return TRUE;
}

static void
hal_property_changed_cb (OhmPlugin   *plugin,
			 guint        id,
			 const gchar *key)
{
	gboolean state;
	if (strcmp (key, "ac_adapter.present") == 0) {
		ohm_plugin_hal_get_bool (plugin, id, "ac_adapter.present", &state);
		ohm_plugin_conf_set_key (plugin, "acadapter.state", state);
	}
}

/**
 * plugin_coldplug:
 * @plugin: This class instance
 *
 * Coldplug, i.e. read and set the initial state of the plugin.
 * We can assume all the required modules have been loaded, although it's
 * dangerous to assume the key values are anything other than the defaults.
 */
static void
plugin_coldplug (OhmPlugin *plugin)
{
	gboolean state;
	guint num;

	/* initialise HAL */
	ohm_plugin_hal_init (plugin);

	/* we want this function to get the property modified events for all devices */
	ohm_plugin_hal_use_property_modified (plugin, hal_property_changed_cb);

	/* get the only device with capability and watch it */
	num = ohm_plugin_hal_add_device_capability (plugin, "ac_adapter");
	if (num == 1) {
		ohm_plugin_hal_get_bool (plugin, 0, "ac_adapter.present", &state);
	} else {
		g_warning ("not tested with not one acadapter");
		state = 1;
	}

	ohm_plugin_conf_set_key (plugin, "acadapter.state", state);
}

static OhmPluginInfo plugin_info = {
	"OHM HAL AC Adapter",		/* description */
	"0.0.1",			/* version */
	"richard@hughsie.com",		/* author */
	plugin_preload,			/* preload */
	NULL,				/* unload */
	plugin_coldplug,		/* coldplug */
	NULL,				/* conf_notify */
};

OHM_INIT_PLUGIN (plugin_info);