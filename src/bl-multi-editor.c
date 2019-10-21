/* bl-multi-editor.c
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

#include "bl-multi-editor.h"
#include "views/bl-editor.h"

struct _BlMultiEditor
{
    GObject parent_instance;

    BlEditor* active;
    GList* editors;
};

G_DEFINE_TYPE (BlMultiEditor, bl_multi_editor, G_TYPE_OBJECT)

enum
{
	FOCUS_CHANGED,
	NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];

static void
bl_multi_editor_class_init (BlMultiEditorClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    signals[FOCUS_CHANGED] =
        g_signal_newv ("focus-changed",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 NULL /* closure */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 0     /* n_params */,
                 NULL  /* param_types */);
}

static void
cb_doc_closed (BlueditWindow *window, BlMultiEditor *self)
{
    g_debug ("Searching editors");
    GList *open_docs = bluedit_window_get_open_documents (window);
    for (GList *elem = self->editors; elem != NULL; elem = elem->next)
    {
        BlEditor *edit = BL_EDITOR (elem->data);
        BlDocument *doc = bl_editor_get_document (edit);
        g_debug ("%p", doc);
        g_debug ("%p", g_list_find (open_docs, doc));

        if (doc != NULL &&
            g_list_find (open_docs, doc) == NULL)
        {
            // Editor's document is not open,
            // go ahead and close it
            g_debug ("Closing File in Editor");
            bl_editor_close_file (edit);
        }
    }
}

BlMultiEditor* bl_multi_editor_new (BlueditWindow* window)
{
    BlMultiEditor* editor = g_object_new(BL_TYPE_MULTI_EDITOR, NULL);
    g_assert(BLUEDIT_IS_WINDOW(BLUEDIT_WINDOW(window)));
    // TODO: Use these for opening newly added files and
    // closing files that have been removed from the program
    // g_signal_connect(G_OBJECT(window), "doc-added", G_CALLBACK(update_combo), editor);
    g_signal_connect(G_OBJECT(window), "doc-closed",
                     G_CALLBACK(cb_doc_closed), editor);
    return editor;
}

void bl_multi_editor_open(BlMultiEditor* self, BlDocument* document)
{
    // Sanity checks
    g_assert (BL_IS_MULTI_EDITOR (self));
    g_assert (BL_IS_DOCUMENT (document));

    // Get active
    BlEditor* active = self->active;

    if (active == NULL)
    {
        g_debug("No active editor");
        return;
    }

    // Load file
    bl_editor_load_file(active, document);
}

BlDocument* bl_multi_get_active_document(BlMultiEditor* self)
{
    BlEditor* active = self->active;
    return bl_editor_get_document(active);
}

BlEditor* bl_multi_get_active_editor (BlMultiEditor* self)
{
    BlEditor* active = self->active;
    return active;
}

static void cb_editor_closed(BlEditor* editor, BlMultiEditor* multi)
{
    // Sanity checks
    g_return_if_fail (BL_IS_EDITOR (editor));
    g_return_if_fail (BL_IS_MULTI_EDITOR (multi));

    // This editor is no longer open and
    // must be removed from the editor list
    multi->editors = g_list_remove (multi->editors, editor);

    // Additionally, we must make sure it
    // is not the active editor
    if (multi->active == editor)
    {
        // Default behaviour is to assign
        // it to the first open editor in
        // the list
        GList* first = g_list_first (multi->editors);
        if (first != NULL)
        {
            multi->active = first->data;
        }
        else
        {
            g_critical ("No editors exist");
        }
    }

    // The BlEditor will clean up after itself
    // So nothing to do here
}

static void
cb_editor_focused(BlEditor* editor, BlMultiEditor* multi)
{
    // Sanity checks
    g_return_if_fail (BL_IS_EDITOR (editor));
    g_return_if_fail (BL_IS_MULTI_EDITOR (multi));

    // Assign active editor
    multi->active = editor;

    // Emit focus-changed signal
    g_signal_emit (multi, signals[FOCUS_CHANGED], 0);

    // Remove Focus Styling from other editors
    for (GList *elem = multi->editors; elem != NULL; elem = elem->next)
    {
        BlEditor *editor = BL_EDITOR (elem->data);
        bl_view_remove_decoration_style (BL_VIEW (editor), "active-editor");
    }

    // Set Focus Styling
    bl_view_set_decoration_style (BL_VIEW (editor), "active-editor");

    // Log
    g_debug ("Changed active editor");
}

void bl_multi_editor_register (BlMultiEditor* self, BlEditor* editor)
{
    // TODO: Sanity checks

    g_debug ("Editor Added - Connecting Signals");
    g_signal_connect (G_OBJECT(editor), "view-close",
                      G_CALLBACK(cb_editor_closed), self);
    g_signal_connect (G_OBJECT(editor), "active-focus",
                      G_CALLBACK(cb_editor_focused), self);

    self->editors = g_list_prepend (self->editors, editor);

    // Set editor as active on registration
    self->active = editor;
}

static void
bl_multi_editor_init (BlMultiEditor *self)
{
    // Initialisation
    self->editors = NULL;
}
