/* bl-workspace.c
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

#include "bl-workspace.h"
#include "views/bl-view.h"
#include "views/bl-editor.h"

#include <spl.h>

struct _BlWorkspace
{
    GtkBin parent_instance;
};

G_DEFINE_TYPE (BlWorkspace, bl_workspace, GTK_TYPE_BIN)

/**
 * bl_workspace_new:
 *
 * Create a new #BlWorkspace.
 *
 * Returns: (transfer full): a newly created #BlWorkspace
 */
BlWorkspace *
bl_workspace_new (void)
{
    return g_object_new (BL_TYPE_WORKSPACE, NULL);
}

static void
bl_workspace_finalize (GObject *object)
{
    G_OBJECT_CLASS (bl_workspace_parent_class)->finalize (object);
}

static void
bl_workspace_class_init (BlWorkspaceClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = bl_workspace_finalize;
}

static void
cb_del_area (SplWorkspace *workspace, gpointer area_data, gpointer null_ptr)
{
    // Destroy the widget
    gtk_widget_destroy (GTK_WIDGET (area_data));
}

/*static void
cb_focused (GtkWidget *widget, SplWorkspace *workspace)
{
    SplArea *area = g_object_get_data (G_OBJECT (widget),
                                       "spl-area");
    spl_workspace_set_active (workspace, area);
    g_debug ("Set focus");
}*/

static void
split_area (GtkWidget *button, gpointer null_ptr)
{
    g_debug ("Splitting Area");
    SplWorkspace *workspace = g_object_get_data (G_OBJECT (button), "spl-workspace");
    SplArea *area = g_object_get_data (G_OBJECT (button), "spl-area");
    guint direction = (guint)g_object_get_data (G_OBJECT (button), "spl-direction");

    g_debug ("hello");

    spl_workspace_set_active (workspace, area);
    spl_workspace_split_active (workspace, direction, 0.5);
}

static void
cb_new_area (SplWorkspace *workspace, SplArea *area, gpointer null_ptr)
{
    g_debug ("Creating widget for SplArea");

    BlEditor *editor = g_object_new (BL_TYPE_EDITOR,
                                     NULL);

    // Split Buttons
    GtkWidget *split_button_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *split_h = gtk_button_new ();
    gtk_button_set_image (GTK_BUTTON (split_h),
                          gtk_image_new_from_resource ("/com/mattjakeman/bluedit/builder-view-right-pane-symbolic.svg"));
    GtkWidget *split_v = gtk_button_new ();
    gtk_button_set_image (GTK_BUTTON (split_v),
                          gtk_image_new_from_resource ("/com/mattjakeman/bluedit/builder-view-bottom-pane-symbolic.svg"));
    helper_set_widget_css_class (split_h, "flat");
    helper_set_widget_css_class (split_v, "flat");

    gtk_box_pack_start (GTK_BOX (split_button_box), split_h, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (split_button_box), split_v, FALSE, FALSE, 0);

    bl_view_set_decoration_end (BL_VIEW (editor), split_button_box);

    // We use GObject associations on the button to set the type of split
    // and other parameters. Alternatively we could pass a struct containing
    // all this as user_data.
    g_object_set_data (G_OBJECT (split_h),
                       "spl-workspace", workspace);
    g_object_set_data (G_OBJECT (split_h),
                       "spl-area", area);
    g_object_set_data (G_OBJECT (split_h),
                       "spl-direction", (void*)SPL_HORIZONTAL);

    g_object_set_data (G_OBJECT (split_v),
                       "spl-workspace", workspace);
    g_object_set_data (G_OBJECT (split_v),
                       "spl-area", area);
    g_object_set_data (G_OBJECT (split_v),
                       "spl-direction", (void*)SPL_VERTICAL);

    g_signal_connect (G_OBJECT (split_h), "clicked",
                      G_CALLBACK (split_area), NULL);

    g_signal_connect (G_OBJECT (split_v), "clicked",
                      G_CALLBACK (split_area), NULL);

    //g_signal_connect (G_OBJECT(widget), "realize", G_CALLBACK(cb_on_realise), NULL);

    // Register
    gtk_widget_show (GTK_WIDGET (editor));
    spl_workspace_register_widget (workspace, area, GTK_WIDGET (editor));
}

static void
bl_workspace_init (BlWorkspace *self)
{
    // SplWorkspace from libsplit provides split-screen functionality
    GtkWidget *spl = g_object_new(SPL_TYPE_WORKSPACE,
                                        "draw-action-regions", TRUE,
                                        "handle-scale-factor", 0.6f,
                                        NULL);

    gtk_container_add (GTK_CONTAINER (self), spl);

    g_signal_connect (spl, "register-widget",
                      G_CALLBACK (cb_new_area), NULL);

    g_signal_connect (spl, "unregister-widget",
                      G_CALLBACK (cb_del_area), NULL);
}
