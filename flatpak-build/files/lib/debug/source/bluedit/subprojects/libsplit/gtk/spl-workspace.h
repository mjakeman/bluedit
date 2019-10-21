/* spl-workspace.h
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

#include <gtk/gtk.h>
#include "../spl-tile-manager.h"

G_BEGIN_DECLS

#define SPL_TYPE_WORKSPACE (spl_workspace_get_type())
G_DECLARE_FINAL_TYPE(SplWorkspace, spl_workspace, SPL, WORKSPACE, GtkContainer);

void spl_workspace_split_active (SplWorkspace *workspace,
                                 guint         direction,
                                 float         fac);

void spl_workspace_set_active (SplWorkspace *self, SplArea *area);

void spl_workspace_register_widget (SplWorkspace *workspace, SplArea *area, GtkWidget *widget);

G_END_DECLS
