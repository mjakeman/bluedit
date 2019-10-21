/* spl-tile-manager.c
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


#include "spl-tile-manager.h"

struct _SplTileManager
{
    GObject parent_instance;
};

typedef struct
{
    // Doubly linked-lists

    // SplVertex
    GList* vertices;

    // SplEdge
    GList* edges;

    // SplArea
    GList* areas;

    // Screen
    guint width;
    guint height;

    // Internal Status
    gdouble min_size;

} SplTileManagerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (SplTileManager, spl_tile_manager, G_TYPE_OBJECT)

enum {
    PROP_0,
    MIN_SIZE, // In real-coords (display pixels)
    N_PROPS
};

enum {
    AREA_CREATED,
    AREA_REMOVED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];
static GParamSpec *properties [N_PROPS];

/**
 * spl_tile_manager_new:
 *
 * Create a new #SplTileManager.
 *
 * Returns: (transfer full): a newly created #SplTileManager
 */
SplTileManager *
spl_tile_manager_new ()
{
    return g_object_new (SPL_TYPE_TILE_MANAGER, NULL);
}

static void
spl_tile_manager_finalize (GObject *object)
{
    // SplTileManager *self = (SplTileManager *)object;
    // SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    G_OBJECT_CLASS (spl_tile_manager_parent_class)->finalize (object);
}

static void
spl_tile_manager_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    SplTileManager *self = SPL_TILE_MANAGER (object);
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    switch (prop_id)
    {
        case MIN_SIZE:
            g_value_set_double (value, priv->min_size);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
spl_tile_manager_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    SplTileManager *self = SPL_TILE_MANAGER (object);
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    switch (prop_id)
    {
        case MIN_SIZE:
            priv->min_size = g_value_get_double (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
spl_tile_manager_class_init (SplTileManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = spl_tile_manager_finalize;
    object_class->get_property = spl_tile_manager_get_property;
    object_class->set_property = spl_tile_manager_set_property;

    signals[AREA_CREATED] =
        g_signal_new ("area-created",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 0 /* class_offset */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 1     /* n_params */,
                 G_TYPE_POINTER  /* param_types */);

    signals[AREA_REMOVED] =
        g_signal_new ("area-removed",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 0 /* class_offset */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 1     /* n_params */,
                 G_TYPE_POINTER  /* param_types */);

    properties[MIN_SIZE] =
        g_param_spec_double ("minimum-size",
                           "Minimum size",
                           "Minimum size of any given area at a time.",
                           0.0f /* minimum value */,
                           1.0f /* maximum value */,
                           0.1f /* default value */,
                           G_PARAM_READWRITE);

    g_object_class_install_properties (object_class,
                                       N_PROPS,
                                       properties);
}

void
print_area_single(SplArea* area)
{
    SplVertex *tl = area->tl;
    SplVertex *tr = area->tr;
    SplVertex *bl = area->bl;
    SplVertex *br = area->br;
    g_debug ("\n - Top Left (%f, %f)\n - Top Right (%f, %f)\n - Bottom Left (%f, %f)\n - Bottom Right (%f, %f)\n",
             tl->x, tl->y, tr->x, tr->y, bl->x, bl->y, br->x, br->y);
}

void
print_vertex_single(SplVertex* vertex)
{
    gdouble x = vertex->x;
    gdouble y = vertex->y;
    g_debug ("Vertex: (%f, %f)", x, y);
}

static void
cb_print_area (SplArea* area, gpointer null_ptr)
{
    print_area_single (area);
}

void
print_areas (SplTileManager *self)
{
    // Get Private
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    GList *areas = priv->areas;

    if (areas == NULL)
    {
        // List is empty, return
        g_debug("No Areas");
        return;
    }

    // Iterate over list
    g_list_foreach(priv->areas, (GFunc)cb_print_area, NULL);
}

static void cb_print_vertex(SplVertex* vertex, gpointer null_ptr)
{
    print_vertex_single (vertex);
}

void
print_vertices (SplTileManager *self)
{
    // Get Private
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    GList *vertices = priv->vertices;

    if (vertices == NULL)
    {
        // List is empty, return
        g_debug("No Vertices");
        return;
    }

    // Iterate over list
    g_list_foreach(priv->vertices, (GFunc)cb_print_vertex, NULL);
}

static void
swap_vertices(SplVertex* tl, SplVertex* tr)
{
    // Create a temporary vertex
    SplVertex* temp = g_malloc(sizeof(SplVertex));

    // Let temp = tl
    memcpy (temp, tl, sizeof(SplVertex));

    // Make tl = tr
    memcpy (tl, tr, sizeof(SplVertex));

    // Make tr = temp
    memcpy (tr, temp, sizeof(SplVertex));

    // Free temp
    g_free(temp);
}

static void
arrange_vertices(SplVertex* tl, SplVertex* tr, SplVertex* bl, SplVertex* br)
{
    // Topleft is greater than topright
    if (tl->x > tr->x)
    {
        swap_vertices(tl, tr);
    }

    // Bottomleft greater than bottomright
    if (bl->x > br->x)
    {
        swap_vertices(bl, br);
    }

    // Topleft greater than bottomleft
    if (tl->y > bl->y)
    {
        swap_vertices(tl, bl);
    }

    // Topright greater than bottomright
    if (tr->y > br->y)
    {
        swap_vertices(tr, br);
    }
}

static SplVertex*
create_vertex(gdouble x, gdouble y)
{
    SplVertex* v = g_malloc(sizeof(SplVertex));
    v->x = x;
    v->y = y;
    return v;
}

static SplEdge*
create_edge(SplVertex *v1, SplVertex *v2)
{
    SplEdge *e = g_malloc(sizeof(SplEdge));
    e->v1 = v1;
    e->v2 = v2;
    return e;
}

gdouble
spl_area_get_height(SplArea *area)
{
    gdouble height1 = area->bl->y - area->tl->y;
    gdouble height2 = area->br->y - area->tr->y;

    // Non rectangular area (undefined behaviour)
    if (height1 != height2)
    {
        g_error("Malformed area");
    }

    // Return
    return height1;
}

gdouble
spl_area_get_width(SplArea *area)
{
    gdouble width1 = area->tr->x - area->tl->x;
    gdouble width2 = area->br->x - area->bl->x;

    // Non rectangular area (undefined behaviour)
    if (width1 != width2)
    {
        g_error("Malformed area: width1 %f width2 %f", width1, width2);
    }

    // Return
    return width1;
}

void spl_area_set_userdata(SplArea *area, gpointer data)
{
    if (area == NULL)
    {
        g_error ("Invalid area");
        return;
    }
    area->user_data = data;
}

gpointer spl_area_get_userdata(SplArea *area)
{
    if (area == NULL)
    {
        g_error ("Invalid area");
        return NULL;
    }
    return area->user_data;
}

static SplArea*
spl_tile_manager_create_area(SplTileManager *self,
                             SplVertex      *tl,
                             SplVertex      *tr,
                             SplVertex      *bl,
                             SplVertex      *br)
{
    // This function assumes that the vertices and edges are already correct
    // Up to the caller to make sure.

    // Create area
    SplArea *area = g_malloc (sizeof(SplArea));
    area->tl = tl;
    area->tr = tr;
    area->bl = bl;
    area->br = br;

    SplEdge* top = create_edge (tl, tr);
    SplEdge* left = create_edge (tl, bl);
    SplEdge* bottom = create_edge (bl, br);
    SplEdge* right = create_edge (tr, br);

    // Colours
    area->r = rand() % 256;
    area->g = rand() % 256;
    area->b = rand() % 256;

    // User Data
    area->user_data = NULL;

    // Get Private
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    // Add to array (we use prepend for better linked list performance)

    // Vertices
    priv->vertices = g_list_prepend (priv->vertices, area->tl);
    priv->vertices = g_list_prepend (priv->vertices, area->tr);
    priv->vertices = g_list_prepend (priv->vertices, area->bl);
    priv->vertices = g_list_prepend (priv->vertices, area->br);

    // Edges
    priv->edges = g_list_prepend (priv->edges, top);
    priv->edges = g_list_prepend (priv->edges, left);
    priv->edges = g_list_prepend (priv->edges, bottom);
    priv->edges = g_list_prepend (priv->edges, right);

    // Area
    priv->areas = g_list_prepend (priv->areas, area);

    // Log
    g_debug("Created Area");
    g_signal_emit (self, signals[AREA_CREATED], 0, area);
    print_area_single(area);

    return area;
}

static void
add_edge(SplTileManager *self,
         SplEdge        *edge)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);
    priv->edges = g_list_prepend(priv->edges, edge);
}

static inline guint
spl_scale (gdouble value, guint scale)
{
    // TODO: Use proper rounding, rather than
    // int-shortening?
    return (guint)(((gdouble)value / 1.0f) * (float)scale);
}

inline guint
spl_scale_width (SplTileManager *self, gdouble value)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);
    return spl_scale (value, priv->width);
}

inline guint
spl_scale_height (SplTileManager *self, gdouble value)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);
    return spl_scale (value, priv->height);
}

static inline gdouble
spl_unscale (guint mouse, guint fac)
{
    return (gdouble)(((float)mouse / (float)fac) * 1.0f);
}

inline gdouble
spl_unscale_width (SplTileManager *self, guint mouse)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);
    return spl_unscale (mouse, priv->width);
}

inline gdouble
spl_unscale_height (SplTileManager *self, guint mouse)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);
    return spl_unscale (mouse, priv->height);
}

void
spl_tile_manager_resize (SplTileManager *self,
                         guint           width,
                         guint           height)
{
    // TODO: Add resize checks to make sure we can resize?
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);
    priv->width = width;
    priv->height = height;
}

static gboolean
spl_area_can_split (SplTileManager *self,
                    SplArea *area,
                    guint direction)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    if (direction == SPL_HORIZONTAL)
    {
        gdouble area_width = spl_area_get_width (area);

        // Check area is big enough to be split
        if (area_width >= 2 * priv->min_size)
        {
            return TRUE;
        }
    }

    if (direction == SPL_VERTICAL)
    {
        gdouble area_height = spl_area_get_height (area);

        // Check area is big enough to be split
        if (area_height >= 2 * priv->min_size)
        {
            return TRUE;
        }
    }

    g_debug ("Split Denied");
    return FALSE;
}

SplArea*
spl_area_split (SplTileManager *self,
                SplArea        *area,
                guint           direction,
                float           fac)
{
    // Get Private
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    if (g_list_find(priv->areas, area) == NULL)
    {
        // Area not found, oops
        g_error("Foreign area passed to split function");
        return NULL;
    }

    // Total Width and Height
    gdouble width = spl_area_get_width(area);
    gdouble height = spl_area_get_height(area);

    // Bordering Vertices
    SplVertex* top_left = area->tl;
    SplVertex* top_right = area->tr;
    SplVertex* bottom_left = area->bl;
    SplVertex* bottom_right = area->br;

    // Make sure vertices refer to what we expect them to
    arrange_vertices (top_left, top_right, bottom_left, bottom_right);

    // New Area
    SplArea* new_area = NULL;

    if (direction == SPL_HORIZONTAL)
    {
        // Horizontal split means that the areas are placed next to
        // each other (side by side)

        // If the factor of the split is greater than 0.5, then the split will occur
        // from the opposite direction, resulting in the new area being placed on the
        // right.
        gboolean reverse;
        if (fac > 0.5)
            reverse = TRUE;
        else
            reverse = FALSE;

        // Check we are big enough to split
        gboolean result = spl_area_can_split (self, area, SPL_HORIZONTAL);
        if (result == FALSE)
            return NULL;

        gdouble a1_width = width * fac;

        // Find Middle Vertices
        gdouble mid_vertex_x = top_left->x + a1_width;
        SplVertex* mid_vertex_top = create_vertex (mid_vertex_x, top_left->y);
        SplVertex* mid_vertex_bottom = create_vertex (mid_vertex_x, bottom_left->y);

        if (reverse)
        {
            // # Old Area is placed on the left (Old Area is a1)

            // This means that the area's tl and bl stay the same, while
            // tr and br point to mid_vertex_top and mid_vertex_bottom
            // respectively.

            area->tr = mid_vertex_top;
            area->br = mid_vertex_bottom;

            // In terms of edges, the left edge remains unchanged. The top
            // and bottom edges have their second vertex changed to the middle
            // vertex. The right edge is shared with a2, and is created for us
            // below.

            add_edge(self, create_edge(area->tl, mid_vertex_top));
            add_edge(self, create_edge(area->bl, mid_vertex_bottom));

            // # New Area is placed on the right (New Area is a2)

            // This means that the area's tl and bl vertices are the middle
            // vertices, and that its tr and br are the same as the original
            // non-split area. We do not need to worry about edges as create_area
            // creates them for us.

            new_area = spl_tile_manager_create_area (self, mid_vertex_top, top_right,
                                                     mid_vertex_bottom, bottom_right);

        }
        else
        {
            // # New Area is placed on the left (New Area is a1)

            // This means that the area's tl and bl vertices are carried
            // over from the original area, while the tr and br vertices are
            // equal to the middle vertices.

            new_area = spl_tile_manager_create_area (self, top_left, mid_vertex_top,
                                                     bottom_left, mid_vertex_bottom);

            // The above function creates all of the edges needed for us, so we do not
            // need to do anything here.

            // # Old Area is placed on the right (Old Area is a2)

            // This means that the middle vertices are this areas tl and bl,
            // and the original rect tr and br are the tr and br for this area.

            area->tl = mid_vertex_top;
            area->bl = mid_vertex_bottom;

            // In terms of edges, we already have the left and right edges created for
            // us, the first by the `create_area` function, and the second from the original
            // rect. We need to create the top and bottom edges here.

            add_edge (self, create_edge (mid_vertex_top, top_right));
            add_edge (self, create_edge (mid_vertex_bottom, bottom_right));
        }

        // Finally, to make this work, we need to add the two newly created
        // vertices to the vertices list
        priv->vertices = g_list_prepend(priv->vertices, mid_vertex_top);
        priv->vertices = g_list_prepend(priv->vertices, mid_vertex_bottom);

    }
    else if (direction == SPL_VERTICAL)
    {
        // Vertical split means that the areas are placed on top of
        // each other

        gdouble a1_height = height * fac;

        // Find Middle Vertices
        gdouble mid_vertex_y = top_left->y + a1_height;
        SplVertex* mid_vertex_left = create_vertex (top_left->x, mid_vertex_y);
        SplVertex* mid_vertex_right = create_vertex (top_right->x, mid_vertex_y);

        // Check we are big enough to split
        gboolean result = spl_area_can_split (self, area, SPL_VERTICAL);
        if (result == FALSE)
            return NULL;

        // If the factor of the split is greater than 0.5, then the split will occur
        // from the opposite direction, resulting in the new area being placed on the
        // bottom.
        if (fac > 0.5)
        {
            // # Old Area is placed on the top (Old Area is a1)

            // This means that the area's tl and tr stay the same, while
            // bl and br point to mid_vertex_left and mid_vertex_right
            // respectively.

            area->bl = mid_vertex_left;
            area->br = mid_vertex_right;

            // In terms of edges, the top edge remains unchanged. The left
            // and right edges have their second vertex changed to the middle
            // vertex. The bottom edge is shared with a2, and is created for us
            // below.

            add_edge(self, create_edge(area->tl, mid_vertex_left));
            add_edge(self, create_edge(area->tr, mid_vertex_right));

            // # New Area is placed on the bottom (New Area is a2)

            // This means that the area's tl and tr vertices are the middle
            // vertices, and that its bl and br are the same as the original
            // non-split area. We do not need to worry about edges as create_area
            // creates them for us.

            new_area = spl_tile_manager_create_area (self, mid_vertex_left, mid_vertex_right,
                                                     bottom_left, bottom_right);

        }
        else
        {
            // # New Area is placed on the top (New Area is a1)

            // This means that the area's tl and tr vertices are carried
            // over from the original area, while the bl and br vertices are
            // equal to the middle vertices.

            new_area = spl_tile_manager_create_area (self, top_left, top_right,
                                                     mid_vertex_left, mid_vertex_right);

            // The above function creates all of the edges needed for us, so we do not
            // need to do anything here.

            // # Old Area is placed on the bottom (Old Area is a2)

            // This means that the middle vertices are this areas tl and tr,
            // and the original rect bl and br are the bl and br for this area (unchanged).

            area->tl = mid_vertex_left;
            area->tr = mid_vertex_right;

            // In terms of edges, we already have the top and bottom edges created for
            // us, the first by the `create_area` function, and the second from the original
            // rect. We need to create the left and right edges here.

            add_edge (self, create_edge (mid_vertex_left, bottom_left));
            add_edge (self, create_edge (mid_vertex_right, bottom_right));
        }

        // Finally, to make this work, we need to add the two newly created
        // vertices to the vertices list
        priv->vertices = g_list_prepend(priv->vertices, mid_vertex_left);
        priv->vertices = g_list_prepend(priv->vertices, mid_vertex_right);
    }
    else
    {
        g_error("Invalid direction");
        return NULL;
    }

    if (new_area != NULL)
    {
        g_debug("Area split successfully");
    }

    // FIXME: We have edges left over from this operation, and this
    // constitutes a memory leak (if negligible). When a split area shares
    // edges with bounding areas, these edges cannot be resized (as this would
    // break adjacent areas), and are instead recreated. Therefore we need to
    // run an edge deduplication function.

    // Return new area
    return new_area;
}

static void
spl_tile_manager_init (SplTileManager *self)
{
    // Initialisation
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);
    priv->areas = NULL;
    priv->edges = NULL;
    priv->vertices = NULL;

    // The caller is expected to call `create_initial`
    // after setting up signal callbacks
}

void
spl_tile_manager_create_initial (SplTileManager *self)
{
    // We are using a top-left origin system
    // where top-left is (0, 0) and bottom-right
    // is (1, 1)

    SplVertex* tl = create_vertex (0, 0);       // TL
    SplVertex* tr = create_vertex (1, 0);     // TR
    SplVertex* bl = create_vertex (0, 1);     // BL
    SplVertex* br = create_vertex (1, 1);   // BR

    // Create area
    spl_tile_manager_create_area(self, tl, tr, bl, br);
}

gboolean within_safety(gdouble n1,
                       gdouble n2,
                       gdouble safety)
{
    if (n1 > n2)
        return ((n1 - n2) < safety);
    else
        return ((n2 - n1) < safety);
}

gboolean within_range(gdouble n,
                      gdouble start,
                      gdouble end)
{
    if (n > start && n < end)
        return TRUE;
    else
        return FALSE;
}

guint
spl_edge_get_orientation(SplEdge* edge)
{
    SplVertex* v1 = edge->v1;
    SplVertex* v2 = edge->v2;

    if (v1->x == v2->x)
    {
        return SPL_VERTICAL;
    }
    else if (v1->y == v2->y)
    {
        return SPL_HORIZONTAL;
    }
    else
    {
        g_error("Malformed line");
        return 0;
    }
}

SplArea *
spl_tile_manager_get_any (SplTileManager *self)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);
    return priv->areas->data;
}

static gboolean
spl_vertex_is_equal(SplVertex *v1, SplVertex *v2)
{
    return (v1->x == v2->x &&
            v1->y == v2->y);
}

static void
spl_tile_manager_remove_area (SplTileManager *self, SplArea *remove)
{
    g_signal_emit (self, signals[AREA_REMOVED], 0, spl_area_get_userdata (remove));

    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);
    priv->areas = g_list_remove(priv->areas, remove);
}

gboolean
spl_area_join(SplTileManager *self, SplArea *keep, SplArea *join)
{
    if (join == NULL ||
        keep == NULL ||
        !(SPL_IS_TILE_MANAGER (self)))
        return FALSE;

    // Get orientation of areas to each other
    enum {
        LEFT,
        RIGHT,
        TOP,
        BOTTOM,
        INVALID
    };

    guint direction = INVALID;

    // Get direction of join
    // Resize keep area to absorb join area's allocation
    if (spl_vertex_is_equal (keep->tl, join->tr) &&
        spl_vertex_is_equal (keep->bl, join->br))
    {
        direction = LEFT;
        g_debug("Join Direction: Left");

        keep->tl = join->tl;
        keep->bl = join->bl;

        add_edge (self, create_edge (keep->tl, keep->tr));
        add_edge (self, create_edge (keep->bl, keep->br));
    }
    else if (spl_vertex_is_equal (keep->tl, join->bl) &&
             spl_vertex_is_equal (keep->tr, join->br))
    {
        direction = TOP;
        g_debug("Join Direction: Top");

        keep->tl = join->tl;
        keep->tr = join->tr;

        add_edge (self, create_edge (keep->tl, keep->bl));
        add_edge (self, create_edge (keep->tr, keep->br));
    }
    else if (spl_vertex_is_equal (keep->tr, join->tl) &&
             spl_vertex_is_equal (keep->br, join->bl))
    {
        direction = RIGHT;
        g_debug("Join Direction: Right");

        keep->tr = join->tr;
        keep->br = join->br;

        add_edge (self, create_edge (keep->tl, keep->tr));
        add_edge (self, create_edge (keep->bl, keep->br));
    }
    else if (spl_vertex_is_equal (keep->bl, join->tl) &&
             spl_vertex_is_equal (keep->br, join->tr))
    {
        direction = BOTTOM;
        g_debug("Join Direction: Bottom");

        keep->bl = join->bl;
        keep->br = join->br;

        add_edge (self, create_edge (keep->tl, keep->bl));
        add_edge (self, create_edge (keep->tr, keep->br));
    }

    if (direction == INVALID)
    {
        return FALSE;
        g_debug("Invalid Join");
    }

    // Remove join area from list
    spl_tile_manager_remove_area (self, join);

    // Delete join area
    // g_free(join);

    // Remove doubles and unused vertices
    // TODO

    // Return success
    return TRUE;
}

gboolean
spl_edge_is_border (SplEdge *edge)
{
    // Criteria:
    // If the x-coords of both vertices are 0/1
    // Or the y-coords of both vertices are 0/1
    // then it is an border edge

    guint dir = spl_edge_get_orientation (edge);

    if (dir == SPL_VERTICAL)
    {
        if (edge->v1->x == 0.0f &&
            edge->v2->x == 0.0f)
            return TRUE;

        if (edge->v1->x == 1.0f &&
            edge->v2->x == 1.0f)
            return TRUE;
    }

    if (dir == SPL_HORIZONTAL)
    {
        if (edge->v1->y == 0.0f &&
            edge->v2->y == 0.0f)
            return TRUE;

        if (edge->v1->y == 1.0f &&
            edge->v2->y == 1.0f)
            return TRUE;
    }

    return FALSE;
}

// This function takes the vertices of an SplEdge and moves them to the
// correct position. This has the added benefit of updating all edges and
// areas which point to them for free. To avoid malformed edges for areas
// with an edge not at a vertex, we find all connected edges in the given
// direction, and resize appropriately.
gboolean
spl_edge_move (SplTileManager *self,
               SplEdge *edge,
               gdouble new_pos)
{
    guint orientation = spl_edge_get_orientation (edge);

    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    // Don't let border edges be moved
    if (spl_edge_is_border (edge))
        return FALSE;

    // Minimum sizes
    // Get areas attached to each vertex
    for (GList *elem = priv->areas; elem != NULL; elem = elem->next)
    {
        SplArea *area = elem->data;
        gboolean connected = FALSE;
        if (orientation == SPL_VERTICAL)
        {
            // Line is vertical, check if either
            // (tl and bl) or (tr and br) equal
            // this edge. It is convention that edge
            // v1 will be top and edge v2 will be bottom

            gdouble width;

            // Left Edge
            if (area->tl == edge->v1 ||
                area->bl == edge->v2)
            {
                connected = TRUE;
                width = area->tr->x - new_pos;
            }

            // Right Edge
            if (area->tr == edge->v1 ||
                area->br == edge->v2)
            {
                connected = TRUE;
                width = new_pos - area->tl->x;
            }

            if (connected &&
                width < priv->min_size)
                return FALSE; // Deny the resize
        }
        if (orientation == SPL_HORIZONTAL)
        {
            // Line is horizontal, check if either
            // (tl and tr) or (bl and br) equal
            // this edge. It is convention that edge
            // v1 will be left and edge v2 will be right

            gdouble height;

            // Top Edge
            if (area->tl == edge->v1 ||
                area->tr == edge->v2)
            {
                connected = TRUE;
                height = area->bl->y - new_pos;
            }

            // Bottom Edge
            if (area->bl == edge->v1 ||
                area->br == edge->v2)
            {
                connected = TRUE;
                height = new_pos - area->tr->y;
            }

            if (connected &&
                height < priv->min_size)
                return FALSE; // Deny the resize
        }
    }

    // The line is vertical
    if (orientation == SPL_VERTICAL)
    {
        g_assert (edge->v1->x == edge->v2->x);

        // Find connected vertices
        gdouble old_x = edge->v1->x;
        for (GList *elem = priv->vertices; elem != NULL; elem = elem->next)
        {
            SplVertex *cmp = elem->data;
            if (cmp->x == old_x)
            {
                g_debug ("New Vertex x = %f", new_pos);
                cmp->x = new_pos;
            }
        }

        return TRUE;
    }

    // The line is horizontal
    if (orientation == SPL_HORIZONTAL)
    {
        g_assert (edge->v1->y == edge->v2->y);

        // Find connected vertices
        gdouble old_y = edge->v1->y;
        for (GList *elem = priv->vertices; elem != NULL; elem = elem->next)
        {
            SplVertex *cmp = elem->data;
            if (cmp->y == old_y)
            {
                cmp->y = new_pos;
                g_debug ("New Vertex y = %f", new_pos);
            }
        }

        return TRUE;
    }

    // print_vertex_single (edge->v1);
    // print_vertex_single (edge->v2);

    // print_areas (self);

    return FALSE;
}

gboolean
spl_edge_check_for_coords (SplEdge *edge, gdouble mouse_x, gdouble mouse_y, gdouble safety)
{
    SplVertex* v1 = edge->v1;
    SplVertex* v2 = edge->v2;

    if (spl_edge_get_orientation (edge) == SPL_VERTICAL)
    {
        // If vertical, then mouse must be within `safety` of v1->x
        // And between the top and bottom y values
        if (within_safety(mouse_x, v1->x, safety) && within_range(mouse_y, v1->y, v2->y))
        {
            return TRUE;
        }
    }
    else if (spl_edge_get_orientation (edge) == SPL_HORIZONTAL)
    {
        // If horizontal, then mouse must be within `safety` of v1->y
        // And between the left and right x values
        if (within_safety(mouse_y, v1->y, safety) && within_range(mouse_x, v1->x, v2->x))
        {
            return TRUE;
        }
    }
    else
    {
        // This should never happen
        g_error("Malformed Edge");
    }

    return FALSE;
}

gboolean
spl_area_check_for_coords (SplArea *area, gdouble mouse_x, gdouble mouse_y)
{
    if (area == NULL)
        return FALSE;

    if (mouse_x > area->tl->x &&
        mouse_x < area->tr->x &&
        mouse_y > area->tl->y &&
        mouse_y < area->bl->y)
        return TRUE;

    return FALSE;
}

SplArea*
spl_area_get_for_coords (SplTileManager *self, gdouble mouse_x, gdouble mouse_y)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    for (GList *elem = priv->areas; elem != NULL; elem = elem->next)
    {
        SplArea *area = elem->data;
        if (spl_area_check_for_coords(area, mouse_x, mouse_y))
        {
            return area;
        }
    }

    return NULL;
}

SplEdge*
spl_edge_get_for_coords (SplTileManager *self, gdouble mouse_x, gdouble mouse_y, gdouble safety)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);

    for (GList *elem = priv->edges; elem != NULL; elem = elem->next)
    {
        SplEdge* edge = elem->data;
        if (spl_edge_check_for_coords(edge, mouse_x, mouse_y, safety))
        {
            return edge;
        }
    }

    // No edge found (default behaviour)
    return NULL;
}

SplArea* focus_nav_get_adjacent(SplTileManager *context, SplArea *area, guint direction, gboolean inverse)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (context);

    if (direction == SPL_VERTICAL)
    {
        if (inverse) // Going Down
        {
            // Prefers top left
            SplVertex* vert = area->bl;

            // Find area where vert is tl
            for (GList *elem = priv->areas; elem != NULL; elem = elem->next)
            {
                SplArea *iter = elem->data;
                if (spl_vertex_is_equal (vert, iter->tl))
                    return iter;
            }
        }
        else // Going Up
        {
            // Prefers bottom left
            SplVertex* vert = area->tl;

            // Find area where vert is bl
            for (GList *elem = priv->areas; elem != NULL; elem = elem->next)
            {
                SplArea *iter = elem->data;
                if (spl_vertex_is_equal (vert, iter->bl))
                    return iter;
            }
        }
    }
    else if (direction == SPL_HORIZONTAL)
    {
        if (inverse) // Going Right
        {
            // Prefers top left
            SplVertex* vert = area->tr;

            // Find area where vert is tl
            for (GList *elem = priv->areas; elem != NULL; elem = elem->next)
            {
                SplArea *iter = elem->data;
                if (spl_vertex_is_equal (vert, iter->tl))
                    return iter;
            }
        }
        else // Going Left
        {
            // Prefers top right
            SplVertex* vert = area->tl;

            // Find area where vert is tr
            for (GList *elem = priv->areas; elem != NULL; elem = elem->next)
            {
                SplArea *iter = elem->data;
                if (spl_vertex_is_equal (vert, iter->tr))
                    return iter;
            }
        }
    }
    else
        {
        g_error("Invalid direction");
        return NULL;
    }

    return NULL;
}

GList *
spl_tile_manager_get_areas(SplTileManager *self)
{
    SplTileManagerPrivate *priv = spl_tile_manager_get_instance_private (self);
    return priv->areas;
}

// Workflow
//
// # Assumptions
// All calculations will be done in screen space coordinates, which will be
// defined to be between 0 and 100 (guint).
//
// # Splitting
// Find direction of split (vertical/horizontal). Find the factor of the split
// from either left to right or top to bottom (e.g. 0.6 means that the new tile
// will appear on the right for a vertical split). Take the area and resize it
// to give it its new, smaller allocation. Create a new area in the left over
// space. Fix edges and vertices and add them to the corresponding linked lists.
// Finally, clean up vertices and edges.
//
// # Joining
// Get two areas. Check that they are eligible for joining (vertices match up
// and areas share an edge). Remove the second area, and resize the
// first area to take up its entire allocation. Clean up vertices and edges.
//
// # Inserting
// Get any edge. Find the orientation of the edge. Get areas adjacent to the
// edge. Find the total size of the two areas, and divide it equally between
// the three areas. Resize and reposition the newly inserted area. It is the
// calling function's responsiblity to get connected edges. Alternatively, they
// can call the insert_connected variant of the function.
//
// # Removing
// Find the orientational preference (default is vertical). Firstly, look at
// area edges and see if there is an adjacent area with the same height/width
// (depending on preference). If there is, resize that area to take up the
// combined allocation of both. If there are connected edges with the same
// height/width, then allow that group to fill the removed area. If no candidate
// area can be found, then remove anyway and resize horizontally and vertically
// to take up the remaining space.
//
// # Swap
// This simply takes two areas and swaps their vertices and edges.
//
// # Safety
// Rather than have complicated clean up functions like Blender, and to reduce
// complexity, each function must be responsible for cleaning up any vertices
// it touches. This ensures that functions use computing power appropriately.
