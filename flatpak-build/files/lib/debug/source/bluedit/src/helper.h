/* helper.h
 *
 * Copyright 2019 Matthew Jakeman <mjakeman26@outlook.co.nz>
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

// Sets the CSS class of a widget
static void
helper_set_widget_css_class (GtkWidget *widget, gchar* value)
{
    GtkStyleContext *context;
    context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(context, value);
}

static void
helper_remove_widget_css_class (GtkWidget *widget, gchar* value)
{
    GtkStyleContext *context;
    context = gtk_widget_get_style_context(widget);
    gtk_style_context_remove_class(context, value);
}
