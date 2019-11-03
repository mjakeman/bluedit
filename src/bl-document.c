/* bl-document.c
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

#include "bl-document.h"

struct _BlDocument
{
    GtkTextBuffer parent_instance;
    GFile* file;
    gboolean untitled;
    guint hash;
};

G_DEFINE_TYPE (BlDocument, bl_document, GTK_TYPE_TEXT_BUFFER)

void
bl_document_set_file (BlDocument* document, GFile* file)
{
    // Load from buffer
    gchar *contents;
    gsize length;

    if (file == NULL)
    {
        g_error("File is invalid");
        return;
    }

    if (g_file_load_contents (file, NULL, &contents, &length, NULL, NULL))
    {
        gtk_text_buffer_set_text (GTK_TEXT_BUFFER (document), contents, length);
        g_free (contents);
    }

    document->file = file;
    document->untitled = FALSE;
}

BlDocument* bl_document_new ()
{
    BlDocument* doc = BL_DOCUMENT(g_object_new(BL_TYPE_DOCUMENT, NULL));
    return doc;
}

BlDocument* bl_document_new_from_file(GFile* file)
{
    g_assert(G_IS_FILE(file));
    BlDocument *doc = bl_document_new();
    bl_document_set_file (BL_DOCUMENT(doc), file);
    doc->untitled = FALSE;
    bl_document_update_save_hash (doc);
    // TODO: set file as a property so it can be loaded in initialisation

    return BL_DOCUMENT(doc);
}

BlDocument* bl_document_new_untitled ()
{
    BlDocument* doc = bl_document_new ();
    doc->untitled = TRUE;
    bl_document_update_save_hash (doc);
    return doc;
}

static void
bl_document_finalize (GObject *object)
{
    G_OBJECT_CLASS (bl_document_parent_class)->finalize (object);
}

static void
bl_document_class_init (BlDocumentClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = bl_document_finalize;
}

gboolean bl_document_is_untitled (BlDocument* doc)
{
    return doc->untitled;
}

gchar* bl_document_get_basename(BlDocument* doc)
{
    if (doc->untitled)
        return "Untitled Document";
    GFile* file = bl_document_get_file(doc);
    return g_file_get_basename (file);
}

gchar* bl_document_get_contents(BlDocument* doc)
{
    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (doc), &start);
    gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &end);
    return gtk_text_buffer_get_text (GTK_TEXT_BUFFER (doc), &start, &end, TRUE);
}

GFile* bl_document_get_file(BlDocument* doc)
{
    g_assert(BL_IS_DOCUMENT (doc));

    GFile* file = doc->file;
    if (doc->untitled)
    {
        g_debug("File has not been initialised");
        return NULL;
    }
    return file;
}

gchar* bl_document_get_uri (BlDocument *doc)
{
    g_assert(BL_IS_DOCUMENT (doc));

    if (doc->untitled == FALSE)
    {
        GFile* file = doc->file;
        return g_file_get_uri (file);
    }

    g_debug("File has not been initialised");
    return NULL;
}

GtkTextBuffer* bl_document_get_buffer (BlDocument* doc)
{
    return GTK_TEXT_BUFFER (doc);
}

static void
bl_document_init(BlDocument* self)
{
    self->file = NULL;
    // TODO: Load contents from file here
    // when the initial property is set
    // Currently done in `helper_set_file`
}

guint bl_document_get_current_hash (BlDocument *self)
{
    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (self), &start);
    gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (self), &end);

    // Get text from GtkTextBuffer
    gchar* text;
    text = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (self), &start, &end, FALSE);

    GString *str = g_string_new(text);
    return g_string_hash (str);
}

guint bl_document_get_save_hash (BlDocument *self)
{
    return self->hash;
}

void bl_document_set_save_hash (BlDocument *self, guint cmp)
{
    self->hash = cmp;
}

gboolean bl_document_unsaved_changes (BlDocument *self)
{
    guint current = bl_document_get_current_hash (self);
    guint saved = bl_document_get_save_hash (self);

    if (current == saved)
        return FALSE;
    else
        return TRUE;
}

void bl_document_update_save_hash (BlDocument *self)
{
    self->hash = bl_document_get_current_hash (self);
}
