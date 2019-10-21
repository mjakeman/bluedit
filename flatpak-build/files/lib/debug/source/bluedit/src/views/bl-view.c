/* bl-view.c
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

#include "bl-view.h"
#include "helper.h"

typedef struct
{
    GtkWidget *decoration;
    GtkWidget *main_box;

    guint spacetype;

} BlViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (BlView, bl_view, GTK_TYPE_BIN)

/**
 * bl_view_new:
 *
 * Create a new #BlView.
 *
 * Returns: (transfer full): a newly created #BlView
 */
BlView *
bl_view_new (void)
{
	g_debug("Boo!");
    return g_object_new (BL_TYPE_VIEW, NULL);
}

static void
bl_view_finalize (GObject *object)
{
    G_OBJECT_CLASS (bl_view_parent_class)->finalize (object);
}

static void
bl_view_class_init (BlViewClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = bl_view_finalize;
}

void bl_view_set_contents (BlView *self, GtkWidget *widget)
{
    BlViewPrivate *priv = bl_view_get_instance_private (self);
    gtk_box_pack_end (GTK_BOX (priv->main_box), widget, TRUE, TRUE, 0);
}

void bl_view_set_decoration_start (BlView *self, GtkWidget *widget)
{
    BlViewPrivate *priv = bl_view_get_instance_private (self);
    gtk_box_pack_start (GTK_BOX (priv->decoration), widget, FALSE, FALSE, 0);
    gtk_widget_show_all (widget);
}

void bl_view_set_decoration_end (BlView *self, GtkWidget *widget)
{
    BlViewPrivate *priv = bl_view_get_instance_private (self);
    gtk_box_pack_end (GTK_BOX (priv->decoration), widget, FALSE, FALSE, 0);
    gtk_widget_show_all (widget);
}

void bl_view_set_menu (BlView *self, GtkWidget *widget, gboolean expand)
{
    BlViewPrivate *priv = bl_view_get_instance_private (self);
    gtk_box_set_center_widget (GTK_BOX (priv->decoration), widget);
    // Set child packing to expand
    g_object_set (G_OBJECT (widget),
                  "hexpand", expand,
                  NULL);
}

void bl_view_set_decoration_style (BlView *self, gchar *css_class)
{
    BlViewPrivate *priv = bl_view_get_instance_private (self);
    helper_set_widget_css_class (priv->decoration, css_class);
}

void bl_view_remove_decoration_style (BlView *self, gchar *css_class)
{
    BlViewPrivate *priv = bl_view_get_instance_private (self);
    helper_remove_widget_css_class (priv->decoration, css_class);
}

static void
bl_view_init (BlView *self)
{
    // BlView Main Layout
    GtkWidget *widget = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    // View Header
    GtkWidget *area_header = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (widget), area_header, FALSE, FALSE, 0);

    BlViewPrivate *priv = bl_view_get_instance_private (self);
    priv->decoration = area_header;
    priv->main_box = widget;

    // CSS
    helper_set_widget_css_class (widget, "bl-view");
    helper_set_widget_css_class (area_header, "bl-view-heading");

    // Add to BlView
    gtk_container_add (GTK_CONTAINER (self), widget);
    gtk_widget_show_all (widget);
}
