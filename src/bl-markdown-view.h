/* bl-markdown-view.h
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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BL_TYPE_MARKDOWN_VIEW (bl_markdown_view_get_type())
G_DECLARE_FINAL_TYPE(BlMarkdownView, bl_markdown_view, BL, MARKDOWN_VIEW, GtkTextView);

void bl_markdown_view_set_buffer (BlMarkdownView* self, GtkTextBuffer* buffer);
void bl_markdown_view_set_font (BlMarkdownView *self, const gchar *font_name);
void bl_markdown_view_set_line_spacing (BlMarkdownView *self, gdouble line_spacing);
G_END_DECLS
