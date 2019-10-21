/* spl-tile-manager.h
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

#pragma once

// TODO: Use glib headers directly?
#include <gtk/gtk.h>

typedef struct
{
    gdouble x, y;
} SplVertex;

typedef struct
{
    SplVertex *v1, *v2;
} SplEdge;

typedef struct
{
    SplVertex *tl, *tr, *bl, *br;
    gpointer user_data;
    guint r, g, b;
} SplArea;

enum
{
    SPL_HORIZONTAL,
    SPL_VERTICAL,
    NUM_DIRECTIONS
};

guint SPL_DIRECTIONS[NUM_DIRECTIONS];

G_BEGIN_DECLS

#define SPL_TYPE_TILE_MANAGER (spl_tile_manager_get_type())
G_DECLARE_FINAL_TYPE (SplTileManager, spl_tile_manager, SPL, TILE_MANAGER, GObject)



// ============
// ------------
// Tile Manager
// ------------
// ============

// Create a new SplTileManager instance. Simple wrapper
// around g_object_new().
SplTileManager *  spl_tile_manager_new ();



// Essential setup function that *must* be called by the user
// in order for the library to function. This allows the
// caller to bind any signal callbacks they want before creating
// the first SplArea.
void              spl_tile_manager_create_initial (SplTileManager *self);




// Resize the SplTileManager to the current screen width/height, so that the
// caller does not need to keep track of this themselves.
void              spl_tile_manager_resize (SplTileManager *self,
                                           guint           width,
                                           guint           height);



// Get a linked list of every area in the SplTileManager. This is useful for
// iterating over all areas in implementations.
GList*            spl_tile_manager_get_areas (SplTileManager *self);



// Get any area in the SplTileManager. There is always guaranteed to be one,
// provided the caller has called `spl_tile_manager_create_initial`, so this
// can be used to retrieve the default area.
SplArea*          spl_tile_manager_get_any (SplTileManager *self);



//
guint spl_scale_width (SplTileManager *self, gdouble value);
guint spl_scale_height (SplTileManager *self, gdouble value);
gdouble spl_unscale_width (SplTileManager *self, guint mouse);
gdouble spl_unscale_height (SplTileManager *self, guint mouse);

// ==============
// --------------
// Edge Functions
// --------------
// ==============

// Move an edge. Update the given edge's vertices to
// resize the associated areas
gboolean  spl_edge_move (SplTileManager *self,
                         SplEdge        *edge,
                         gdouble         new_pos);

// Get the orientation of the edge. Either SPL_HORIZONTAL
// or SPL_VERTICAL
guint     spl_edge_get_orientation (SplEdge* edge);

// Gets the SplEdge at the coordinates, or returns NULL
SplEdge * spl_edge_get_for_coords (SplTileManager *self,
                                   gdouble         mouse_x,
                                   gdouble         mouse_y,
                                   gdouble         safety);

// Checks whether the given edge is at the given coords
gboolean  spl_edge_check_for_coords (SplEdge *edge,
                                     gdouble  mouse_x,
                                     gdouble  mouse_y,
                                     gdouble  safety);


// ==============
// --------------
// Area Functions
// --------------
// ==============

// Set the area's associated data
void      spl_area_set_userdata (SplArea  *area,
                                 gpointer  data);

// Gets the area's associated data
gpointer  spl_area_get_userdata (SplArea *area);

// Get the height of the area
gdouble   spl_area_get_height (SplArea *area);

// Get the width of the area
gdouble   spl_area_get_width (SplArea *area);

// Split the area into two distinct areas. If the operation
// succeeds, return the newly created SplArea, otherwise return
// NULL if they area could not be split
SplArea * spl_area_split (SplTileManager *self,
                          SplArea        *area,
                          guint           direction,
                          float           fac);

// Join the two areas into one area, deleting the second area. If
// the operation is successful, return TRUE, otherwise return FALSE.
gboolean  spl_area_join(SplTileManager *self, SplArea *keep, SplArea *join);

// Get the area at the given coordinates
SplArea * spl_area_get_for_coords (SplTileManager *self,
                                   gdouble         mouse_x,
                                   gdouble         mouse_y);

// Check if the area exists at the given coordinates
gboolean  spl_area_check_for_coords (SplArea *area,
                                     gdouble  mouse_x,
                                     gdouble  mouse_y);

// Debug
void print_areas (SplTileManager *self);
void print_area_single (SplArea* area);
void print_vertices (SplTileManager *self);
void print_vertex_single (SplVertex* vertex);

G_END_DECLS
