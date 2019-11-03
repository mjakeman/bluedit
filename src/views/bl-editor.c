/* bl-editor.c
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

#include "bl-editor.h"
#include "bl-multi-editor.h"
#include "bl-markdown-view.h"

struct _BlEditor
{
    BlView parent_instance;
    BlMarkdownView *text_view;
    GtkStack *stack;
    GtkLabel *file_label;
    BlMultiEditor *multi;
    GtkOverlay *overlay;
    GtkLabel *save_status;

    // Current Document
    BlDocument *document;
    gboolean saved;
};

G_DEFINE_TYPE (BlEditor, bl_editor, BL_TYPE_VIEW)

enum
{
    VIEW_CLOSE,
    ACTIVE_FOCUS,
	NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];

static void
bl_editor_class_init (BlEditorClass *klass)
{
    // GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    signals[VIEW_CLOSE] =
        g_signal_newv ("view-close",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 NULL /* closure */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 0     /* n_params */,
                 NULL  /* param_types */);

    signals[ACTIVE_FOCUS] =
        g_signal_newv ("active-focus",
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
update_save_label (BlDocument *doc,
                   BlEditor *editor)
{
    gboolean result = bl_document_unsaved_changes (doc);
    editor->saved = !result;

    if (editor->saved)
        gtk_widget_hide (GTK_WIDGET (editor->save_status));
    else
        gtk_widget_show (GTK_WIDGET (editor->save_status));
}

void focus_changed (GtkTextView* widget, BlEditor* editor)
{
    // Parameter sanity checks
    g_return_if_fail (GTK_IS_TEXT_VIEW(widget));
    g_return_if_fail (BL_IS_EDITOR(editor));

    // Whenever the user is using the text view, we (the BlEditor)
    // have focus in the application. We notify the BlMultiEditor
    // about the change, and it will assign us as 'active editor'.
    // This means that when a file is selected in the sidebar, it
    // will open in this BlEditor.

    // Log it
    g_debug ("Editor focus changed");

    // TODO: Cheap way of updating save-state.
    // We will certainly want to refactor this into BlDocument,
    // maybe using signals (e.g. on the DOC_SAVED signal, we could
    // update all editors)
    update_save_label (editor->document, editor);

    // Notify multi-editor
    g_signal_emit (editor, signals[ACTIVE_FOCUS], 0);
}

gboolean bl_editor_is_saved (BlEditor *editor)
{
    return editor->saved;
}

// Prompts the user to save the current file
// as a new file
void bl_editor_save_file_as (BlEditor *editor)
{
    BlDocument *doc = editor->document;

    if (doc == NULL)
        return;

    GtkWidget *dialogue;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialogue = gtk_file_chooser_dialog_new ("Save File",
                                            GTK_WINDOW(gtk_widget_get_toplevel (GTK_WIDGET(editor))),
                                            action, "_Cancel", GTK_RESPONSE_CANCEL,
                                            "_Save", GTK_RESPONSE_ACCEPT,
                                            NULL);

    chooser = GTK_FILE_CHOOSER (dialogue);

    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);
    res = gtk_dialog_run (GTK_DIALOG (dialogue));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename (chooser);

        GFile *file = g_file_new_for_path (filename);
        gchar* contents = bl_document_get_contents(doc);
        gint len = strlen(contents);
        g_file_replace_contents (file, contents, len, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, NULL);
        editor->document = bluedit_window_open_document_from_file (BLUEDIT_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET (editor))), file);

        g_free (filename);
    }

    gtk_widget_destroy (dialogue);
}

struct Transition {
    GtkWidget *widget;
    time_t start;
    guint delay;
    guint progress;
    guint target;
};

static gboolean
update_transition (struct Transition *timeout)
{
    if (timeout->progress >= timeout->target)
    {
        // Destroy the widget and timeout
        g_debug ("Transition Finished: Cleaning Up");
        gtk_widget_destroy (timeout->widget);
        g_free (timeout);
        return FALSE;
    }

    if (difftime (time(NULL), timeout->start) >= timeout->delay)
    {
        gdouble opacity = ((double)timeout->progress / timeout->target) * 1.0f;
        gtk_widget_set_opacity (timeout->widget, 1.0f - opacity);
        timeout->progress++;
    }

    return TRUE;
}

static void
create_transition (GtkWidget *widget,
                   guint      delay,
                   gdouble    duration)
{
    struct Transition *timeout = (struct Transition*)g_malloc (sizeof (struct Transition));
    timeout->widget = widget;
    timeout->delay = delay;
    timeout->start = time(NULL);
    timeout->progress = 0;
    timeout->target = (guint)((duration * 1000) / 10);

    g_timeout_add (10, (GSourceFunc)update_transition, timeout);
}

// Saves the file currently loaded in
// the editor. If no file is set, it will
// save as.
void bl_editor_save_file (BlEditor *editor)
{
    BlDocument *doc = editor->document;

    if (doc == NULL)
        return;

    GFile* file = bl_document_get_file (doc);

    // If file is none (i.e. Untitled file), then we will save as
    if (file == NULL)
    {
        bl_editor_save_file_as (editor);
        return;
    }

    gchar* contents = bl_document_get_contents(doc);
    gint len = strlen(contents);
    g_file_replace_contents (file, contents, len, NULL, TRUE, G_FILE_CREATE_NONE, NULL, NULL, NULL);

    // Update save status
    editor->saved = TRUE;
    bl_document_update_save_hash (doc);
    gtk_widget_hide (GTK_WIDGET (editor->save_status));

    // Show Overlay
    GtkWidget *label = gtk_label_new("File Saved");
    gtk_widget_set_valign (label, GTK_ALIGN_START);
    helper_set_widget_css_class (label, "save-label");
    gtk_overlay_add_overlay (editor->overlay, label);
    gtk_widget_show (label);

    create_transition (label, 1, 0.5);
}

// Close the active editor, unset self->document
// It does *not* close the file from the whole programme,
// which is the responsiblity of the caller
void bl_editor_close_file (BlEditor *self)
{
    // TODO: Load another file instead of closing?
    gtk_stack_set_visible_child_name (self->stack, "open-prompt");
    gtk_label_set_text (self->file_label, "No Open Files");
    bl_view_remove_decoration_style (BL_VIEW (self), "active-editor");
    self->document = NULL;
    self->saved = TRUE;
}

static void
cb_changed (GtkTextBuffer *doc,
            BlEditor      *editor)
{
    // BlDocument is subclassed from GtkTextBuffer
    g_return_if_fail (BL_IS_DOCUMENT (doc));
    update_save_label (BL_DOCUMENT (doc), editor);
}

void bl_editor_load_file(BlEditor* self, BlDocument* document)
{
    // Parameter sanity check
    g_return_if_fail (BL_IS_EDITOR (self));
    g_return_if_fail (BL_IS_DOCUMENT (document));

    // Check if file is open
    if (strcmp(gtk_stack_get_visible_child_name(self->stack), "open-prompt") == 0)
    {
        gtk_stack_set_visible_child_name (self->stack, "edit-mode");
    }

    BlMarkdownView* view = self->text_view;
    GtkTextBuffer *text = bl_document_get_buffer (document);
    g_return_if_fail (GTK_IS_TEXT_BUFFER (text));
    bl_markdown_view_set_buffer (view, text);
    self->document = document;

    // Save Handling
    self->saved = TRUE;
    g_signal_connect (bl_document_get_buffer (document), "changed",
                      (GCallback)cb_changed, self);

    // Update Heading
    gtk_label_set_text (self->file_label, bl_document_get_basename (document));

    // Grab focus
    gtk_widget_grab_focus(GTK_WIDGET(self->text_view));
    bl_view_set_decoration_style (BL_VIEW (self), "active-editor");
}

BlDocument* bl_editor_get_document(BlEditor* self)
{
    g_assert(BL_IS_EDITOR(self));
    return self->document;
}

static void cb_drag_data(BlEditor* self, GdkDragContext* context, gint x, gint y,
                         GtkSelectionData* data, guint info, guint time, gpointer null_ptr)
{
    g_return_if_fail (BL_IS_EDITOR (self));

    // URI List
    if (info == BL_TARGET_URI)
    {
        // IMPORTANT: Only use the first uri in the list
        // and open the rest in the background
        gchar** array = gtk_selection_data_get_uris(data);
        GtkWidget* window = gtk_widget_get_toplevel(GTK_WIDGET(self));
        gboolean first_file = TRUE;

        for (gchar** i = array; *i != NULL; ++i)
        {
            GFile* file = g_file_new_for_uri (*i);
            BlDocument* doc = bluedit_window_open_document_from_file (BLUEDIT_WINDOW(window), file);

            if (first_file == TRUE)
            {
                bl_editor_load_file (self, doc);
                first_file = FALSE;
            }
        }

        gtk_drag_finish (context, TRUE, FALSE, time);
    }
    // BlDocument
    else if (info == BL_TARGET_DOC)
    {
        // Get document pointer from data
        // TODO: Clean this up, majorly
        BlDocument **doc = (void *) gtk_selection_data_get_data (data);
        if (BL_IS_DOCUMENT (*doc))
        {
            bl_editor_load_file (self, *doc);
            gtk_drag_finish (context, TRUE, FALSE, time);
        }
        else
        {
            g_critical ("BlDocument not valid");
        }
    }

    gtk_drag_finish (context, FALSE, FALSE, time);
}

static void
cb_on_destroy (GtkWidget *object, gpointer null_ptr)
{
    g_signal_emit (object, signals[VIEW_CLOSE], 0);
}

static void
cb_prop_toggled (GtkToggleButton *widget, GtkPopover *popover)
{
    gboolean active = gtk_toggle_button_get_active (widget);
    if (active)
    {
        gtk_popover_popup (popover);
        gtk_widget_show_all (GTK_WIDGET(popover));
    }
    else
        gtk_popover_popdown (popover);
}

static void
cb_popover_closed (GtkPopover *popover, GtkToggleButton *btn)
{
    gtk_toggle_button_set_active (btn, FALSE);
}

static void
cb_font_set (GtkFontButton *btn, BlEditor *self)
{
    PangoFontDescription *font = gtk_font_chooser_get_font_desc (GTK_FONT_CHOOSER (btn));
    bl_markdown_view_set_font (self->text_view, font);
}

static void
cb_save (GtkButton *btn, BlEditor *self)
{
    bl_editor_save_file (self);
}

static void
cb_save_as (GtkButton *btn, BlEditor *self)
{
    bl_editor_save_file_as (self);
}

static void
cb_word_wrap (GtkToggleButton *btn, BlEditor *self)
{
    gboolean state = gtk_toggle_button_get_active (btn);
    if (state)
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (self->text_view), GTK_WRAP_WORD);
    else
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (self->text_view), GTK_WRAP_NONE);
}

static void
cb_close (GtkButton *btn, BlEditor *self)
{
    // Remove the file from the progamme
    BlueditWindow *window = BLUEDIT_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET(self)));
    if (window != NULL)
        bluedit_window_close_document (window, self->document);
    else
        g_error ("Window is NULL");

    // Close the editor
    bl_editor_close_file (self);
}

static void
setup_popover (BlEditor *editor, GtkPopover *popover)
{
    // Popover Menu Box
    GtkWidget *popover_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    helper_set_widget_css_class (popover_box, "bl-editor-popover");

    // Title ("Document")
    GtkWidget *label = gtk_label_new ("Document");
    helper_set_widget_css_class (label, "subtitle");
    helper_set_widget_css_class (label, "dim-label");
    gtk_box_pack_start (GTK_BOX (popover_box), label, FALSE, FALSE, 0);

    // Font Styles
    GtkWidget *font_btn = gtk_font_button_new ();
    gtk_box_pack_start (GTK_BOX (popover_box), font_btn, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (font_btn), "font-set",
                      G_CALLBACK (cb_font_set), editor);

    GtkWidget *word_wrap = gtk_check_button_new_with_label ("Word Wrap");
    gtk_box_pack_start (GTK_BOX (popover_box), word_wrap, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (word_wrap), "toggled",
                      G_CALLBACK (cb_word_wrap), editor);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (word_wrap), TRUE);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (editor->text_view), GTK_WRAP_WORD);

    // Divider
    GtkWidget *div1 = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start (GTK_BOX (popover_box), div1, FALSE, FALSE, 0);

    // Saving
    GtkWidget *save = gtk_button_new_with_label ("Save");
    helper_set_widget_css_class (save, "flat");
    g_signal_connect (G_OBJECT (save), "clicked",
                      G_CALLBACK (cb_save), editor);

    GtkWidget *save_as = gtk_button_new_with_label ("Save As");
    helper_set_widget_css_class (save_as, "flat");
    g_signal_connect (G_OBJECT (save_as), "clicked",
                      G_CALLBACK (cb_save_as), editor);

    gtk_box_pack_start (GTK_BOX (popover_box), save, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (popover_box), save_as, FALSE, FALSE, 0);

    // Divider
    GtkWidget *div2 = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start (GTK_BOX (popover_box), div2, FALSE, FALSE, 0);

    GtkWidget *close_btn = gtk_button_new_with_label ("Close");
    helper_set_widget_css_class (close_btn, "flat");
    g_signal_connect (G_OBJECT (close_btn), "clicked",
                      G_CALLBACK (cb_close), editor);

    gtk_box_pack_start (GTK_BOX (popover_box), close_btn, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (popover), popover_box);
}

// Essentially 'continues' from bl_editor_init, but only after the
// editor instance has been initialised.
static void cb_on_realise(BlEditor* self)
{
    // Parameter sanity checks
    g_return_if_fail (BL_IS_EDITOR(self));

    // We've created a new BlEditor instance, so let's register
    // it with the BlMultiEditor singleton. This keeps track of
    // editor instances for us.
    GtkWidget* window = gtk_widget_get_toplevel(GTK_WIDGET(self));

    // Check that the window is real and usable
    g_assert(BLUEDIT_IS_WINDOW(window));

    // Get the multi editor singleton
    GObject* multi = bluedit_window_get_multi(BLUEDIT_WINDOW(window));
    self->multi = BL_MULTI_EDITOR(multi);

    // Check multi-editor
    g_assert(BL_IS_MULTI_EDITOR (multi));

    // Register this editor instance with the singleton
    bl_multi_editor_register (BL_MULTI_EDITOR(multi), self);

    // Drag and Drop
    GtkTargetList *list = gtk_target_list_new (NULL, 0);
    gtk_target_list_add (list, gdk_atom_intern_static_string ("BL_DOCUMENT"),
                         GTK_TARGET_SAME_APP, BL_TARGET_DOC);
    /*gtk_target_list_add (list, gdk_atom_intern_static_string ("text/uri-list"),
                         0, BL_TARGET_URI);
    gtk_target_list_add (list, gdk_atom_intern_static_string ("text/plain"),
                         0, BL_TARGET_TEXT);*/


    gtk_drag_dest_set(GTK_WIDGET(self), GTK_DEST_DEFAULT_ALL,
                      NULL, 0, GDK_ACTION_COPY);
    gtk_drag_dest_set_target_list (GTK_WIDGET (self), list);

    g_signal_connect(G_OBJECT(self), "drag-data-received",
                     G_CALLBACK(cb_drag_data), NULL);
}

static void
bl_editor_init (BlEditor* self)
{
    // Set saved
    self->saved = TRUE;

    // Stack
    GtkWidget* stack = gtk_stack_new();
    bl_view_set_contents (BL_VIEW(self), stack);
    self->stack = GTK_STACK(stack);

    GtkWidget* label = gtk_label_new("There are no open files. Drag "\
                                    "a file here to open.");
    gtk_stack_add_named (GTK_STACK(stack), label, "open-prompt");
    gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);

    // Edit Mode Overlay
    GtkWidget* overlay = gtk_overlay_new();
    self->overlay = GTK_OVERLAY(overlay);

    // Vertical box
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (overlay), box);
    gtk_stack_add_named (GTK_STACK(stack), overlay, "edit-mode");

    // Set default view
    gtk_stack_set_visible_child_name (GTK_STACK(stack), "open-prompt");

    // Scrolled window so we can have
    // scrollbars in the text view
    GtkWidget* scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    // The text view for inside the scrolled window
    GtkWidget* text_view = g_object_new(BL_TYPE_MARKDOWN_VIEW, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    self->text_view = BL_MARKDOWN_VIEW(text_view);
    gtk_widget_show_all (stack);

    // Set BlView header
    GtkWidget *header_widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    bl_view_set_menu (BL_VIEW (self), header_widget, TRUE);

    GtkWidget *file_label = gtk_label_new ("No Open Files");
    gtk_box_pack_start (GTK_BOX (header_widget), file_label, FALSE, FALSE, 0);

    // Large black dot for indicating 'unsaved changes'
    GtkWidget *save_status = gtk_label_new ("•");
    helper_set_widget_css_class (save_status, "save-status");
    gtk_box_pack_start (GTK_BOX (header_widget),
                        save_status, FALSE, FALSE, 0);

    GtkWidget *prop_button = gtk_toggle_button_new ();
    GtkWidget *menu_icon = gtk_image_new_from_icon_name ("open-menu-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image (GTK_BUTTON (prop_button), menu_icon);
    gtk_box_pack_end (GTK_BOX (header_widget), prop_button, FALSE, FALSE, 0);
    helper_set_widget_css_class (prop_button, "flat");

    // Properties Popover
    GtkWidget *popover = gtk_popover_new (prop_button);
    gtk_popover_set_position (GTK_POPOVER (popover), GTK_POS_BOTTOM);
    g_signal_connect (G_OBJECT (prop_button), "toggled",
                      G_CALLBACK (cb_prop_toggled), popover);
    g_signal_connect (G_OBJECT (popover), "closed",
                      G_CALLBACK (cb_popover_closed), prop_button);

    setup_popover (self, GTK_POPOVER(popover));

    gtk_widget_show_all (header_widget);
    gtk_widget_hide (save_status);
    self->file_label = GTK_LABEL (file_label);
    self->save_status = GTK_LABEL (save_status);

    // Rest of the initialisation is in the function `cb_on_realise`
    // as we need the widget to have been realised to get the toplevel
    // window (which we need for getting the multi editor singleton).
    g_signal_connect(G_OBJECT(self), "realize", G_CALLBACK(cb_on_realise), NULL);

    // Connect the grab-focus signal
    g_signal_connect (G_OBJECT(self->text_view), "grab-focus",
                      G_CALLBACK(focus_changed), self);

    // Connect the destroy signal
    g_signal_connect (G_OBJECT(self), "destroy",
                      G_CALLBACK(cb_on_destroy), NULL);
}
