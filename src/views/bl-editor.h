/* bl-editor.h
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
#include "helper.h"
#include "bl-document.h"
#include "bl-view.h"

G_BEGIN_DECLS

#define BL_TYPE_EDITOR (bl_editor_get_type())
G_DECLARE_FINAL_TYPE (BlEditor, bl_editor, BL, EDITOR, BlView)

void bl_editor_load_file(BlEditor* self, BlDocument* document);
BlDocument* bl_editor_get_document(BlEditor* self);
void bl_editor_save_file (BlEditor *editor);
void bl_editor_save_file_as (BlEditor *editor);
void bl_editor_close_file (BlEditor *self);
gboolean bl_editor_is_saved (BlEditor *editor);

G_END_DECLS
