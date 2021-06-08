/* bl-toolbar.c
 *
 * Copyright 2021 Matthew Jakeman <unknown@domain.org>
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

#include "bl-toolbar.h"
#include "helper.h"

G_DEFINE_TYPE (BlToolbar, bl_toolbar, GTK_TYPE_BOX)

/**
 * bl_toolbar_new:
 *
 * Create a new #BlToolbar.
 *
 * Returns: (transfer full): a newly created #BlToolbar
 */
BlToolbar *
bl_toolbar_new (void)
{
    return g_object_new (BL_TYPE_TOOLBAR, NULL);
}

static void
bl_toolbar_finalize (GObject *object)
{
    G_OBJECT_CLASS (bl_toolbar_parent_class)->finalize (object);
}

static void
bl_toolbar_class_init (BlToolbarClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class, "/com/mattjakeman/bluedit/bluedit-toolbar.ui");
    gtk_widget_class_bind_template_child (widget_class, BlToolbar, open_btn);
    gtk_widget_class_bind_template_child (widget_class, BlToolbar, save_btn);
    gtk_widget_class_bind_template_child (widget_class, BlToolbar, new_btn);
    gtk_widget_class_bind_template_child (widget_class, BlToolbar, menu_btn);

    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = bl_toolbar_finalize;
}

static void
bl_toolbar_init (BlToolbar *self)
{
    // Init template
    gtk_widget_init_template (GTK_WIDGET (self));
    helper_set_widget_css_class (GTK_WIDGET (self), "bl-toolbar");

    helper_set_widget_css_class (GTK_WIDGET (self->open_btn), "flat");
    helper_set_widget_css_class (GTK_WIDGET (self->save_btn), "flat");
    helper_set_widget_css_class (GTK_WIDGET (self->new_btn), "flat");
    helper_set_widget_css_class (GTK_WIDGET (self->menu_btn), "flat");
}
