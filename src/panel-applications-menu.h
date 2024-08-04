/* panel-applications-menu.h
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

#pragma once

#define WNCK_I_KNOW_THIS_IS_UNSTABLE

#include <gtk/gtk.h>
#include <libhandy-1/handy.h>
#include <math.h>
#include <gdk/gdkx.h>
#include <ctype.h>

#include <libwnck/libwnck.h>

G_BEGIN_DECLS

#define PANEL_TYPE_APPLICATIONS_MENU (panel_applications_menu_get_type())

G_DECLARE_FINAL_TYPE (PanelApplicationsMenu, panel_applications_menu, PANEL, APPLICATIONS_MENU, HdyApplicationWindow)

G_END_DECLS

void panel_applications_menu_set_bg (PanelApplicationsMenu *self, GdkPixbuf *bg);
void hide_applications_menu (PanelApplicationsMenu *self);
void show_applications_menu (PanelApplicationsMenu *self);
