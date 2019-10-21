/* bluedit-window.h
 *
 * Copyright 2019 Matthew Jakeman
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
 */

#pragma once

#include <gtk/gtk.h>
#include "helper.h"
#include "bl-document.h"

G_BEGIN_DECLS

#define BLUEDIT_TYPE_WINDOW (bluedit_window_get_type())

G_DECLARE_FINAL_TYPE (BlueditWindow, bluedit_window, BLUEDIT, WINDOW, GtkApplicationWindow)

GList* bluedit_window_get_open_documents (BlueditWindow* window);
GObject* bluedit_window_get_multi(BlueditWindow* self);
BlDocument* bluedit_window_open_document_from_file (BlueditWindow* window, GFile* file);
BlDocument* bluedit_window_open_document (BlueditWindow* window, BlDocument* document);
void bluedit_window_close_document (BlueditWindow* window, BlDocument* document);

G_END_DECLS
