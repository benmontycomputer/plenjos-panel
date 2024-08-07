#include "plenjadock-application.h"

#define SIGNAL_PIN 0
#define SIGNAL_UNPIN 1

guint application_signals[2];

/* Private structure definition. */
/*typedef struct {
  //char *filename;
  //
  // DON'T USE THIS STRUCT

  *//* other private fields *//*
} PlenjaDockApplicationPrivate;*/

/*
 * forward definitions
 */

void free_string_list (gchar **list);

struct _PlenjaDockApplication
{
  GObject parent_instance;

  WnckApplication *app;

  int current_window;

  GtkEventBox *event_box;

  GtkFixed *fixed;

  GdkPixbuf *indicator_pixbuf;

  GtkImage *icon;

  GtkImage *indicator;

  GtkImage *shadow;

  gchar *icon_name;
  gchar *exec_path;

  GtkIconTheme *icon_theme;
  int icon_size;
  int scale;

  gchar *desktop_path;

  gboolean pinned;

  /* Other members, including private data. */
};

G_DEFINE_TYPE (PlenjaDockApplication, plenjadock_application, G_TYPE_OBJECT)

static void
plenjadock_application_dispose (GObject *gobject)
{
  //PlenjaDockApplication *priv = plenjadock_application_get_instance_private (PLENJADOCK_APPLICATION (gobject));

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
  G_OBJECT_CLASS (plenjadock_application_parent_class)->dispose (gobject);
}

static void
plenjadock_application_finalize (GObject *gobject)
{
  //PlenjaDockApplication *priv = plenjadock_application_get_instance_private (PLENJADOCK_APPLICATION (gobject));

  //g_free (priv->filename);

  /* Always chain up to the parent class; as with dispose(), finalize()
   * is guaranteed to exist on the parent's class virtual function table
   */
  G_OBJECT_CLASS (plenjadock_application_parent_class)->finalize (gobject);
}

static void
plenjadock_application_class_init (PlenjaDockApplicationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = plenjadock_application_dispose;
  object_class->finalize = plenjadock_application_finalize;

  application_signals[SIGNAL_PIN] =
  g_signal_newv ("application-pin-request",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 NULL /* closure */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 0     /* n_params */,
                 NULL  /* param_types */);

  application_signals[SIGNAL_UNPIN] =
  g_signal_newv ("application-unpin-request",
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
plenjadock_application_init (PlenjaDockApplication *self)
{
  //PlenjaDockApplicationPrivate *priv = plenjadock_Application_get_instance_private (self);

  //priv->input_stream = g_object_new (VIEWER_TYPE_INPUT_STREAM, NULL);
  //priv->filename = /* would be set as a property */;

  self->current_window = 0;
}

int plenjadock_application_get_contains_window (PlenjaDockApplication *self,
                                                 WnckWindow            *window)
{
  GList *windows = wnck_application_get_windows (self->app);
  int result = g_list_index (windows, window);
  g_list_free (windows);
  return result;
}

void plenjadock_application_set_app (PlenjaDockApplication *self,
                                     WnckApplication       *app)
{
  self->app = app;
  plenjadock_application_update_indicator (self);
}

WnckApplication *plenjadock_application_get_app (PlenjaDockApplication *self)
{
  return self->app;
}

void menu_action_activate (GtkMenuItem *menu_item,
                           char        *action)
{
  // fix unused parameter warning
  (void)menu_item;

  g_return_if_fail (action);
  size_t new_exec_len = strlen (action) + 3;
  char *new_exec = malloc (new_exec_len);
  snprintf (new_exec, new_exec_len, "%s &", action);
  system (new_exec);
  free (new_exec);
  free (action);
}

void pin_item_activate (GtkMenuItem           *menu_item,
                        PlenjaDockApplication *self)
{
  // fix unused parameter warning
  (void)menu_item;

  g_return_if_fail (self);

  g_signal_emit (self, self->pinned ? application_signals[SIGNAL_UNPIN] : application_signals[SIGNAL_PIN], 0);
}

void menu_window_show_activate (GtkMenuItem *menu_item,
                                WnckWindow  *window)
{
  // fix unused parameter warning
  (void)menu_item;

  wnck_window_activate (window, gdk_x11_get_server_time (gdk_get_default_root_window ()));
}

gboolean plenjadock_application_on_click (GtkWidget             *widget,
                                          GdkEventButton        *event,
                                          PlenjaDockApplication *self)
{
  if (event->button == GDK_BUTTON_SECONDARY) {
    gtk_widget_unset_state_flags (widget, GTK_STATE_FLAG_PRELIGHT);
    char *desktop_path = plenjadock_application_get_desktop_path (self);
    printf ("desktop: %s\n", desktop_path);
    fflush (stdout);

    GtkMenu *menu = GTK_MENU (gtk_menu_new ());

    // The menu should have:
    // - open windows (potentially with close button next to label)
    // - "New Window" button
    // - action buttons specified by app
    // - Pin/Unpin button
    // - "Close All" button

    int current_menu_item = 0;
    GtkMenuItem *open_windows_label = GTK_MENU_ITEM (gtk_menu_item_new_with_label ("Open Windows:"));
    gtk_widget_set_name (GTK_WIDGET (open_windows_label), "dockmenulabelitem");
    gtk_menu_attach (menu, GTK_WIDGET (open_windows_label), 0, 1, current_menu_item, current_menu_item + 1);

    current_menu_item++;

    if (WNCK_IS_APPLICATION (self->app)) {
      GList *windows = wnck_application_get_windows (self->app);

      for (GList *window = windows; window; window = window->next) {
        const char *name = wnck_window_get_name (window->data);

        GtkMenuItem *menu_item = GTK_MENU_ITEM (gtk_menu_item_new ());
        gtk_widget_set_name (GTK_WIDGET (menu_item), "dockmenuitem");

        if (strlen (name) <= 20) {
          gtk_menu_item_set_label (menu_item, name);
        } else {
          char *name_clipped = malloc (21);
          snprintf (name_clipped, 21, "%s", name);
          name_clipped[17] = '.';
          name_clipped[18] = '.';
          name_clipped[19] = '.';
          gtk_menu_item_set_label (menu_item, name_clipped);
          free (name_clipped);
        }

        g_signal_connect (menu_item, "activate", G_CALLBACK (menu_window_show_activate), window->data);

        gtk_menu_attach (menu, GTK_WIDGET (menu_item), 0, 1, current_menu_item, current_menu_item + 1);

        current_menu_item++;
      }
    }

    gtk_menu_attach (menu, gtk_separator_menu_item_new (), 0, 1, current_menu_item, current_menu_item + 1);

    current_menu_item++;

    gboolean needs_new_window_item = TRUE;

    if (desktop_path) {
      GKeyFile *key_file = g_key_file_new ();

      if (g_key_file_load_from_file (key_file, desktop_path, G_KEY_FILE_NONE, NULL)) {
        size_t actions_list_len;
        char **actions_list = g_key_file_get_string_list (key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ACTIONS, &actions_list_len, NULL);

        if (actions_list && actions_list_len) {
          for (size_t i = 0; i < actions_list_len; i++) {
            printf ("action key name: %s\n", actions_list[i]);

            if (!strcmp ("new-window", actions_list[i])) {
              needs_new_window_item = FALSE;
            }

            char *action_name = NULL;
            char *action_exec = NULL;

            size_t key_group_full_len = strlen ("Desktop Action") + strlen (actions_list[i]) +
            /* this is 2; 1 for the null terminator and 1 for the space */ 2;
            char *key_group_full = malloc (key_group_full_len);

            snprintf (key_group_full, key_group_full_len, "Desktop Action %s", actions_list[i]);

            action_name = g_key_file_get_string (key_file, key_group_full, G_KEY_FILE_DESKTOP_KEY_NAME, NULL);
            action_exec = g_key_file_get_string (key_file, key_group_full, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);

            free (key_group_full);

            GtkMenuItem *menu_item = GTK_MENU_ITEM (gtk_menu_item_new ());

            gtk_menu_item_set_label (menu_item, action_name);
            g_signal_connect (menu_item, "activate", G_CALLBACK (menu_action_activate), g_strdup (action_exec));
            if (action_name)
              free (action_name);
            if (action_exec)
              free (action_exec);

            gtk_widget_set_name (GTK_WIDGET (menu_item), "dockmenuitem");
            gtk_menu_attach (menu, GTK_WIDGET (menu_item), 0, 1, current_menu_item, current_menu_item + 1);

            current_menu_item++;
          }
        }

        if (actions_list) {
          free_string_list (actions_list);
        }
      }

      g_key_file_free (key_file);
    }

    if (needs_new_window_item) {
      GtkMenuItem *menu_item = GTK_MENU_ITEM (gtk_menu_item_new ());

      gtk_menu_item_set_label (menu_item, "New Window");
      g_signal_connect (menu_item, "activate", G_CALLBACK (menu_action_activate), g_strdup (self->exec_path));

      gtk_widget_set_name (GTK_WIDGET (menu_item), "dockmenuitem");
      gtk_menu_attach (menu, GTK_WIDGET (menu_item), 0, 1, current_menu_item, current_menu_item + 1);

      current_menu_item++;
    }

    GtkMenuItem *pin_item = GTK_MENU_ITEM (gtk_menu_item_new ());

    if (self->pinned) {
      gtk_menu_item_set_label (pin_item, "Unpin");
      g_signal_connect (pin_item, "activate", G_CALLBACK (pin_item_activate), self);
    } else {
      gtk_menu_item_set_label (pin_item, "Pin");
      g_signal_connect (pin_item, "activate", G_CALLBACK (pin_item_activate), self);
    }

    gtk_widget_set_name (GTK_WIDGET (pin_item), "dockmenuitem");
    gtk_menu_attach (menu, GTK_WIDGET (pin_item), 0, 1, current_menu_item, current_menu_item + 1);

    current_menu_item++;

    gtk_widget_show_all (GTK_WIDGET (menu));

    GdkRectangle *rect = calloc (1, sizeof (GdkRectangle));
    rect->x = 28;
    rect->y = 0;
    rect->width = 0;

    gtk_widget_set_name (GTK_WIDGET (menu), "dockmenu");

    gtk_menu_popup_at_rect (menu, gtk_widget_get_window (widget), rect, GDK_GRAVITY_NORTH, GDK_GRAVITY_SOUTH, NULL);

    free (rect);

    free (desktop_path);
  }
  else if (event->button == GDK_BUTTON_PRIMARY) {
    if (!self->app) {
      // launch the app

      if (self->exec_path) {
        size_t new_exec_len = strlen (self->exec_path) + 3;
        char *new_exec = malloc (new_exec_len);
        snprintf (new_exec, new_exec_len, "%s &", self->exec_path);
        system (new_exec);
        free (new_exec);
      }
      return FALSE;
    }

    // DON'T FREE
    GList *windows = wnck_application_get_windows (self->app);

    // TODO: figure out: does this need to be freed?
    GdkWindow *root = gdk_get_default_root_window ();

    g_return_val_if_fail (windows, FALSE);

    if (g_list_length (windows) > 1) {
      if (self->current_window >= 0 && (int)g_list_length (windows) > self->current_window) {
        GList *current_win = g_list_nth (windows, (guint)(self->current_window));

        if (WNCK_IS_WINDOW (current_win->data) && !wnck_window_is_active(current_win->data)) {
          wnck_window_activate (current_win->data, gdk_x11_get_server_time (root));
        } else {
          if (current_win->next) {
            self->current_window++;
            wnck_window_activate (current_win->next->data, gdk_x11_get_server_time (root));
          } else {
            self->current_window = 0;
            wnck_window_activate (g_list_first(windows)->data, gdk_x11_get_server_time (root));
          }
        }
      } else {
        if (WNCK_IS_WINDOW (windows->data)) {
          wnck_window_activate (windows->data, gdk_x11_get_server_time (root));
        }
      }
    } else {
      WnckWindow *current_win = windows->data;

      if (WNCK_IS_WINDOW (current_win)) {
        if (wnck_window_is_active (current_win)) {
          wnck_window_minimize (current_win);
        } else {
          wnck_window_activate (current_win, gdk_x11_get_server_time (root));
        }
      }
    }
  }

  return FALSE;
}

void plenjadock_application_unrender (PlenjaDockApplication *self) {
  // TODO: check if this actually works

  if (GTK_IS_WIDGET (self->icon)) {
    gtk_widget_destroy (GTK_WIDGET (self->icon));
  }
  self->icon = NULL;

  if (GTK_IS_WIDGET (self->shadow)) {
    gtk_widget_destroy (GTK_WIDGET (self->shadow));
  }
  self->shadow = NULL;

  if (GDK_IS_PIXBUF (self->indicator_pixbuf)) {
    g_object_unref (self->indicator_pixbuf);
  }
  self->indicator_pixbuf = NULL;

  if (GTK_IS_WIDGET (self->indicator)) {
    gtk_widget_destroy (GTK_WIDGET (self->indicator));
  }
  self->indicator = NULL;

  if (GTK_IS_WIDGET (self->event_box)) {
    gtk_widget_destroy (GTK_WIDGET (self->event_box));
  }
  self->event_box = NULL;
}

gboolean hover_start (GtkWidget        *widget,
                      GdkEventCrossing  *event,
                      gpointer          user_data)
{
  // fix unused parameter warning
  (void)user_data;
  if (event->mode != GDK_CROSSING_NORMAL) {
    return FALSE;
  }

  gtk_widget_set_state_flags (widget, GTK_STATE_FLAG_PRELIGHT, FALSE);

  return FALSE;
}

gboolean hover_end (GtkWidget        *widget,
                    GdkEventCrossing  *event,
                    gpointer          user_data)
{
  // fix unused parameter warning
  (void)user_data;
  if (event->mode != GDK_CROSSING_NORMAL) {
    return FALSE;
  }

  gtk_widget_unset_state_flags (widget, GTK_STATE_FLAG_PRELIGHT);

  return FALSE;
}

void plenjadock_application_update_indicator (PlenjaDockApplication *self) {
  if (plenjadock_application_get_is_open (self))
    gtk_widget_set_name (GTK_WIDGET (self->indicator), "dockindicatoractive");
  else
    gtk_widget_set_name (GTK_WIDGET (self->indicator), "dockindicatorinactive");
}

GtkWidget *plenjadock_application_render (PlenjaDockApplication *self) {
  plenjadock_application_unrender (self);

  self->event_box = GTK_EVENT_BOX (gtk_event_box_new ());

  gtk_widget_set_name (GTK_WIDGET (self->event_box), "dockitem");

  gtk_widget_set_size_request (GTK_WIDGET (self->event_box), 60, 64);

  g_signal_connect (self->event_box, "enter-notify-event", G_CALLBACK (hover_start), NULL);
  g_signal_connect (self->event_box, "leave-notify-event", G_CALLBACK (hover_end), NULL);

  g_signal_connect (self->event_box, "button-release-event", G_CALLBACK (plenjadock_application_on_click), self);

  GdkPixbuf *icon_unscaled;
  gboolean free_icon_unscaled = TRUE;

  gboolean fallback_icon = FALSE;

  if (self->icon_name)
    icon_unscaled = gtk_icon_theme_load_icon_for_scale (self->icon_theme, self->icon_name, self->icon_size, self->scale, 0, NULL);
  else if (WNCK_IS_APPLICATION (self->app) && !wnck_application_get_icon_is_fallback (self->app)) {
    // I'm pretty sure this shouldn't be g_object_unref()'d after
    icon_unscaled = wnck_application_get_icon (self->app);
    free_icon_unscaled = FALSE;
  } else {
    icon_unscaled = gtk_icon_theme_load_icon_for_scale (self->icon_theme, "application-x-executable", self->icon_size, self->scale, 0, NULL);
    fallback_icon = TRUE;
  }

  if (!icon_unscaled) {
    icon_unscaled = gtk_icon_theme_load_icon_for_scale (self->icon_theme, "application-x-executable", self->icon_size, self->scale, 0, NULL);
    fallback_icon = TRUE;
  }

  self->icon = GTK_IMAGE (gtk_image_new ());

  cairo_surface_t *s = gdk_cairo_surface_create_from_pixbuf(icon_unscaled, 0, gtk_widget_get_window(GTK_WIDGET (self->icon)));

  if(s) {
      gtk_image_set_from_surface(self->icon, s);
      cairo_surface_destroy(s);
  }

  if (free_icon_unscaled) {
    g_object_unref (icon_unscaled);
  }

  self->shadow = GTK_IMAGE (gtk_image_new ());
  if (!fallback_icon)
    gtk_widget_set_name (GTK_WIDGET (self->shadow), "dockitemshadow");
  gtk_widget_set_size_request (GTK_WIDGET (self->shadow), 46, 46);

  GdkPixbuf *indicator_unscaled = gdk_pixbuf_new_from_file ("../data/icons/hicolor/scalable/status/plenjadock-app-open-indicator.svg", NULL);

  self->indicator_pixbuf = gdk_pixbuf_scale_simple (indicator_unscaled, 4, 4, GDK_INTERP_BILINEAR);

  g_object_unref (indicator_unscaled);

  self->indicator = GTK_IMAGE (gtk_image_new_from_pixbuf (self->indicator_pixbuf));

  gtk_widget_set_size_request (GTK_WIDGET (self->icon), 56, 56);

  gtk_widget_set_hexpand (GTK_WIDGET (self->icon), FALSE);
  gtk_widget_set_name (GTK_WIDGET (self->icon), "dockitem");

  plenjadock_application_update_indicator (self);

  self->fixed = GTK_FIXED (gtk_fixed_new ());

  gtk_fixed_put (self->fixed, GTK_WIDGET (self->shadow), 7, 8);
  gtk_fixed_put (self->fixed, GTK_WIDGET (self->icon), 2, 3);
  gtk_fixed_put (self->fixed, GTK_WIDGET (self->indicator), 28, 57);

  gtk_container_add (GTK_CONTAINER (self->event_box), GTK_WIDGET (self->fixed));

  return GTK_WIDGET (self->event_box);
}

void plenjadock_application_set_props (PlenjaDockApplication *self,
                                       gchar                 *icon_name,
                                       gchar                 *app_name,
                                       gchar                 *path,
                                       gchar                 *desktop_path,
                                       gboolean               pinned,
                                       GtkIconTheme          *icon_theme,
                                       int                    icon_size,
                                       int                    scale)
{
  // TODO: consider actually doing something with app_name
  // fix unused parameter warning
  (void)app_name;
  self->icon_name = g_strdup (icon_name);
  self->exec_path = g_strdup (path);
  self->desktop_path = g_strdup (desktop_path);

  self->icon_theme = icon_theme;
  self->icon_size = icon_size;
  self->scale = scale;

  self->pinned = pinned;
}

char *plenjadock_application_get_exec_path (PlenjaDockApplication *self) {
  return g_strdup (self->exec_path);
}

char *plenjadock_application_get_desktop_path (PlenjaDockApplication *self) {
  return g_strdup (self->desktop_path);
}

void plenjadock_application_set_desktop_path (PlenjaDockApplication *self, gchar *desktop_path) {
  self->desktop_path = g_strdup (desktop_path);
}

gboolean plenjadock_application_get_pinned (PlenjaDockApplication *self) {
  return self->pinned;
}

void plenjadock_application_set_pinned (PlenjaDockApplication *self, gboolean pinned) {
  self->pinned = pinned;
}

gboolean plenjadock_application_get_is_open (PlenjaDockApplication *self) {
  return self->app != NULL;
}

