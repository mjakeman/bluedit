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
GFile* bl_document_get_file(BlDocument* doc);
GtkTextBuffer* bl_document_get_buffer(BlDocument* doc);
gchar* bl_document_get_basename(BlDocument* doc);
gchar* bl_document_get_contents(BlDocument* doc);
gchar* bl_document_get_uri (BlDocument *doc);

G_END_DECLS
