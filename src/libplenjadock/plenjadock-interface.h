// GObject creation tutorial: https://docs.gtk.org/gobject/tutorial.html

/* inclusion guard */
#pragma once

#include <glib-object.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include <libwnck/libwnck.h>

#include <gtk/gtk.h>

#include <gdk/gdkx.h>

#include "plenjadock-application.h"

#include <math.h>

#include <stdlib.h>

char *secure_getenv (const char *name);

/*
 * Potentially, include other headers on which this header depends.
 */

G_BEGIN_DECLS

/*
 * Type declaration.
 */
#define PLENJADOCK_TYPE_INTERFACE plenjadock_interface_get_type ()
G_DECLARE_FINAL_TYPE (PlenjaDockInterface, plenjadock_interface, PLENJADOCK, INTERFACE, GObject)

/*
 * Method definitions.
 */
PlenjaDockInterface *plenjadock_interface_new (void);
int plenjadock_interface_start (PlenjaDockInterface *self);
PlenjaDockApplication *plenjadock_interface_get_active_app (PlenjaDockInterface *self);
void plenjadock_interface_set_apps_container (PlenjaDockInterface *self, GtkContainer *container);
GtkWidget *plenjadock_interface_add_app (PlenjaDockInterface *self, PlenjaDockApplication *app, int position, gboolean can_pin_unpin);
GdkWindow *plenjadock_interface_get_desktop_window (PlenjaDockInterface *self);

void free_string_list (gchar **list);

G_END_DECLS
