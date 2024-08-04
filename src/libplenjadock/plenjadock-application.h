/* inclusion guard */
#pragma once

#include <glib-object.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include <libwnck/libwnck.h>

/*
 * Potentially, include other headers on which this header depends.
 */

G_BEGIN_DECLS

/*
 * Type declaration.
 */
#define PLENJADOCK_TYPE_APPLICATION plenjadock_application_get_type ()
G_DECLARE_FINAL_TYPE (PlenjaDockApplication, plenjadock_application, PLENJADOCK, APPLICATION, GObject)

/*
 * Method definitions.
 */
PlenjaDockApplication *plenjadock_application_new (void);
int plenjadock_application_get_contains_window (PlenjaDockApplication *self, WnckWindow *window);
void plenjadock_application_unrender (PlenjaDockApplication *self);
GtkWidget *plenjadock_application_render (PlenjaDockApplication *self);
void plenjadock_application_set_app (PlenjaDockApplication *self, WnckApplication *app);
WnckApplication *plenjadock_application_get_app (PlenjaDockApplication *self);
void plenjadock_application_set_props (PlenjaDockApplication *self, gchar *icon_name, gchar *app_name, gchar *path, gchar *desktop_path, gboolean pinned, GtkIconTheme *icon_theme, int icon_size, int scale);
char *plenjadock_application_get_exec_path (PlenjaDockApplication *self);
gboolean plenjadock_application_get_pinned (PlenjaDockApplication *self);
void plenjadock_application_set_pinned (PlenjaDockApplication *self, gboolean pinned);
gboolean plenjadock_application_get_is_open (PlenjaDockApplication *self);
void plenjadock_application_update_indicator (PlenjaDockApplication *self);
char *plenjadock_application_get_desktop_path (PlenjaDockApplication *self);
void plenjadock_application_set_desktop_path (PlenjaDockApplication *self, gchar *desktop_path);

G_END_DECLS
