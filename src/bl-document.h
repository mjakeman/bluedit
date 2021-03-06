/* bl-document.h
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

G_BEGIN_DECLS

#define BL_TYPE_DOCUMENT (bl_document_get_type())
G_DECLARE_FINAL_TYPE (BlDocument, bl_document, BL, DOCUMENT, GtkTextBuffer)

BlDocument* bl_document_new_from_file(GFile* file);
BlDocument* bl_document_new_untitled ();
GFile* bl_document_get_file(BlDocument* doc);
GtkTextBuffer* bl_document_get_buffer(BlDocument* doc);
gchar* bl_document_get_basename(BlDocument* doc);
gchar* bl_document_get_contents(BlDocument* doc);
gchar* bl_document_get_uri (BlDocument *doc);
void bl_document_set_file (BlDocument* document, GFile* file);
gboolean bl_document_is_untitled (BlDocument* doc);

// Low Level Hash Functions
void bl_document_set_save_hash (BlDocument *self, guint cmp);
guint bl_document_get_save_hash (BlDocument *self);
guint bl_document_get_current_hash (BlDocument *self);

// High Level Hash Functions
gboolean bl_document_unsaved_changes (BlDocument *self);
void bl_document_update_save_hash (BlDocument *self);

G_END_DECLS
