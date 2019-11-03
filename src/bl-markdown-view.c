/* bl-markdown-view.c
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

#include "bl-markdown-view.h"
#include <cmark.h>

struct _BlMarkdownView
{
    GtkTextView parent_instance;
    GtkTextTag *font_tag;
    PangoFontDescription *font;
};

G_DEFINE_TYPE(BlMarkdownView, bl_markdown_view, GTK_TYPE_TEXT_VIEW);

static void
apply_heading_tag(GtkTextBuffer *buffer,
                  int           level,
                  GtkTextIter   *start,
                  GtkTextIter   *end)
{
    if (level == 1) {
        gtk_text_buffer_apply_tag_by_name (buffer, "heading1", start, end);
    }
    else if (level == 2) {
        gtk_text_buffer_apply_tag_by_name (buffer, "heading2", start, end);
    }
    else if (level == 3) {
        gtk_text_buffer_apply_tag_by_name (buffer, "heading3", start, end);
    }
    else if (level == 4) {
        gtk_text_buffer_apply_tag_by_name (buffer, "heading4", start, end);
    }
    else if (level == 5) {
        gtk_text_buffer_apply_tag_by_name (buffer, "heading5", start, end);
    }
    else if (level == 6) {
        gtk_text_buffer_apply_tag_by_name (buffer, "heading6", start, end);
    }
}

static void
apply_tag(GtkTextBuffer   *buffer,
          cmark_node      *node,
          GtkTextIter     *start,
          GtkTextIter     *end)
{
    switch (cmark_node_get_type(node))
    {
        // All Headings (1-6)
        case CMARK_NODE_HEADING:
        {
            int level = cmark_node_get_heading_level(node);
            apply_heading_tag(buffer, level, start, end);
            break;
        }

        // Bold
        case CMARK_NODE_STRONG:
        {
            gtk_text_buffer_apply_tag_by_name (buffer, "bold", start, end);
            break;
        }

        // Italic
        case CMARK_NODE_EMPH:
        {
            gtk_text_buffer_apply_tag_by_name (buffer, "italic", start, end);
            break;
        }

        // Do nothing (for completeness)
        default:
        {
            break;
        }
    }
}

static void
update_font (BlMarkdownView *self)
{
    if (self->font)
    {
        g_object_set (G_OBJECT (self->font_tag),
                  "font-desc", self->font,
                  NULL);
    }
}

void
bl_markdown_view_set_font (BlMarkdownView *self, PangoFontDescription *font)
{
    self->font = font;
    update_font (self);
}

static void
highlight_buffer (GtkTextBuffer* buffer, BlMarkdownView* self)
{
    // Sanity checks
    g_assert(BL_IS_MARKDOWN_VIEW (self));

    // Log it
    g_debug("Highlighting Buffer");

    // Adapted from https://github.com/ali-rantakari/peg-markdown-highlight/blob/master/example_gtk2/gtkexample.c
    // Modified for gtk3 and cmark by Matthew Jakeman

    // Get start and end iterators for text buffer
    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);

    // Get text from GtkTextBuffer
    gchar* text;
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    // Get length
    int length = strlen(text);

    // Remove existing tags
    gtk_text_buffer_remove_all_tags(buffer, &start, &end);

    // Markdown Parsing
    cmark_node *document = cmark_parse_document((char *)text, length,
                                                 CMARK_OPT_DEFAULT | CMARK_OPT_SOURCEPOS);

    g_debug("%s", cmark_render_html(document, CMARK_OPT_DEFAULT));

    // Cmark Iterator loop
    cmark_event_type ev_type;
    cmark_iter *iter = cmark_iter_new(document);

    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        if (ev_type == CMARK_EVENT_ENTER) {
            cmark_node *cur = cmark_iter_get_node(iter);

            // Get cmark positioning
            const char* name = cmark_node_get_type_string(cur);
            int start_line = cmark_node_get_start_line (cur);
            int start_col = cmark_node_get_start_column (cur);
            int end_line = cmark_node_get_end_line (cur);
            int end_col = cmark_node_get_end_column (cur);
            g_debug("%s - Start (%d, %d) to End (%d, %d)", name, start_line, start_col, end_line, end_col);

            // GtkTextIter for start and end positions of nodes
            GtkTextIter node_start;
            GtkTextIter node_end;

            // Hacky code to work around cmark's strangeness
            // TODO: Migrate to a different library in the future
            // Or submit a patch to cmark?

            start_col = start_col == 0 ? 0 : start_col - 1;
            end_col = end_col == 0 ? 0 : end_col;
            start_line = start_line == 0 ? 0 : start_line - 1;
            end_line = end_line == 0 ? 0 : end_line - 1;

            // Get GtkTextBuffer offsets for each TextIter
            gtk_text_buffer_get_iter_at_line_index(buffer, &node_start,
                                                   start_line, start_col);
            gtk_text_buffer_get_iter_at_line_index(buffer, &node_end,
                                                   end_line, end_col);

#if 0
            // Get gtk positioning
            int gtk_start = gtk_text_iter_get_line (&node_start);
            int gtk_end = gtk_text_iter_get_line (&node_end);
            int gtk_start_col = gtk_text_iter_get_line_index (&node_start);
            int gtk_end_col = gtk_text_iter_get_line_index (&node_end);
            g_debug("gtk - Start (%d, %d) to End (%d, %d)\n", gtk_start, gtk_start_col, gtk_end, gtk_end_col);
#endif

            // Apply tag
            apply_tag (buffer, cur, &node_start, &node_end);

        }
    }

    // Update font tag
    gtk_text_buffer_apply_tag (buffer, self->font_tag, &start, &end);

    g_debug("\n\n");
}

static void
initialise_buffer (BlMarkdownView* self)
{
    // Sanity checks
    g_assert(BL_IS_MARKDOWN_VIEW (self));

    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self));

    // Create tags
    GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table (buffer);

    // TODO: Find a better way of telling if the buffer has been initialised
    // We should probably move this into BlDocument itself to handle multiple
    // file types outside of just markdown.
    if (gtk_text_tag_table_lookup (tag_table, "heading1") == NULL)
    {
        // PSA: `create_tag` needs to be NULL terminated, otherwise
        // everything breaks, in a very painful way.

        // Tag: Heading 1
        gtk_text_buffer_create_tag(buffer, "heading1",
                                   "scale", (double)2,
                                   NULL);
        // Tag: Heading 2
        gtk_text_buffer_create_tag(buffer, "heading2",
                                   "foreground", "#d70022",
                                   "scale", (double)1.5,
                                   NULL);

        // Tag: Heading 3
        gtk_text_buffer_create_tag(buffer, "heading3",
                                   "foreground", "#0a84ff",
                                   "scale", (double)1.4,
                                   NULL);

        // Tag: Heading 4
        gtk_text_buffer_create_tag(buffer, "heading4",
                                   "foreground", "#ff9400",
                                   "scale", (double)1.3,
                                   NULL);

        // Tag: Heading 5
        gtk_text_buffer_create_tag(buffer, "heading5",
                                   "foreground", "#9400ff",
                                   "scale", (double)1.2,
                                   NULL);

        // Tag: Heading 6
        gtk_text_buffer_create_tag(buffer, "heading6",
                                   "foreground", "#363959",
                                   "weight", PANGO_WEIGHT_BOLD,
                                   NULL);

        // Tag: Bold
        gtk_text_buffer_create_tag(buffer, "bold",
                                   "weight", PANGO_WEIGHT_BOLD,
                                   NULL);

        // Tag: Italic
        gtk_text_buffer_create_tag(buffer, "italic",
                                   "style", PANGO_STYLE_ITALIC,
                                   NULL);

        // Tag: Widget Font Styling
        self->font_tag =
            gtk_text_buffer_create_tag (buffer, "user-style", NULL);

        gtk_text_tag_set_priority (self->font_tag, 0);
    }

    self->font_tag = gtk_text_tag_table_lookup (tag_table, "user-style");

    update_font (self);

    highlight_buffer(buffer, self);

    // Re-highlight on changed event
    g_signal_connect(G_OBJECT(buffer), "changed",
                     G_CALLBACK(highlight_buffer), self);
}

void bl_markdown_view_set_buffer (BlMarkdownView* self, GtkTextBuffer* buffer)
{
    gtk_text_view_set_buffer (GTK_TEXT_VIEW(self), buffer);
    initialise_buffer (self);
}

static void
bl_markdown_view_class_init (BlMarkdownViewClass* klass)
{

}

static void
bl_markdown_view_init (BlMarkdownView* self)
{
    initialise_buffer (self);
}
