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

#ifndef __OHM_PLUGIN_H
#define __OHM_PLUGIN_H

#ifndef __USE_GNU
#define __USE_GNU 1                      /* we want Dl_info, dladdr, et al... */
#endif
#include <dlfcn.h>

#include <dbus/dbus.h>
#include <gmodule.h>

#include <ohm/ohm-dbus.h>

G_BEGIN_DECLS

typedef struct _OhmPlugin OhmPlugin;
typedef struct _OhmPluginDesc OhmPluginDesc;
typedef struct _OhmPluginKeyIdMap OhmPluginKeyIdMap;

struct _OhmPluginKeyIdMap {
	const char	*key_name;
	gint		local_key_id;
};

typedef enum {
	OHM_LICENSE_LGPL,
	OHM_LICENSE_GPL,
	OHM_LICENSE_MIT,
	OHM_LICENSE_BSD,
	OHM_LICENSE_NON_FREE,
	OHM_LICENSE_FREE_OTHER
} OhmLicenseType;

typedef struct {
  const char *name;                             /* method name */
  const char *signature;                        /* method signature */
  void       *ptr;                              /* method pointer */
} ohm_method_t;

#define OHM_PLUGIN_METHODS_END { NULL, NULL, NULL }


/**
 * OhmPluginDesc:
 * @description: Plugin description
 * @version: Plugin version
 * @author: Plugin author
 * @license: Plugin license type
 * @initialize: method to call on plugin initialization
 * @destroy: method to call on plugin destruction
 * @notify: method to call to notify of key changes, using the id's described by
 *          #OHM_PLUGIN_INTERESTED
 * @padding: Padding for future expansion
 */
struct _OhmPluginDesc {
	const char		*description;
	const char		*version;
	const char		*author;
	OhmLicenseType	license;
	void		 (*initialize)(OhmPlugin *plugin);
	void		 (*destroy)   (OhmPlugin *plugin);
	void		 (*notify)    (OhmPlugin *plugin, gint id, gint value);

  	ohm_method_t      *exports;
  	ohm_method_t      *imports;
	gpointer padding[6];
};

#define OHM_PLUGIN_DESCRIPTION(description, version, author, license, initialize, destroy, notify) \
	G_MODULE_EXPORT const OhmPluginDesc ohm_plugin_desc = { \
		description, \
		version, \
		author, \
		license,\
		initialize, \
		destroy, \
		notify, \
		NULL, \
		NULL, \
		{0} \
	}

#define OHM_PLUGIN_INTERESTED(...) \
	G_MODULE_EXPORT const OhmPluginKeyIdMap ohm_plugin_interested[] = {__VA_ARGS__, {NULL,0}}

#define OHM_PLUGIN_PROVIDES(...) \
	G_MODULE_EXPORT const gchar *ohm_plugin_provides[] = {__VA_ARGS__,NULL}

#define OHM_PLUGIN_REQUIRES(...) \
	G_MODULE_EXPORT const gchar *ohm_plugin_requires[] = {__VA_ARGS__,NULL}

#define OHM_PLUGIN_SUGGESTS(...) \
	G_MODULE_EXPORT const gchar *ohm_plugin_suggests[] = {__VA_ARGS__,NULL}

#define OHM_PLUGIN_PREVENTS(...) \
	G_MODULE_EXPORT const gchar *ohm_plugin_prevents[] = {__VA_ARGS__,NULL}


#define OHM_EXPORTABLE(return_type, name, arguments)		 \
  static const char *name##_SIGNATURE = #return_type #arguments; \
  static return_type name arguments

#define OHM_EXPORT(name, public_name)		\
    { public_name, name##_SIGNATURE, name }

#define OHM_EXPORT_VAR "_plugin_exports"
#define OHM_PLUGIN_PROVIDES_METHODS(plugin, n, ...)			\
  G_MODULE_EXPORT ohm_method_t plugin##_plugin_exports[(n)+1] = {	\
    [0 ... (n)] = OHM_PLUGIN_METHODS_END };				\
  static void __attribute__((constructor)) plugin_init_exports(void)	\
  {									\
    G_MODULE_EXPORT ohm_method_t exports[] = {				\
      __VA_ARGS__,							\
      OHM_PLUGIN_METHODS_END						\
    };									\
    int i;								\
    for (i = 0; exports[i].name; i++)					\
      plugin##_plugin_exports[i] = exports[i];				\
  }


#define OHM_IMPORTABLE(return_type, name, arguments)			\
    static const char *name##_SIGNATURE = #return_type #arguments;	\
    static return_type (*name) arguments

#define OHM_IMPORT(public_name, name)			\
    { public_name, name##_SIGNATURE, (void *)&name }
    
#define OHM_IMPORT_VAR "_plugin_imports"
#define OHM_PLUGIN_REQUIRES_METHODS(plugin, n, ...)			\
  G_MODULE_EXPORT ohm_method_t plugin##_plugin_imports[(n)+1] = {	\
    [0 ... (n)] = OHM_PLUGIN_METHODS_END };				\
  static void __attribute__((constructor)) plugin_init_imports(void)	\
  {									\
    G_MODULE_EXPORT ohm_method_t imports[] = {				\
      __VA_ARGS__,							\
      OHM_PLUGIN_METHODS_END						\
    };									\
    int i;								\
    for (i = 0; imports[i].name; i++)					\
      plugin##_plugin_imports[i] = imports[i];				\
  }

#define OHM_PLUGIN_DBUS_METHODS(...)				  \
  G_MODULE_EXPORT ohm_dbus_method_t ohm_plugin_dbus_methods[] = { \
    __VA_ARGS__,						  \
    OHM_DBUS_METHODS_END,					  \
  }

#define OHM_PLUGIN_DBUS_SIGNALS(...) \
  G_MODULE_EXPORT ohm_dbus_signal_t ohm_plugin_dbus_signals[] = { \
    __VA_ARGS__,						  \
    OHM_DBUS_SIGNALS_END					  \
  }


typedef void (*OhmPluginHalPropMod)			(OhmPlugin	*plugin,
							 guint		 id,
							 const gchar	*key);
typedef void (*OhmPluginHalCondition)			(OhmPlugin	*plugin,
							 guint		 id,
							 const gchar	*name,
							 const gchar	*detail);

/* used by plugin to do crazy stuff */
gboolean	 ohm_plugin_spawn_async			(OhmPlugin      *plugin,
							 const gchar	*commandline);

/* used by plugin to manager */
gboolean	 ohm_plugin_conf_get_key		(OhmPlugin      *plugin,
							 const gchar	*key,
							 gint		*value);
gboolean	 ohm_plugin_conf_set_key		(OhmPlugin      *plugin,
							 const gchar	*key,
							 gint		 value);
/* used by plugin for hal */
gboolean	 ohm_plugin_hal_init			(OhmPlugin	*plugin);
gboolean	 ohm_plugin_hal_use_property_modified	(OhmPlugin	*plugin,
							 OhmPluginHalPropMod func);
gboolean	 ohm_plugin_hal_use_condition		(OhmPlugin	*plugin,
							 OhmPluginHalCondition func);
gboolean	 ohm_plugin_hal_get_bool		(OhmPlugin	*plugin,
							 guint		 id,
							 const gchar	*key,
							 gboolean	*state);
gboolean	 ohm_plugin_hal_get_int			(OhmPlugin	*plugin,
							 guint		 id,
							 const gchar	*key,
							 gint		*state);
gchar		*ohm_plugin_hal_get_udi			(OhmPlugin	*plugin,
							 guint		 id);
guint		 ohm_plugin_hal_add_device_capability	(OhmPlugin	*plugin,
							 const gchar	*capability);


/* used by plugin for non-HAL DBUS access */

DBusConnection *ohm_plugin_dbus_get_connection(void);

int ohm_plugin_dbus_add_method(const char *path, const char *member,
			       DBusObjectPathMessageFunction handler,
			       void *data);

int ohm_plugin_dbus_del_method(const char *path, const char *member,
			       DBusObjectPathMessageFunction handler,
			       void *data);

int ohm_plugin_dbus_add_signal(const char *sender, const char *interface,
			       const char *sig, const char *path,
			       DBusObjectPathMessageFunction handler,
			       void *data);
void ohm_plugin_dbus_del_signal(const char *sender, const char *interface,
				const char *sig, const char *path,
				DBusObjectPathMessageFunction handler,
				void *data);

/* used by plugin for dynamically resolve exported plugin methods */
gboolean ohm_module_find_method(char *name, char **sigptr, void **methptr);


G_END_DECLS

#endif /* __OHM_PLUGIN_H */