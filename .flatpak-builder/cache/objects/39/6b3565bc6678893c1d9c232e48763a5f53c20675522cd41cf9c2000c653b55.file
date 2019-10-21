/* spl-workspace.c
 *
 * Copyright 2019 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "spl-workspace.h"
#include "../spl-tile-manager.h"

#define ACTION_REGION_SIZE 20
// TODO: Use properties
#define SAFETY 0.01

struct _SplWorkspace
{
    GtkContainer parent_instance;
};

typedef struct
{
    SplTileManager *context;
    SplArea *active;
    GList *children;
    GtkGesture *gesture;

    // Handle
    GdkWindow *event_window;

    // State
    SplEdge *last_edge;
    SplArea *last_area;

    // Action Regions
    gboolean draw_action_regions;
    gboolean draw_ar_native;
    gdouble ar_scale_factor;

    // Dimensions
    GdkRectangle widget_size;


} SplWorkspacePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (SplWorkspace, spl_workspace, GTK_TYPE_CONTAINER)

enum {
    PROP_0,
    PROP_ACTION_REGION,
    PROP_AR_SCALE_FACTOR,
    N_PROPS
};

enum {
    REGISTER_WIDGET,
    UNREGISTER_WIDGET,
    N_SIGNALS
};

static guint signals[N_SIGNALS];
static GParamSpec *properties [N_PROPS];

/**
 * spl_workspace_new:
 *
 * Create a new #SplWorkspace.
 *
 * Returns: (transfer full): a newly created #SplWorkspace
 */
SplWorkspace *
spl_workspace_new (void)
{
    return g_object_new (SPL_TYPE_WORKSPACE, NULL);
}

static void
spl_workspace_finalize (GObject *object)
{
    // SplWorkspace *self = (SplWorkspace *)object;
    // SplWorkspacePrivate *priv = spl_workspace_get_instance_private (self);

    G_OBJECT_CLASS (spl_workspace_parent_class)->finalize (object);
}

static void
spl_workspace_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    SplWorkspace *self = SPL_WORKSPACE (object);
    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_ACTION_REGION:
            g_value_set_boolean (value, priv->draw_action_regions);
            break;

        case PROP_AR_SCALE_FACTOR:
            g_value_set_double (value, priv->ar_scale_factor);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
spl_workspace_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    SplWorkspace *self = SPL_WORKSPACE (object);
    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (self);

    switch (prop_id)
    {
        case PROP_ACTION_REGION:
            priv->draw_action_regions = g_value_get_boolean (value);
            break;

        case PROP_AR_SCALE_FACTOR:
            priv->ar_scale_factor = g_value_get_double (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

void spl_workspace_split_active (SplWorkspace *workspace,
                                 guint         direction,
                                 float         fac)
{
    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (workspace);

    if (priv->active == NULL)
        priv->active = spl_tile_manager_get_any (priv->context);

    spl_area_split (priv->context, priv->active, direction, fac);
}

static void spl_workspace_size_allocate (GtkWidget *self,
                                         GtkAllocation *allocation)
{
    gtk_widget_set_allocation(self, allocation);
    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (SPL_WORKSPACE (self));

    // First thing we need to do is set the size of
    // SplTileManager. If we do not do this all scaling operations
    // will break.
    spl_tile_manager_resize (priv->context, allocation->width, allocation->height);


    for (GList* elem = spl_tile_manager_get_areas (priv->context); elem != NULL; elem = elem->next)
    {
        //g_debug("GTK Allocation");
        SplArea* area = elem->data;

        //g_debug ("%f", spl_area_get_width (area));
        GtkWidget* widget = GTK_WIDGET (spl_area_get_userdata (area));
        if (GTK_IS_WIDGET(widget))
        {
            //g_debug ("Allocating child widget");

            // Resize SplView associated with the area to take up the allocation
            GtkAllocation child_allocation;
            child_allocation.x = /*allocation->x*/ + spl_scale_width (priv->context, area->tl->x);
            child_allocation.y = /*allocation->y*/ + spl_scale_height (priv->context, area->tl->y);
            child_allocation.width = /*allocation->x*/ + spl_scale_width (priv->context, spl_area_get_width (area));
            child_allocation.height = /*allocation->y*/ + spl_scale_height (priv->context, spl_area_get_height (area));
            gtk_widget_size_allocate(widget, &child_allocation);
        }
        else
        {
            g_debug ("No widget associated with this area");
        }
    }

    if (gtk_widget_get_realized (self))
    {
        gdk_window_move_resize (priv->event_window,
                                allocation->x, allocation->y,
                                allocation->width, allocation->height);
    }

    // Resize size representation
    // TODO: Just use the GdkWindow instead?
    priv->widget_size.x = allocation->x;
    priv->widget_size.y = allocation->y;
    priv->widget_size.width = allocation->width;
    priv->widget_size.height = allocation->height;
}

static void
spl_workspace_map (GtkWidget *widget)
{
    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (SPL_WORKSPACE(widget));

    if (priv->event_window)
        gdk_window_show (priv->event_window);

    GTK_WIDGET_CLASS (spl_workspace_parent_class)->map(widget);
}

static void
spl_workspace_forall (GtkContainer *container, gboolean include_internals,
    GtkCallback callback, gpointer callback_data)
{
    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (SPL_WORKSPACE (container));
    g_list_foreach(priv->children, (GFunc)callback, callback_data);
}

static void
spl_workspace_add(GtkContainer *container, GtkWidget *widget)
{
    g_return_if_fail(container || SPL_IS_WORKSPACE(container));
    g_return_if_fail(widget || GTK_IS_WIDGET(widget));
    g_return_if_fail(gtk_widget_get_parent(widget) == NULL);

    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (SPL_WORKSPACE (container));

    /* Add the child to our list of children.
     * All the real work is done in gtk_widget_set_parent(). */
    priv->children = g_list_append(priv->children, widget);
    gtk_widget_set_parent(widget, GTK_WIDGET(container));

    /* Queue redraw */
    if(gtk_widget_get_visible(widget))
        gtk_widget_queue_resize(GTK_WIDGET(container));
}

static void
spl_workspace_remove(GtkContainer *container, GtkWidget *widget)
{
    g_return_if_fail(container || SPL_IS_WORKSPACE(container));
    g_return_if_fail(widget || GTK_IS_WIDGET(widget));

    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (SPL_WORKSPACE (container));

    /* Remove the child from our list of children.
     * Again, all the real work is done in gtk_widget_unparent(). */
    GList *link = g_list_find(priv->children, widget);
    if(link) {
        gboolean was_visible = gtk_widget_get_visible(widget);
        gtk_widget_unparent(widget);

        priv->children = g_list_delete_link(priv->children, link);

        /* Queue redraw */
        if(was_visible)
            gtk_widget_queue_resize(GTK_WIDGET(container));
    }
}

static GType
spl_workspace_child_type(GtkContainer *container)
{
	return GTK_TYPE_WIDGET;
}

// We need to override _realize in order to
// have our own GdkWindow, which is needed for
// both cursor handling and edge/resize gestures
static void
spl_workspace_realize (GtkWidget *widget)
{
    SplWorkspace *workspace = SPL_WORKSPACE (widget);
    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (workspace);

    GtkAllocation allocation;
    GdkWindowAttr attributes;

    gtk_widget_set_realized (widget, TRUE);
    gtk_widget_get_allocation (widget, &allocation);

    attributes.x = allocation.x;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.y = allocation.y;
    attributes.width  = allocation.width;
    attributes.height = allocation.height;
    attributes.event_mask = gtk_widget_get_events (widget) |
                            GDK_POINTER_MOTION_MASK |
                            GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_SMOOTH_SCROLL_MASK |
                            GDK_SCROLL_MASK |
                            GDK_TOUCH_MASK;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);

    priv->event_window = gdk_window_new (gtk_widget_get_parent_window (widget),
                             &attributes, GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL);

    gtk_widget_register_window (widget, priv->event_window);
    gtk_widget_set_window (widget, priv->event_window);
    //gdk_window_set_user_data (window, widget);

    // Create the initial area once realized (signals should have already been
    // connected).
    spl_tile_manager_create_initial (priv->context);
}

static gboolean
spl_workspace_draw (GtkWidget *widget,
                    cairo_t *cr)
{
    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (SPL_WORKSPACE (widget));

    for (GList *elem = priv->children; elem != NULL; elem = elem->next)
    {
        gtk_container_propagate_draw (GTK_CONTAINER (widget), elem->data, cr);

        // Corner Drawing
        // TODO: Hide this behind a property

        GtkAllocation child_allocation;
        gtk_widget_get_allocation (elem->data, &child_allocation);

        // Only draw Action Regions if the user
        // has enabled this property
        if (priv->draw_action_regions)
        {
            GtkStyleContext *style = gtk_widget_get_style_context (elem->data);
            gtk_style_context_add_class (style, "pane-separator");

            guint x = child_allocation.x;
            guint y = child_allocation.y;
            guint w = child_allocation.width;
            guint h = child_allocation.height;

            gdouble fac = priv->ar_scale_factor;

            cairo_save (cr);
            cairo_set_source_rgba (cr, 0.7f, 0.7f, 0.7f, 0.6f);

            // TL
            cairo_move_to (cr, x, y);
            cairo_line_to (cr, x + (ACTION_REGION_SIZE * fac), y);
            cairo_line_to (cr, x, y+ (ACTION_REGION_SIZE * fac));
            cairo_line_to (cr, x, y);
            cairo_new_sub_path (cr);

            // BR
            cairo_move_to (cr, (w + x), (h + y));
            cairo_line_to (cr, (w + x) - (ACTION_REGION_SIZE * fac), (h + y));
            cairo_line_to (cr, (w + x), (h + y) - (ACTION_REGION_SIZE * fac));
            cairo_line_to (cr, (w + x), (h + y));
            cairo_new_sub_path (cr);

            cairo_fill (cr);
            cairo_restore (cr);
        }
    }

    return FALSE;
}

static void
spl_workspace_class_init (SplWorkspaceClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = spl_workspace_finalize;
    object_class->get_property = spl_workspace_get_property;
    object_class->set_property = spl_workspace_set_property;

    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    //widget_class->get_preferred_width = spl_workspace_get_preferred_width;
    //widget_class->get_preferred_height = spl_workspace_get_preferred_height;
    widget_class->size_allocate = spl_workspace_size_allocate;
    widget_class->realize = spl_workspace_realize;
    widget_class->map = spl_workspace_map;
    widget_class->draw = spl_workspace_draw;

    GtkContainerClass *container_class = GTK_CONTAINER_CLASS(klass);
    container_class->child_type = spl_workspace_child_type;
    container_class->add = spl_workspace_add;
    container_class->remove = spl_workspace_remove;
    container_class->forall = spl_workspace_forall;

    signals[REGISTER_WIDGET] =
        g_signal_new ("register-widget",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 0 /* class_offset */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 1     /* n_params */,
                 G_TYPE_POINTER  /* param_types */);

    signals[UNREGISTER_WIDGET] =
        g_signal_new ("unregister-widget",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 0 /* class_offset */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 1     /* n_params */,
                 G_TYPE_POINTER  /* param_types */);

    properties[PROP_ACTION_REGION] =
        g_param_spec_boolean ("draw-action-regions",
                             "Draw Action Regions",
                             "Toggle whether action regions should be drawn.",
                             FALSE  /* default value */,
                             G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    properties[PROP_AR_SCALE_FACTOR] =
        g_param_spec_double ("handle-scale-factor",
                             "Handle Scale Factor",
                             "How big the handle should be compared to the action region",
                             0.0f, // min
                             10.0f, // max
                             1.0f, // default
                             G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_properties (object_class,
                                       N_PROPS,
                                       properties);

}

/*static GdkWindow *
spl_workspace_create_child_window (SplWorkspace *self, GtkWidget *widget)
{
    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (self);
    //gtk_widget_set_parent_window (widget, priv->event_window);

    GdkWindow *window;
    GdkWindowAttr attributes;
    GtkAllocation allocation;
    guint attributes_mask;

    gtk_widget_get_allocation (widget, &allocation);
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes_mask = GDK_WA_X | GDK_WA_Y| GDK_WA_VISUAL;

    window = gdk_window_new (gtk_widget_get_window (widget),
                             &attributes, attributes_mask);
    gtk_widget_register_window (widget, window);
    gtk_widget_set_parent_window (widget, window);

    return window;
}*/

void
spl_workspace_set_active (SplWorkspace *self, SplArea *area)
{
    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (self);
    priv->active = area;
    g_debug ("Set focus");
}

void
spl_workspace_register_widget (SplWorkspace *workspace, SplArea *area, GtkWidget *widget)
{
    gtk_container_add (GTK_CONTAINER (workspace), widget);
    spl_area_set_userdata (area, widget);
}

static void
cb_new_area (SplTileManager *context, SplArea *area, SplWorkspace *self)
{
    // Forward the signal to the user so they can create
    // the GtkWidget.
    g_signal_emit (self, signals[REGISTER_WIDGET], 0, area);

    // Reallocate size
    gtk_widget_queue_resize(GTK_WIDGET(self));
}

static void
cb_del_area (SplTileManager *context, gpointer area_data, SplWorkspace *self)
{
    g_debug ("Removing widget for SplArea");

    // Forward the signal to the user so they can
    // clean up.
    g_signal_emit (self, signals[UNREGISTER_WIDGET], 0, area_data);

    // Resize the container (redraw)
    if (gtk_widget_get_visible (GTK_WIDGET (self)))
    {
        gtk_widget_queue_resize(GTK_WIDGET(self));
    }
}

static void cb_gesture_drag_begin (GtkGestureDrag *gesture,
                                   gdouble         start_x,
                                   gdouble         start_y,
                                   gpointer        self)
{
    //g_debug ("Drag Start");

    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (self);
    priv->last_area = spl_area_get_for_coords (priv->context,
                                               spl_unscale_width (priv->context, start_x),
                                               spl_unscale_height (priv->context, start_y));

    gboolean corner_action = FALSE;
    gboolean edge_action = FALSE;

    // Step 1: Check if we are splitting/joining from a corner

    if (priv->last_area != NULL)
    {
        GtkWidget *widget = spl_area_get_userdata (priv->last_area);
        GtkAllocation allocation;
        gtk_widget_get_allocation (widget, &allocation);

        // Only proceed if we are in an action region

        // Top Left
        if (start_x < allocation.x + ACTION_REGION_SIZE &&
            start_y < allocation.y + ACTION_REGION_SIZE)
        {
            // We are in the top-left action corner
            g_debug ("Top Left");
            corner_action = TRUE;
        }

        /*// Top Right
        if (start_x > allocation.x + allocation.width - ACTION_REGION_SIZE &&
            start_y < allocation.y + ACTION_REGION_SIZE)
        {
            // We are in the top-right action corner
            g_debug ("Top Right");
            corner_action = TRUE;
        }*/

        /*// Bottom Left
        if (start_x < allocation.x + ACTION_REGION_SIZE &&
            start_y > allocation.y + allocation.height - ACTION_REGION_SIZE)
        {
            // We are in the bottom-left action corner
            g_debug ("Bottom Left");
            corner_action = TRUE;
        }*/

        // Bottom Right
        if (start_x > allocation.x + allocation.width - ACTION_REGION_SIZE &&
            start_y > allocation.y + allocation.height - ACTION_REGION_SIZE)
        {
            // We are in the bottom-right action corner
            g_debug ("Bottom Right");
            corner_action = TRUE;
        }
    }

    // Step 2: Check if we are resizing on an edge

    GtkAllocation workspace_size;
    gtk_widget_get_allocation (self, &workspace_size);

    SplEdge *edge = spl_edge_get_for_coords (priv->context,
                                             spl_unscale_width (priv->context, start_x),
                                             spl_unscale_height (priv->context, start_y),
                                             SAFETY);

    if (edge != NULL && !corner_action)
    {
        edge_action = TRUE;
        priv->last_edge = edge;
        priv->last_area = NULL;

        g_debug ("Found edge");
    }

    // Otherwise, cancel the gesture
    if (corner_action == FALSE &&
        edge_action == FALSE)
    {
        priv->last_area = NULL;
        gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
        g_debug ("Gesture cancelled");
    }
    else {
        gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
        g_debug ("Gesture claimed");
    }
}

static void
cb_gesture_drag_update (GtkGestureDrag *gesture,
                        gdouble         offset_x,
                        gdouble         offset_y,
                        gpointer        self)
{
    //g_debug ("Drag Update");

    gdouble start_x, start_y;
    gtk_gesture_drag_get_start_point (gesture, &start_x, &start_y);

    gdouble abs_x = start_x + offset_x;
    gdouble abs_y = start_y + offset_y;

    // This is where we do visual indication/feedback
    // for the join or split movement.

    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (self);

    // Resize edge
    if (priv->last_edge != NULL)
    {
        g_debug ("Moving edge");
        GtkAllocation workspace_size;
        gtk_widget_get_allocation (self, &workspace_size);

        guint orientation = spl_edge_get_orientation (priv->last_edge);

        if (orientation == SPL_HORIZONTAL)
            spl_edge_move (priv->context, priv->last_edge,
                           spl_unscale_height (priv->context, abs_y));
        else if (orientation == SPL_VERTICAL)
            spl_edge_move (priv->context, priv->last_edge,
                           spl_unscale_width (priv->context, abs_x));


        gtk_widget_queue_resize (self);
    }
}

static void cb_gesture_drag_end (GtkGestureDrag *gesture,
                                 gdouble         offset_x,
                                 gdouble         offset_y,
                                 gpointer        self)
{
    //g_debug ("Drag End");

    gdouble start_x, start_y;
    gtk_gesture_drag_get_start_point (gesture, &start_x, &start_y);

    gdouble abs_x = start_x + offset_x;
    gdouble abs_y = start_y + offset_y;

    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (self);

    if (priv->last_area == NULL)
        return;

    if (!spl_area_check_for_coords (priv->last_area,
                                    spl_unscale_width (priv->context, abs_x),
                                    spl_unscale_height (priv->context, abs_y)))
    {
        // We have changed to a new area
        SplArea *focus = spl_area_get_for_coords (priv->context,
                                                  spl_unscale_width (priv->context, abs_x),
                                                  spl_unscale_height (priv->context, abs_y));

        // Initiate a join between the old area and the current,
        // keeping the old area
        spl_area_join (priv->context, priv->last_area, focus);

        // Finally, set the active area as the newly joined one
        priv->active = priv->last_area;
    }

    priv->last_area = NULL;
    priv->last_edge = NULL;
}

static void
spl_workspace_init (SplWorkspace *self)
{
    // gtk_widget_set_has_window(GTK_WIDGET(self), FALSE);

    SplWorkspacePrivate *priv = spl_workspace_get_instance_private (self);
    priv->context = g_object_new (SPL_TYPE_TILE_MANAGER,
                                  "minimum-size", 0.1f,
                                  NULL);
    priv->active = NULL;

    g_signal_connect (priv->context, "area-created",
                      G_CALLBACK (cb_new_area), self);

    g_signal_connect (priv->context, "area-removed",
                      G_CALLBACK (cb_del_area), self);

    // We call `create_initial` in `spl_workspace_realize ()` so that
    // the user has a chance to setup the appropriate signals.

    priv->gesture = gtk_gesture_drag_new (GTK_WIDGET (self));
    gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (priv->gesture),
                                                GTK_PHASE_CAPTURE);

    // For resizing
    g_signal_connect (priv->gesture, "drag-begin",
                      G_CALLBACK (cb_gesture_drag_begin), self);
    g_signal_connect (priv->gesture, "drag-update",
                      G_CALLBACK (cb_gesture_drag_update), self);
    g_signal_connect (priv->gesture, "drag-end",
                      G_CALLBACK (cb_gesture_drag_end), self);

}
