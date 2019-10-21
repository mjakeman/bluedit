/* bl-multi-editor.h
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
#include "bl-document.h"
#include "bluedit-window.h"
#include "views/bl-editor.h"

G_BEGIN_DECLS

#define BL_TYPE_MULTI_EDITOR (bl_multi_editor_get_type())
G_DECLARE_FINAL_TYPE (BlMultiEditor, bl_multi_editor, BL, MULTI_EDITOR, GObject)

BlMultiEditor* bl_multi_editor_new (BlueditWindow* window);
void bl_multi_editor_register(BlMultiEditor* self, BlEditor* editor);
void bl_multi_editor_open(BlMultiEditor* self, BlDocument* document);
BlDocument* bl_multi_get_active_document(BlMultiEditor* self);
BlEditor* bl_multi_get_active_editor (BlMultiEditor* self);

G_END_DECLS
