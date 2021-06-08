/* bl-toolbar.h
 *
 * Copyright 2021 Matthew Jakeman <mjakeman26@outlook.co.nz>
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

#define BL_TYPE_TOOLBAR (bl_toolbar_get_type())
G_DECLARE_FINAL_TYPE (BlToolbar, bl_toolbar, BL, TOOLBAR, GtkBox)

struct _BlToolbar
{
    GtkBox parent_instance;
    GtkButton *open_btn;
    GtkButton *save_btn;
    GtkButton *new_btn;
    GtkButton *menu_btn;
};

BlToolbar * bl_toolbar_new (void);

G_END_DECLS
