/* bl-view.h
 *
 * Copyright 2019 Matthew Jakeman <unknown@domain.org>
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
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BL_TYPE_VIEW (bl_view_get_type())
G_DECLARE_DERIVABLE_TYPE (BlView, bl_view, BL, VIEW, GtkBin)

struct _BlViewClass
{
    GtkBinClass parent_class;
};

void bl_view_set_contents (BlView *self, GtkWidget *widget);
void bl_view_set_menu (BlView *self, GtkWidget *widget, gboolean expand);
void bl_view_set_decoration_start (BlView *self, GtkWidget *widget);
void bl_view_set_decoration_end (BlView *self, GtkWidget *widget);
void bl_view_set_decoration_style (BlView *self, gchar *css_class);
void bl_view_remove_decoration_style (BlView *self, gchar *css_class);

G_END_DECLS
