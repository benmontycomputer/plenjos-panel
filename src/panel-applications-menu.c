/* panel-applications-menu.c
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
#include "panel-applications-menu.h"

#define MARKUP_FORMAT "<span size=\"small\">%s</span>"

struct _PanelApplicationsMenu
{
  GtkApplicationWindow parent_instance;

  GtkScrolledWindow *scrolled_window;

  GtkFlowBox *apps_flow_box;
  // GtkGrid *favorites_grid;

  gboolean supports_alpha;

  size_t items_count;
  size_t favorites_count;

  GtkIconTheme *icon_theme;
  int icon_size;
  int scale;

  GdkPixbuf *desktop_blurred;

  GdkRectangle monitor_geometry;

  GSettings *settings;
};

G_DEFINE_TYPE(PanelApplicationsMenu, panel_applications_menu, GTK_TYPE_APPLICATION_WINDOW)

static void
panel_applications_menu_class_init(PanelApplicationsMenuClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(widget_class, "/com/plenjos/Panel/panel-applications-menu.ui");
  gtk_widget_class_bind_template_child(widget_class, PanelApplicationsMenu, apps_flow_box);
  // gtk_widget_class_bind_template_child (widget_class, PanelApplicationsMenu, favorites_grid);
  gtk_widget_class_bind_template_child(widget_class, PanelApplicationsMenu, scrolled_window);
}

static int applications_menu_hide(GtkWidget *widget)
{
  gdk_window_hide(gtk_widget_get_window(widget));
  return FALSE;
}

void hide_applications_menu(PanelApplicationsMenu *self)
{
  applications_menu_hide(GTK_WIDGET(&self->parent_instance));
}

void show_applications_menu(PanelApplicationsMenu *self)
{
  gtk_window_present(GTK_WINDOW (&self->parent_instance));

  gtk_window_resize(GTK_WINDOW(&self->parent_instance), self->monitor_geometry.width, self->monitor_geometry.height);
  gtk_window_move(GTK_WINDOW(&self->parent_instance), 0, 0);
}

static gboolean check_escape(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  (void)data;
  if (event->keyval == GDK_KEY_Escape)
  {
    applications_menu_hide(widget);
    return TRUE;
  }
  return FALSE;
}

static void focus_out_event(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
  (void)event;
  (void)user_data;

  applications_menu_hide(widget);
}

typedef struct
{
  char *exec;
  PanelApplicationsMenu *self;
} ClickedArgs;

static void applications_menu_app_clicked(GtkWidget *widget, GdkEventButton *event, ClickedArgs *args)
{
  // stops unused parameter warning
  (void)event;
  (void)widget;

  hide_applications_menu(GTK_WIDGET(&args->self->parent_instance));
  size_t new_exec_len = strlen(args->exec) + 3;
  char *new_exec = malloc(new_exec_len);
  snprintf(new_exec, new_exec_len, "%s &", args->exec);
  system(new_exec);
  free(new_exec);
}

GtkWidget *applications_menu_render_app(PanelApplicationsMenu *self,
                                        char *icon_name,
                                        char *exec_path,
                                        char *display_name)
{
  GtkButton *button = GTK_BUTTON(gtk_button_new());
  gtk_widget_set_name(GTK_WIDGET(button), "panel_applications_menu_item");
  GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2));

  GtkImage *icon = GTK_IMAGE(gtk_image_new());

  if (icon_name)
  {
    GdkPixbuf *icon_unscaled = gtk_icon_theme_load_icon_for_scale(self->icon_theme, icon_name, self->icon_size, self->scale, 0, NULL);

    if (icon_unscaled)
    {
      cairo_surface_t *s = gdk_cairo_surface_create_from_pixbuf(icon_unscaled, 0, gtk_widget_get_window(GTK_WIDGET(icon)));

      if (s)
      {
        gtk_image_set_from_surface(icon, s);
        cairo_surface_destroy(s);
      }

      g_object_unref(icon_unscaled);
    }
  }

  GtkLabel *label = GTK_LABEL(gtk_label_new(""));

  size_t len = 12;
  char *name = malloc(len);
  snprintf(name, len, "%s", display_name);

  if (display_name && strlen(display_name) >= len)
  {
    name[len - 4] = '.';
    name[len - 3] = '.';
    name[len - 2] = '.';
    gtk_widget_set_tooltip_text(GTK_WIDGET(label), display_name);
  }

  // gtk_label_set_text (label, name);

  char *markup = NULL;

  markup = g_markup_printf_escaped(MARKUP_FORMAT, name);

  free(name);

  gtk_label_set_markup(label, markup);

  free(markup);

  gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(icon));
  gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(label));
  gtk_widget_show_all(GTK_WIDGET(box));
  gtk_button_set_image(button, GTK_WIDGET(box));

  ClickedArgs *clicked_args = malloc(sizeof(ClickedArgs));

  clicked_args->exec = g_strdup(exec_path);
  clicked_args->self = self;

  g_signal_connect(button, "button-press-event", G_CALLBACK(applications_menu_app_clicked), clicked_args);

  return GTK_WIDGET(button);
}

static void applications_menu_add_app_from(PanelApplicationsMenu *self, char *icon_name, char *exec_path, char *display_name)
{
  GtkWidget *button = applications_menu_render_app(self, icon_name, exec_path, display_name);

  gtk_container_add(GTK_CONTAINER(self->apps_flow_box), GTK_WIDGET(button));

  self->items_count++;
}

void panel_applications_menu_set_bg(PanelApplicationsMenu *self, GdkPixbuf *bg)
{
  self->desktop_blurred = bg;

  gtk_widget_queue_draw(GTK_WIDGET(&self->parent_instance));
}

int sort_apps(GKeyFile *kf1, GKeyFile *kf2)
{
  gchar *name1 = g_key_file_get_string(kf1, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL);
  gchar *name2 = g_key_file_get_string(kf2, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL);

  if (!name1)
  {
    if (name2)
      free(name2);
    return 1;
  }
  if (!name2)
  {
    // We don't need to check if name1 is null because the function would have already exited if it was.
    free(name1);
    return -1;
  }
  // https://stackoverflow.com/questions/2661766/how-do-i-lowercase-a-string-in-c
  for (size_t i = 0; name1[i]; i++)
  {
    name1[i] = tolower(name1[i]);
  }
  for (size_t i = 0; name2[i]; i++)
  {
    name2[i] = tolower(name2[i]);
  }
  int returnval = strcmp(name1, name2);
  free(name1);
  free(name2);
  return returnval;
}

gboolean expose_draw_dashboard(GtkWidget *widget, cairo_t *cr, PanelApplicationsMenu *self)
{
  cairo_save(cr);

  // Adapted from https://stackoverflow.com/questions/4183546/how-can-i-draw-image-with-rounded-corners-in-cairo-gtk
  double width, height, radius;
  int wi, hi;
  gtk_window_get_size(&self->parent_instance, &wi, &hi);

  width = (double)wi - 8;
  height = (double)hi - 6;
  radius = 16;

  double x = 4;
  double y = 2;

  double degrees = M_PI / 180.0;

  cairo_new_sub_path(cr);
  cairo_arc(cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
  cairo_arc(cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
  cairo_arc(cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
  cairo_arc(cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
  cairo_close_path(cr);

  cairo_clip(cr);

  if (GDK_IS_PIXBUF(self->desktop_blurred))
  {
    gint x_win, y_win;
    gdk_window_get_position(gtk_widget_get_window(widget), &x_win, &y_win);
    gdk_cairo_set_source_pixbuf(cr, self->desktop_blurred, -x_win, -y_win);
  }
  else
  {
    if (self->supports_alpha)
    {
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    }
    else
    {
      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    }
  }

  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);

  cairo_restore(cr);

  return FALSE;
}

// https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
char **str_split(char *a_str, const char a_delim)
{
  char **result = 0;
  size_t count = 0;
  char *tmp = a_str;
  char *last_comma = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;

  /* Count how many elements will be extracted. */
  while (*tmp)
  {
    if (a_delim == *tmp)
    {
      count++;
      last_comma = tmp;
    }
    tmp++;
  }

  /* Add space for trailing token. */
  count += last_comma < (a_str + strlen(a_str) - 1);

  /* Add space for terminating null string so caller
     knows where the list of returned strings ends. */
  count++;

  result = malloc(sizeof(char *) * count);

  if (result)
  {
    size_t idx = 0;
    char *token = strtok(a_str, delim);

    while (token)
    {
      assert(idx < count);
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    assert(idx == count - 1);
    *(result + idx) = 0;
  }

  return result;
}

static void
panel_applications_menu_init(PanelApplicationsMenu *self)
{
  if (!self->settings)
  {
    self->settings = g_settings_new("com.plenjos.Panel");
  }

  gtk_widget_init_template(GTK_WIDGET(self));

  g_signal_connect(&self->parent_instance, "key_press_event", G_CALLBACK(check_escape), NULL);
  g_signal_connect(&self->parent_instance, "focus-out-event", G_CALLBACK(focus_out_event), NULL);

  g_signal_connect(&self->parent_instance, "draw", G_CALLBACK(expose_draw_dashboard), self);

  gtk_widget_show(GTK_WIDGET(&self->parent_instance));

  gdk_monitor_get_geometry(gdk_display_get_monitor_at_window(gtk_widget_get_display(GTK_WIDGET(&self->parent_instance)), gtk_widget_get_window(GTK_WIDGET(&self->parent_instance))), &self->monitor_geometry);

  GdkScreen *screen = gtk_widget_get_screen(GTK_WIDGET(&self->parent_instance));

  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_resource(cssProvider, "/com/plenjos/Panel/theme.css");
  gtk_style_context_add_provider_for_screen(screen,
                                            GTK_STYLE_PROVIDER(cssProvider),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);

  // gtk_widget_set_size_request (GTK_WIDGET (&self->parent_instance), self->monitor_geometry.width, self->monitor_geometry.height);
  gtk_window_set_keep_above(&self->parent_instance, TRUE);

  // TODO: possibly keep the window always open or loaded so it doesn't reload apps and icons every time

  self->icon_theme = gtk_icon_theme_get_default();
  self->icon_size = 96;
  self->scale = gtk_widget_get_scale_factor(GTK_WIDGET(&self->parent_instance));

  char *data_dirs = getenv("XDG_DATA_DIRS");

  GList *applications = NULL;

  if (!data_dirs)
  {
    fprintf(stderr, "Error: XDG_DATA_DIRS is not set. Apps menu may not load applications.\n");
    fflush(stderr);
  }
  else
  {
    // https://stackoverflow.com/questions/7704144/how-do-i-use-glib-or-any-other-library-to-list-all-the-files-in-a-directory
    char **data_dirs_list = str_split(data_dirs, ':');

    for (size_t data_dir_index = 0; data_dirs_list[data_dir_index]; data_dir_index++) {
      size_t applications_dir_len = strlen(data_dirs_list[data_dir_index]) + strlen("/applications/") + 1;
      char *applications_dir = malloc(applications_dir_len);

      snprintf(applications_dir, applications_dir_len, "%s/applications/", data_dirs_list[data_dir_index]);

      DIR *d;
      struct dirent *dir;

      d = opendir(applications_dir);
      if (d)
      {
        while ((dir = readdir(d)) != NULL)
        {
          GKeyFile *key_file = g_key_file_new();

          size_t len = strlen(applications_dir) + strlen(dir->d_name) + 1;
          char *path = malloc(len);

          snprintf(path, len, "%s%s", applications_dir, dir->d_name);

          printf("%s\n", path);
          fflush(stdout);
          if (g_key_file_load_from_file(key_file, path, G_KEY_FILE_NONE, NULL))
          {
            printf("%s\n", path);
            applications = g_list_append(applications, key_file);
          }
          else
          {
            g_key_file_free(key_file);
          }

          free(path);
        }
        closedir(d);
      }

      free (applications_dir);
    }

    fflush(stdout);

    applications = g_list_sort(applications, (GCompareFunc)sort_apps);

    for (GList *application = applications; application; application = application->next)
    {
      gchar *name = g_key_file_get_string((GKeyFile *)application->data, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL);
      gchar *exec = g_key_file_get_string((GKeyFile *)application->data, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
      gchar *icon = g_key_file_get_string((GKeyFile *)application->data, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

      if (name && exec)
      {
        applications_menu_add_app_from(self, icon, exec, name);
      }

      free(name);
      free(exec);
      free(icon);

      g_key_file_free((GKeyFile *)application->data);
    }

    g_list_free(applications);

    gtk_widget_show_all(GTK_WIDGET(self->apps_flow_box));
  }

  /*gchar **favorites = g_settings_get_strv (self->settings, "items-desktop-paths");

  for (size_t i = 0; favorites[i]; i++) {
    GKeyFile *key_file = g_key_file_new ();

    if (g_key_file_load_from_file (key_file, favorites[i], 0, NULL)) {
      gchar *name = g_key_file_get_string ((GKeyFile *)key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL);
      gchar *exec = g_key_file_get_string ((GKeyFile *)key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
      gchar *icon = g_key_file_get_string ((GKeyFile *)key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

      if (name && exec) {
        GtkWidget *app = applications_menu_render_app (self, icon, exec, name);

        gtk_grid_attach (self->favorites_grid, GTK_WIDGET (app), self->favorites_count % 4, (int)(self->favorites_count / 4), 1, 1);

        self->favorites_count++;
      }

      free (name);
      free (exec);
      free (icon);
    }

    g_key_file_free ((GKeyFile *)key_file);
  }

  gtk_widget_show_all (GTK_WIDGET (self->favorites_grid));

  free_string_list (favorites);*/

  gtk_widget_set_margin_start(GTK_WIDGET(self->scrolled_window), self->monitor_geometry.width / 4);
  gtk_widget_set_margin_end(GTK_WIDGET(self->scrolled_window), self->monitor_geometry.width / 4);
}
