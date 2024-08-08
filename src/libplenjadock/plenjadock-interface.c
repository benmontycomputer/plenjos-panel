#include "plenjadock-interface.h"

#define SIGNAL_ACTIVE_CHANGE 0
#define SIGNAL_APPLICATIONS_CHANGED 1
#define SIGNAL_DESKTOP_REGISTERED 2
#define SIGNAL_PANEL_DASHBOARD_SHOW_REQUEST 3

guint interface_signals[4];

/* Private structure definition. */
/*typedef struct {
  //char *filename;
  //
  // DON'T USE THIS STRUCT

  *//* other private fields *//*
} PlenjaDockInterfacePrivate;*/

/*
 * forward definitions
 */

struct _PlenjaDockInterface
{
  GObject parent_instance;

  WnckScreen *screen;

  GList *apps;

  int current_app;

  GtkContainer *apps_container;

  GdkWindow *desktop_window;

  GSettings *settings;

  GtkIconTheme *icon_theme;
  int icon_size;

  /* Other members, including private data. */
};

G_DEFINE_TYPE (PlenjaDockInterface, plenjadock_interface, G_TYPE_OBJECT)

static void
plenjadock_interface_dispose (GObject *gobject)
{
  //PlenjaDockInterface *priv = plenjadock_interface_get_instance_private (PLENJADOCK_INTERFACE (gobject));

  /* In dispose(), you are supposed to free all types referenced from this
   * object which might themselves hold a reference to self. Generally,
   * the most simple solution is to unref all members on which you own a
   * reference.
   */

  /* dispose() might be called multiple times, so we must guard against
   * calling g_object_unref() on an invalid GObject by setting the member
   * NULL; g_clear_object() does this for us.
   */
  //g_clear_object (&priv->input_stream);

  /* Always chain up to the parent class; there is no need to check if
   * the parent class implements the dispose() virtual function: it is
   * always guaranteed to do so
   */
  G_OBJECT_CLASS (plenjadock_interface_parent_class)->dispose (gobject);
}

static void
plenjadock_interface_finalize (GObject *gobject)
{
  //PlenjaDockInterface *priv = plenjadock_interface_get_instance_private (PLENJADOCK_INTERFACE (gobject));

  //g_free (priv->filename);

  /* Always chain up to the parent class; as with dispose(), finalize()
   * is guaranteed to exist on the parent's class virtual function table
   */
  G_OBJECT_CLASS (plenjadock_interface_parent_class)->finalize (gobject);
}

static void
plenjadock_interface_class_init (PlenjaDockInterfaceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = plenjadock_interface_dispose;
  object_class->finalize = plenjadock_interface_finalize;

  interface_signals[SIGNAL_ACTIVE_CHANGE] =
  g_signal_newv ("active-window-changed",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 NULL /* closure */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 0     /* n_params */,
                 NULL  /* param_types */);

  interface_signals[SIGNAL_APPLICATIONS_CHANGED] =
  g_signal_newv ("applications-list-changed",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 NULL /* closure */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 0     /* n_params */,
                 NULL  /* param_types */);

  interface_signals[SIGNAL_DESKTOP_REGISTERED] =
  g_signal_newv ("desktop-window-registered",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 NULL /* closure */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 0     /* n_params */,
                 NULL  /* param_types */);

  interface_signals[SIGNAL_PANEL_DASHBOARD_SHOW_REQUEST] =
  g_signal_newv ("panel-dashboard-show-request",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 NULL /* closure */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 0     /* n_params */,
                 NULL  /* param_types */);
}

static void
plenjadock_interface_init (PlenjaDockInterface *self)
{
  // fix unused parameter warning
  (void)self;
  //PlenjaDockInterfacePrivate *priv = plenjadock_interface_get_instance_private (self);

  //priv->input_stream = g_object_new (VIEWER_TYPE_INPUT_STREAM, NULL);
  //priv->filename = /* would be set as a property */;
}

PlenjaDockApplication *plenjadock_interface_get_active_app (PlenjaDockInterface *self) {
  if (!self || !(self->apps)) {
    return NULL;
  }

  if (self->current_app > 0) {
    return g_list_nth (self->apps, self->current_app)->data;
  } else {
    return NULL;
  }
}

void plenjadock_interface_active_window_changed (WnckScreen          *screen,
                                      WnckWindow     *previously_active_window,
                                      PlenjaDockInterface  *self)
{
  // fix unused parameter warning
  (void)previously_active_window;

  if (screen != self->screen) {
    fprintf (stderr, "Error: active window changed, but not on current screen.\n");
    fflush (stderr);
    return;
  }

  WnckWindow *active_window = wnck_screen_get_active_window (self->screen);

  g_return_if_fail (WNCK_IS_WINDOW (active_window));

  WnckApplication *current_app = wnck_window_get_application (active_window);
  for (GList *item = self->apps; item != NULL; item = item->next) {
    if (plenjadock_application_get_app (item->data) == current_app) {
      self->current_app = g_list_index (self->apps, item->data);
      break;
    }
  }

  g_signal_emit (self, interface_signals[SIGNAL_ACTIVE_CHANGE], 0);
}

void plenjadock_interface_window_opened (WnckScreen          *screen,
                                         WnckWindow          *window,
                                         PlenjaDockInterface *self)
{
  // fix unused parameter warning
  (void)screen;

  // TODO: use this to check if a window has been opened for an app
  // that was previously set to not be displayed
  // (like a settings window for this dock)
  // This should NOT be freed
  const char *classname = wnck_window_get_class_instance_name (window);

  if (!classname) {
    return;
  }

  if (!strcmp (classname, "desktop")) {
    gulong xid = wnck_window_get_xid (window);

    GdkWindow *lookup = (GdkWindow *)gdk_x11_window_lookup_for_display (gdk_display_get_default (), xid);
    if (lookup) {
      self->desktop_window = lookup;
    } else {
      GdkWindow *t = (GdkWindow *)gdk_x11_window_foreign_new_for_display (gdk_display_get_default (), xid);
      self->desktop_window = t;
    }

    g_signal_emit (self, interface_signals[SIGNAL_DESKTOP_REGISTERED], 0);
  }
}

void free_string_list (gchar **list) {
  size_t i;
  for (i = 0; list[i]; i++) {
    free (list[i]);
  }
  // This is needed to free the NULL pointer marking the end of the list
  free (list[i]);
  free (list);
}

void plenjadock_interface_app_opened (WnckScreen          *screen,
                                      WnckApplication     *app,
                                      PlenjaDockInterface  *self)
{
  if (screen != self->screen) {
    fprintf (stderr, "Error: application opened, but not on current screen.\n");
    fflush (stderr);
    return;
  }

  // Only add if at least one window doesn't have the skip-tasklist property set to true
  GList *windows = wnck_application_get_windows (app);

  for (GList *window = windows; window != NULL; window = window->next) {
    if (WNCK_IS_WINDOW (window->data) && !wnck_window_is_skip_tasklist (WNCK_WINDOW (window->data)) && strcmp(wnck_window_get_class_group_name (window->data), "Plenjos-panel")) {
      const char *class_group_name = wnck_window_get_class_group_name (window->data);

      gulong xid = wnck_window_get_xid (window->data);

      GdkDisplay *display = gdk_display_get_default ();

      guchar *prop_text_unsigned = NULL;
      gint len_raw = 0;

      GdkWindow *window_from_xid = gdk_x11_window_foreign_new_for_display (display, xid);

      gdk_property_get (window_from_xid, gdk_atom_intern ("_GTK_APPLICATION_ID", FALSE),
                        gdk_atom_intern ("UTF8_STRING", FALSE), 0, 1024, FALSE, NULL, NULL, &len_raw, &prop_text_unsigned);

      g_object_unref (window_from_xid);

      char *prop_text = NULL;

      size_t prop_text_len = len_raw;

      if (prop_text_len > 1) {
        prop_text = calloc (prop_text_len + 1, sizeof (char));

        for (size_t i = 0; i < prop_text_len; i++) {
          prop_text[i] = ((char *)prop_text_unsigned)[i];
        }
      }

      g_free (prop_text_unsigned);

      printf ("_GTK_APPLICATION_ID: %s\n", prop_text ? prop_text : "(unable to get _GTK_APPLICATION_ID )\n");
      fflush (stdout);

      // First, we need to make sure this isn't a pinned application
      int pid = wnck_application_get_pid (app);

      // https://stackoverflow.com/questions/4143000/find-the-string-length-of-an-int
      int pid_len = (pid == 0 ? 1 : ((int)(log10(abs(pid))+1) + (pid < 0 ? 1 : 0)));

      size_t path_len = strlen("/proc//exe") + pid_len + 1;
      char *path = malloc (path_len);
      snprintf (path, path_len, "/proc/%d/exe", pid);

      char *result = malloc (PATH_MAX + 1);

      ssize_t newlen = readlink (path, result, PATH_MAX + 1);

      result[newlen] = '\0';

      free (path);

      char *desktop_path = NULL;

      // DON'T FREE OR MODIFY
      char *xdg_data_dirs_string = (char *)secure_getenv ("XDG_DATA_DIRS");

      char **xdg_data_dirs = NULL;
      size_t xdg_data_dirs_count = 0;

      if (xdg_data_dirs_string && strlen (xdg_data_dirs_string) > 0) {
        xdg_data_dirs = malloc (sizeof (char *));

        size_t data_dir_index = 0;
        size_t data_dir_current_pos = 0;

        xdg_data_dirs[0] = NULL;

        for (size_t i = 0; xdg_data_dirs_string[i]; i++) {
          char c = xdg_data_dirs_string[i];

          if (xdg_data_dirs [data_dir_index])
            xdg_data_dirs [data_dir_index] = realloc (xdg_data_dirs [data_dir_index], data_dir_current_pos + 1);
          else
            xdg_data_dirs [data_dir_index] = malloc (1);

          // TODO: check if we need to account for escape chars ( \{something} )
          if (c == ':') {
            xdg_data_dirs[data_dir_index][data_dir_current_pos] = '\0';
            data_dir_index++;
            data_dir_current_pos = 0;
            xdg_data_dirs = realloc (xdg_data_dirs, (data_dir_index + 1) * sizeof (char *));
            xdg_data_dirs[data_dir_index] = NULL;
          } else {
            xdg_data_dirs[data_dir_index][data_dir_current_pos] = c;
            data_dir_current_pos++;
          }
        }

        xdg_data_dirs[data_dir_index] = realloc (xdg_data_dirs [data_dir_index], data_dir_current_pos + 1);
        
        xdg_data_dirs[data_dir_index][data_dir_current_pos] = '\0';

        xdg_data_dirs_count = data_dir_index + 1;

        // This is needed for the free_string_list() later
        xdg_data_dirs = realloc (xdg_data_dirs, (xdg_data_dirs_count + 1) * sizeof (char *));
        xdg_data_dirs [xdg_data_dirs_count] = NULL;
      } else {
        fprintf (stderr, "XDG_DATA_DIRS not set. Can't lookup app info.\n");
        fflush (stderr);
      }

      // Now, we need to get the .desktop file path
      // TODO: use all paths in $XDG_DATA_DIRS
      if (prop_text) {
        for (size_t i = 0; i < xdg_data_dirs_count; i++) {
          size_t desktop_path_len = strlen ("/applications/.desktop") + strlen(xdg_data_dirs[i]) + strlen (prop_text) + 1;

          desktop_path = malloc (desktop_path_len);

          snprintf (desktop_path, desktop_path_len, "%s/applications/%s.desktop", xdg_data_dirs[i], prop_text);

          if (!access (desktop_path, F_OK)) {
            printf ("could access %s\n", desktop_path);
            fflush (stdout);
            break;
          } else {
            free (desktop_path);
            desktop_path = NULL;
          }
        }

        free (prop_text);
      }

      if (!desktop_path) {
        if (class_group_name) {
          for (size_t i = 0; i < xdg_data_dirs_count; i++) {
            size_t desktop_path_len = strlen ("/applications/.desktop") + strlen(xdg_data_dirs[i]) + strlen (class_group_name) + 1;

            desktop_path = malloc (desktop_path_len);

            snprintf (desktop_path, desktop_path_len, "%s/applications/%s.desktop", xdg_data_dirs[i], class_group_name);

            if (!access (desktop_path, F_OK)) {
              printf ("could access %s\n", desktop_path);
              fflush (stdout);
              break;
            } else {
              free (desktop_path);
              desktop_path = NULL;
            }
          }
        }
      }

      if (!desktop_path) {
        const char *class_instance_name = wnck_window_get_class_instance_name (window->data);
        if (class_instance_name) {
          for (size_t i = 0; i < xdg_data_dirs_count; i++) {
            size_t desktop_path_len = strlen ("/applications/.desktop") + strlen(xdg_data_dirs[i]) + strlen (class_instance_name) + 1;

            desktop_path = malloc (desktop_path_len);

            snprintf (desktop_path, desktop_path_len, "%s/applications/%s.desktop", xdg_data_dirs[i], class_instance_name);
            printf ("%s", desktop_path);
            fflush(stdout);

            if (!access (desktop_path, F_OK)) {
              printf ("could access %s\n", desktop_path);
              fflush (stdout);
              break;
            } else {
              free (desktop_path);
              desktop_path = NULL;
            }
          }
        }
      }

      for (GList *item = self->apps; item; item = item->next) {
        char *exec_path = plenjadock_application_get_exec_path (item->data);
        char *app_desktop_path = plenjadock_application_get_desktop_path (item->data);

        char *startup_class_name = NULL;

        if (app_desktop_path) {
          GKeyFile *key_file = g_key_file_new ();

          if (g_key_file_load_from_file (key_file, app_desktop_path, G_KEY_FILE_NONE, NULL)) {
            startup_class_name = g_key_file_get_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_STARTUP_WM_CLASS, NULL);
          }

          g_key_file_free (key_file);
        }

        printf ("test %s %s\n", startup_class_name, class_group_name);
        fflush (stdout);
        if ((exec_path && result && !strcmp (exec_path, result))
            || (desktop_path && app_desktop_path && !strcmp (desktop_path, app_desktop_path))
            || (startup_class_name && class_group_name && !strcmp (startup_class_name, class_group_name))) {
          plenjadock_application_set_app (item->data, app);
          free (result);
          if (exec_path) {
            free (exec_path);
          }
          if (app_desktop_path) {
            free (app_desktop_path);
          }
          if (desktop_path) {
            free (desktop_path);
          }
          if (startup_class_name) {
            free (startup_class_name);
          }
          if (xdg_data_dirs) {
            free_string_list (xdg_data_dirs);
          }
          return;
        }
        if (exec_path) {
          free (exec_path);
        }
        if (app_desktop_path) {
          free (app_desktop_path);
        }
        if (startup_class_name) {
          free (startup_class_name);
        }
      }

      if (xdg_data_dirs) {
        free_string_list (xdg_data_dirs);
      }

      free (result);

      PlenjaDockApplication *new_app = g_object_new (PLENJADOCK_TYPE_APPLICATION, NULL);

      plenjadock_application_set_app (new_app, app);

      plenjadock_application_set_props (new_app, NULL, NULL, NULL, NULL, FALSE, self->icon_theme, self->icon_size, gtk_widget_get_scale_factor (GTK_WIDGET (self->apps_container)));

      if (desktop_path) {
        GKeyFile *key_file = g_key_file_new ();

        if (g_key_file_load_from_file (key_file, desktop_path, G_KEY_FILE_NONE, NULL)) {
          // Don't free these
          gchar *name = g_key_file_get_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL);
          gchar *exec = g_key_file_get_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
          gchar *icon = g_key_file_get_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

          plenjadock_application_set_props (new_app, icon, name, exec, desktop_path, FALSE, self->icon_theme, self->icon_size, gtk_widget_get_scale_factor (GTK_WIDGET (self->apps_container)));

          g_free (name);
          g_free (exec);
          g_free (icon);
        }

        g_key_file_free (key_file);

        free (desktop_path);
      }

      plenjadock_interface_add_app (self, new_app, -1, TRUE);

      break;
    }
  }
}

void plenjadock_interface_app_closed (WnckScreen          *screen,
                                      WnckApplication     *app,
                                      PlenjaDockInterface  *self)
{
  if (screen != self->screen) {
    fprintf (stderr, "Error: application closed, but not on current screen.\n");
    fflush (stderr);
    return;
  }

  for (GList *item = self->apps; item != NULL; item = item->next) {
    if (plenjadock_application_get_app (item->data) == app) {
      if (plenjadock_application_get_pinned (item->data)) {
        plenjadock_application_set_app (item->data, NULL);
        break;
      }

      plenjadock_application_unrender (item->data);

      self->apps = g_list_remove (self->apps, item->data);
      // TODO: check if item->data needs to be freed or destroyed or smth
      // (potential memory leak?)
      plenjadock_interface_active_window_changed (self->screen, NULL, self);
      break;
    }
  }
}

void plenjadock_interface_set_apps_container (PlenjaDockInterface *self,
                                        GtkContainer           *container)
{
  self->apps_container = container;
}

void plenjadock_interface_launch_dashboard (GtkWidget             *widget,
                                             GdkEventButton        *event,
                                             PlenjaDockInterface *self)
{
  // fix unused parameter warning
  (void)widget;
  printf("test2\n");
  fflush(stdout);
  if (event->button == GDK_BUTTON_PRIMARY) {
    g_signal_emit (self, interface_signals[SIGNAL_PANEL_DASHBOARD_SHOW_REQUEST], 0);
  }
}

int plenjadock_interface_start (PlenjaDockInterface *self) {
  self->icon_theme = gtk_icon_theme_get_default ();
  self->icon_size = 52;

  self->screen = wnck_screen_get_default();

  g_signal_connect (self->screen, "application-opened", G_CALLBACK (plenjadock_interface_app_opened), self);
  g_signal_connect (self->screen, "application-closed", G_CALLBACK (plenjadock_interface_app_closed), self);
  g_signal_connect (self->screen, "active-window-changed", G_CALLBACK (plenjadock_interface_active_window_changed), self);
  g_signal_connect (self->screen, "window-opened", G_CALLBACK (plenjadock_interface_window_opened), self);

  gint scale_factor = gtk_widget_get_scale_factor (GTK_WIDGET (self->apps_container));
  wnck_set_default_icon_size(52 * scale_factor);

  PlenjaDockApplication *dashboard_shortcut = g_object_new (PLENJADOCK_TYPE_APPLICATION, NULL);

  plenjadock_application_set_props (dashboard_shortcut, "view-app-grid", "Dashboard", NULL, NULL, TRUE, self->icon_theme, self->icon_size, gtk_widget_get_scale_factor (GTK_WIDGET (self->apps_container)));

  GtkWidget *dashboard_shortcut_rendered = plenjadock_interface_add_app (self, dashboard_shortcut, 0, FALSE);
  // TODO: launch dashboard
  g_signal_connect (GTK_EVENT_BOX (dashboard_shortcut_rendered), "button-release-event", G_CALLBACK (plenjadock_interface_launch_dashboard), self);

  if (!self->settings) {
    self->settings = g_settings_new ("com.plenjos.Panel");
  }

  gchar **names = g_settings_get_strv (self->settings, "items-names");
  gchar **paths = g_settings_get_strv (self->settings, "items-paths");
  gchar **icons = g_settings_get_strv (self->settings, "items-icons");
  gchar **desktop_files = g_settings_get_strv (self->settings, "items-desktop-paths");

  for (int i = 0; names[i]; i++) {
    PlenjaDockApplication *new_app = g_object_new (PLENJADOCK_TYPE_APPLICATION, NULL);

    plenjadock_application_set_props (new_app, icons[i], names[i], paths[i], desktop_files[i], TRUE, self->icon_theme, self->icon_size, gtk_widget_get_scale_factor (GTK_WIDGET (self->apps_container)));
    plenjadock_application_set_pinned (new_app, TRUE);

    plenjadock_interface_add_app (self, new_app, i+1, TRUE);
  }

  free_string_list (names);
  free_string_list (paths);
  free_string_list (icons);
  free_string_list (desktop_files);

  return 0;
}

#define PLACEHOLDER_NAME "[placeholder]"
#define PLACEHOLDER_ICON "[placeholder]"

void pin_app_request (PlenjaDockApplication *app,
                      PlenjaDockInterface   *self)
{
  // we just gotta pin this to the end of the list

  gchar **names = g_settings_get_strv (self->settings, "items-names");
  gchar **paths = g_settings_get_strv (self->settings, "items-paths");
  gchar **icons = g_settings_get_strv (self->settings, "items-icons");
  gchar **desktop_files = g_settings_get_strv (self->settings, "items-desktop-paths");

  size_t old_pinned_count;

  for (old_pinned_count = 0; desktop_files[old_pinned_count]; old_pinned_count++);

  names = realloc (names, (old_pinned_count + 2) * sizeof (gchar *));
  names[old_pinned_count + 1] = NULL;
  size_t placeholder_name_len = strlen (PLACEHOLDER_NAME) + 1;
  names[old_pinned_count] = malloc (placeholder_name_len);
  snprintf (names[old_pinned_count], placeholder_name_len, PLACEHOLDER_NAME);

  // TODO: get some sort of name to put here

  paths = realloc (paths, (old_pinned_count + 2) * sizeof (gchar *));
  paths[old_pinned_count + 1] = NULL;
  paths[old_pinned_count] = plenjadock_application_get_exec_path (app);

  desktop_files = realloc (desktop_files, (old_pinned_count + 2) * sizeof (gchar *));
  desktop_files[old_pinned_count + 1] = NULL;
  desktop_files[old_pinned_count] = plenjadock_application_get_desktop_path (app);

  char *icon_name = NULL;

  GKeyFile *key_file = g_key_file_new ();
  if (desktop_files[old_pinned_count] && g_key_file_load_from_file (key_file, desktop_files[old_pinned_count], G_KEY_FILE_NONE, NULL)) {
    icon_name = g_key_file_get_string (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL);
  }

  g_key_file_free (key_file);

  icons = realloc (icons, (old_pinned_count + 2) * sizeof (gchar *));
  icons[old_pinned_count + 1] = NULL;
  if (icon_name) {
    icons[old_pinned_count] = icon_name;
  } else {
    size_t placeholder_icon_len = strlen (PLACEHOLDER_ICON) + 1;
    icons[old_pinned_count] = malloc (placeholder_icon_len);
    snprintf (icons[old_pinned_count], placeholder_icon_len, PLACEHOLDER_ICON);
  }

  g_settings_set_strv (self->settings, "items-names", names);
  g_settings_set_strv (self->settings, "items-paths", paths);
  g_settings_set_strv (self->settings, "items-icons", icons);
  g_settings_set_strv (self->settings, "items-desktop-paths", desktop_files);

  free_string_list (names);
  free_string_list (paths);
  free_string_list (icons);
  free_string_list (desktop_files);

  plenjadock_application_set_pinned (app, TRUE);
}

void unpin_app_request (PlenjaDockApplication *app,
                        PlenjaDockInterface   *self)
{
  char *desktop_path = plenjadock_application_get_desktop_path (app);

  if (!desktop_path) {
    printf ("Error: desktop_path is null; can't unpin app.\n");
    fflush (stdout);
    return;
  }

  gchar **desktop_files = g_settings_get_strv (self->settings, "items-desktop-paths");

  for (size_t i = 0; desktop_files[i]; i++) {
    if (!strcmp (desktop_files[i], desktop_path)) {
      gchar **names = g_settings_get_strv (self->settings, "items-names");
      gchar **paths = g_settings_get_strv (self->settings, "items-paths");
      gchar **icons = g_settings_get_strv (self->settings, "items-icons");

      free (names[i]);
      free (paths[i]);
      free (icons[i]);
      free (desktop_files[i]);

      // calling realloc() to shrink the lists is actually just a waste of time because the list is about to get freed anyways
      // and there is already a NULL at the end so it isn't needed to make the code work.

      for (size_t j = i; names[j]; j++) {
        names[j] = names[j+1];
      }

      for (size_t j = i; paths[j]; j++) {
        paths[j] = paths[j+1];
      }

      for (size_t j = i; icons[j]; j++) {
        icons[j] = icons[j+1];
      }

      for (size_t j = i; desktop_files[j]; j++) {
        desktop_files[j] = desktop_files[j+1];
      }

      // the "... discards qualifiers in nested pointer types" thing can be ignored safely here.
      g_settings_set_strv (self->settings, "items-names", names);
      g_settings_set_strv (self->settings, "items-paths", paths);
      g_settings_set_strv (self->settings, "items-icons", icons);
      g_settings_set_strv (self->settings, "items-desktop-paths", desktop_files);

      free_string_list (names);
      free_string_list (paths);
      free_string_list (icons);

      plenjadock_application_set_pinned (app, FALSE);

      break;
    }
  }

  if (desktop_path) {
    free (desktop_path);
  }

  if (desktop_files) {
    free_string_list (desktop_files);
  }
}

static GtkTargetEntry entries[] = {
   { "GTK_EVENT_BOX", GTK_TARGET_SAME_APP, 0 }
};

void drag_data_get (GtkWidget           *widget,
                    GdkDragContext      *context,
                    GtkSelectionData    *selection_data,
                    guint                info,
                    guint                time_uint,
                    gpointer user_data)
{
  // fix unused parameter warnings
  (void)context;
  (void)info;
  (void)time_uint;
  (void)user_data;

  gtk_selection_data_set (selection_data,
        gdk_atom_intern_static_string ("GTK_EVENT_BOX"),
        32,
        (const guchar *)&widget,
        sizeof (gpointer));
}

void drag_data_received (GtkWidget           *target,
                         GdkDragContext      *context,
                         gint                 x,
                         gint                 y,
                         GtkSelectionData    *selection_data,
                         guint                info,
                         guint                time_uint,
                         PlenjaDockInterface *self)
{
  // fix unused parameter warnings
  (void)context;
  (void)x;
  (void)y;
  (void)info;
  (void)time_uint;
  GtkWidget *source = *(gpointer*)gtk_selection_data_get_data (selection_data);

  if (source == target)
    return;

  GList *children = gtk_container_get_children (GTK_CONTAINER (gtk_widget_get_parent (target)));

  int position = -1;

  int i = 0;

  for (GList *child = children; child; child = child->next) {
    if (GTK_WIDGET (child->data) == target) {
      position = i;
      break;
    }
    i++;
  }

  // Don't need to free the children themselves; just the list.
  g_list_free (children);

  gtk_box_reorder_child (GTK_BOX (self->apps_container), source, position);
  // TODO: keep pinned itens and unpinned items separate; save arrangement of pinned apps
}

GtkWidget *plenjadock_interface_add_app (PlenjaDockInterface   *self,
                                  PlenjaDockApplication *app,
                                  int position,
                                  gboolean can_pin_unpin)
{
  if (can_pin_unpin) {
    g_signal_connect (app, "application-pin-request", G_CALLBACK (pin_app_request), self);
    g_signal_connect (app, "application-unpin-request", G_CALLBACK (unpin_app_request), self);
  }

  self->apps = g_list_insert (self->apps, app, position);

  GtkWidget *rendered = plenjadock_application_render (app);

  gtk_drag_source_set (rendered, GDK_BUTTON1_MASK, entries, 1, GDK_ACTION_MOVE);
  gtk_drag_dest_set (rendered, GTK_DEST_DEFAULT_ALL, entries, 1, GDK_ACTION_MOVE);

  g_signal_connect (rendered, "drag-data-get", G_CALLBACK (drag_data_get), NULL);
  g_signal_connect (rendered, "drag-data-received", G_CALLBACK (drag_data_received), self);


  gtk_container_add (self->apps_container, rendered);
  gtk_widget_show_all (GTK_WIDGET (self->apps_container));

  return rendered;
}

GdkWindow *plenjadock_interface_get_desktop_window (PlenjaDockInterface *self)
{
  return self->desktop_window;
}
