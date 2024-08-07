/* panel-window.c
 *
 * Copyright 2023 Benjamin Montgomery
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "panel-config.h"
#include "panel-window.h"

struct _PanelWindow
{
  GtkWindow parent_instance;

  GtkBox *window_box;

  PlenjaDockInterface *dock_interface;

  gboolean supports_alpha;

  GSettings *settings;

  GdkWindow *desktop_window;

  GdkPixbuf *desktop_blurred;

  //PanelDashboardWindow *dashboard_window;
  PanelApplicationsMenu *apps_menu;

  GdkRectangle monitor_geometry;
};

G_DEFINE_TYPE (PanelWindow, panel_window, GTK_TYPE_WINDOW)

static void
panel_window_class_init (PanelWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/plenjos/Panel/panel-window.ui");
  gtk_widget_class_bind_template_child (widget_class, PanelWindow, window_box);
}

void screen_changed (GtkWidget *widget, GdkScreen *old_screen, PanelWindow *self) {
  // fix unused parameter warning
  (void)old_screen;
  GdkScreen *screen = gtk_widget_get_screen (widget);
  GdkVisual *visual = gdk_screen_get_rgba_visual (screen);

  if (!visual) {
    printf ("Your screen does not support alpha channels!\n");
    visual = gdk_screen_get_system_visual (screen);
    self->supports_alpha = FALSE;
  } else {
    printf ("Your screen supports alpha channels!\n");
    self->supports_alpha = TRUE;
  }

  fflush (stdout);

  gtk_widget_set_visual (widget, visual);
}

void update_desktop_pixbuf (PanelWindow *self) {
  g_return_if_fail (GDK_IS_WINDOW (self->desktop_window));

  GdkWindow *win = gtk_widget_get_window (GTK_WIDGET (&self->parent_instance));

  GdkPixbuf *pbuf = gdk_pixbuf_get_from_window (self->desktop_window, 0, 0, self->monitor_geometry.width, self->monitor_geometry.height);

  int scale_factor = gdk_monitor_get_scale_factor (gdk_display_get_monitor_at_window (gdk_display_get_default (), win));

  cairo_surface_t *surfaceold = gdk_cairo_surface_create_from_pixbuf (pbuf, 1, win);

  cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, cairo_image_surface_get_width (surfaceold), cairo_image_surface_get_height (surfaceold));

  cairo_surface_set_device_scale (surface, scale_factor, scale_factor);

  stack_blur (surfaceold, surface, 240, 240);

  if (GDK_IS_PIXBUF (self->desktop_blurred)) {
    g_object_unref (self->desktop_blurred);
  }
  self->desktop_blurred = NULL;

  GdkPixbuf *unscaled = gdk_pixbuf_get_from_surface (surface, 0, 0, self->monitor_geometry.width * scale_factor, self->monitor_geometry.height * scale_factor);

  self->desktop_blurred = gdk_pixbuf_scale_simple (unscaled, self->monitor_geometry.width, self->monitor_geometry.height, GDK_INTERP_BILINEAR);

  g_object_unref (unscaled);
  g_object_unref (pbuf);
  cairo_surface_destroy (surfaceold);
  cairo_surface_destroy (surface);
}

gboolean expose_draw (GtkWidget *widget, cairo_t *cr, PanelWindow *self) {
  cairo_save (cr);

  // Adapted from https://stackoverflow.com/questions/4183546/how-can-i-draw-image-with-rounded-corners-in-cairo-gtk
  double width, height, radius;
  int wi, hi;
  gtk_window_get_size (&self->parent_instance, &wi, &hi);

  width = (double)wi - 8;
  height = (double)hi - 6;
  radius = 16;

  double x = 4;
  double y = 2;

  double degrees = M_PI / 180.0;

  cairo_new_sub_path (cr);
  cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
  cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
  cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
  cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
  cairo_close_path (cr);

  cairo_clip (cr);

  if (GDK_IS_PIXBUF (self->desktop_blurred)) {
    gint x_win,y_win;
    gdk_window_get_position (gtk_widget_get_window (widget), &x_win, &y_win);
    gdk_cairo_set_source_pixbuf (cr, self->desktop_blurred, -x_win, -y_win);
  } else {
    if (self->supports_alpha) {
      cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
    } else {
      cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    }
  }

  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);

  cairo_restore (cr);

  return FALSE;
}

gboolean on_configure_event (GtkWidget *widget, GdkEventConfigure config, PanelWindow *self) {
  (void)widget;
  if (config.x != ((self->monitor_geometry.width) - config.width) / 2
      || config.y != self->monitor_geometry.height - config.height) {
    gdk_window_move (config.window, (self->monitor_geometry.width - config.width) / 2, self->monitor_geometry.height - config.height);

    printf("%d %d %d %d\n", self->monitor_geometry.width, self->monitor_geometry.height, config.width, config.height);
    fflush(stdout);
    //gtk_window_move (GTK_WINDOW (widget), (self->monitor_geometry.width - config.width) / 2, self->monitor_geometry.height - config.height);
  }

  return FALSE;
}

static void on_desktop_window_register (PlenjaDockInterface *interface, PanelWindow *self) {
  self->desktop_window = plenjadock_interface_get_desktop_window (interface);

  update_desktop_pixbuf (self);
}

static void on_apps_menu_show_request (PlenjaDockInterface *interface, PanelWindow *self) {
  // fix unused parameter warning
  (void)interface;
  printf("test\n");
  fflush(stdout);

  if (!self->apps_menu) {
    self->apps_menu = g_object_new (PANEL_TYPE_APPLICATIONS_MENU,
		                         "application", NULL,
		                         NULL);
  }

  show_applications_menu (self->apps_menu);
}

static void
panel_window_init (PanelWindow *self)
{
  hdy_init ();

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (&self->parent_instance, "configure-event", G_CALLBACK (on_configure_event), self);

  g_signal_connect (&self->parent_instance, "draw", G_CALLBACK (expose_draw), self);
  g_signal_connect (&self->parent_instance, "screen-changed", G_CALLBACK (screen_changed), self);

  screen_changed (GTK_WIDGET (&self->parent_instance), NULL, self);

  // TODO: should this be gtk_window_present () instead?
  gtk_widget_show (GTK_WIDGET (&self->parent_instance));

  GdkWindow *dock_window = gtk_widget_get_window (GTK_WIDGET (&self->parent_instance));

  gdk_monitor_get_geometry (gdk_display_get_monitor_at_window (gdk_display_get_default (), gtk_widget_get_window (GTK_WIDGET (&self->parent_instance))), &self->monitor_geometry);

  gtk_widget_set_size_request (GTK_WIDGET (&self->parent_instance), 0, 72);
  gdk_window_move (dock_window, self->monitor_geometry.width / 2, self->monitor_geometry.height - 72);
  gdk_window_set_keep_above (dock_window, TRUE);

  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource (cssProvider, "/com/plenjos/Panel/theme.css");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                               GTK_STYLE_PROVIDER (cssProvider),
                               GTK_STYLE_PROVIDER_PRIORITY_USER);

  gtk_widget_set_name (GTK_WIDGET (&self->parent_instance), "dockwindow");
  gtk_widget_set_name (GTK_WIDGET (self->window_box), "dockwindowbox");

  long *prop1 = malloc (4 * sizeof (long));
  prop1[0] = 0;
  prop1[1] = 0;
  prop1[2] = 0;
  prop1[3] = 72*gdk_window_get_scale_factor (dock_window);

  GdkWindow *topw = gdk_window_get_toplevel (dock_window);
  gdk_property_change (topw, gdk_atom_intern ("_NET_WM_STRUT", TRUE), gdk_atom_intern ("CARDINAL", TRUE), 32, GDK_PROP_MODE_REPLACE, (const guchar *)prop1, 4);

  free (prop1);

  self->dock_interface = g_object_new (PLENJADOCK_TYPE_INTERFACE, NULL);

  plenjadock_interface_set_apps_container (self->dock_interface, GTK_CONTAINER (self->window_box));

  plenjadock_interface_start (self->dock_interface);
  g_signal_connect (self->dock_interface, "desktop-window-registered", G_CALLBACK (on_desktop_window_register), self);
  g_signal_connect (self->dock_interface, "panel-dashboard-show-request", G_CALLBACK (on_apps_menu_show_request), self);

  // IMPORTANT TODO: fix app pinning when StartupWMClass is not set
}
