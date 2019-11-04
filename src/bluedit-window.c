/* bluedit-window.c
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

// Headers
#include "bl-workspace.h"
#include "bluedit-config.h"
#include "bluedit-window.h"
#include "bl-multi-editor.h"
#include "views/bl-editor.h"
#include "views/bl-view.h"

struct _BlueditWindow
{
    GtkApplicationWindow  parent_instance;

    GList* open_documents;
    BlMultiEditor* multi_editor;

    GtkTreeView* tree;

    /* Template widgets */
    GtkHeaderBar*       header_bar;
    GtkButton*          open_btn;
    GtkButton*          save_btn;
    GtkButton*          new_btn;
};

G_DEFINE_TYPE (BlueditWindow, bluedit_window, GTK_TYPE_APPLICATION_WINDOW)

enum
{
	DOC_ADDED,
    DOC_CLOSED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


static void
bluedit_window_class_init (BlueditWindowClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class, "/com/mattjakeman/bluedit/bluedit-window.ui");
    gtk_widget_class_bind_template_child (widget_class, BlueditWindow, header_bar);
    gtk_widget_class_bind_template_child (widget_class, BlueditWindow, open_btn);
    gtk_widget_class_bind_template_child (widget_class, BlueditWindow, save_btn);
    gtk_widget_class_bind_template_child (widget_class, BlueditWindow, new_btn);

    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    signals[DOC_ADDED] =
        g_signal_newv ("doc-added",
                 G_TYPE_FROM_CLASS (object_class),
                 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                 NULL /* closure */,
                 NULL /* accumulator */,
                 NULL /* accumulator data */,
                 NULL /* C marshaller */,
                 G_TYPE_NONE /* return_type */,
                 0     /* n_params */,
                 NULL  /* param_types */);

    signals[DOC_CLOSED] =
        g_signal_newv ("doc-closed",
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

BlDocument *bluedit_window_new_document (BlueditWindow *window)
{
    BlDocument *document = bl_document_new_untitled ();
    window->open_documents = g_list_append(window->open_documents, document);

    // Log it
    g_debug("Opened Document");

    // Emit signals so that other objects can update accordingly
    g_signal_emit (window, signals[DOC_ADDED], 0);

    // Success
    return document;
}

BlDocument* bluedit_window_open_document (BlueditWindow* window, BlDocument* document)
{
    // First, let's check if the file is valid
    GFile* file = bl_document_get_file (document);
    if (file == NULL)
    {
        // Something has gone wrong
        g_error("File is invalid");
        return FALSE;
    }

    // Get the list of open documents
    GList* open = bluedit_window_get_open_documents (window);

    // Now, see if the file is already open
    GList* elem;
    for (elem = open; elem != NULL; elem = elem->next)
    {
        if (bl_document_is_untitled (elem->data))
            continue;

        // This is the file corresponding to the iterator
        GFile* existing_file = bl_document_get_file (elem->data);

        g_debug("Current file: %s", g_file_get_path(file));
        g_debug("Existing file: %s", g_file_get_path(existing_file));

        // Compare file paths
        if (strcmp(g_file_get_path(file), g_file_get_path(existing_file)) == 0)
        {
            // Log
            g_debug("File already open");
            // TODO: Move to front?
            // Alternatively, reveal in Project Explorer

            // Return the already loaded document
            return elem->data;
        }
    }

    // Everything is fine, add to list
    window->open_documents = g_list_append(window->open_documents, document);

    // Log it
    g_debug("Opened Document");

    // Emit signals so that other objects can update accordingly
    g_signal_emit (window, signals[DOC_ADDED], 0);

    // Success
    return document;
}

// This function returns NULL if the document has already been loaded
// or if it is invalid
BlDocument* bluedit_window_open_document_from_file (BlueditWindow* window, GFile* file)
{
    g_assert(BLUEDIT_IS_WINDOW(window));
    g_assert(G_IS_FILE(file));

    // Create document from file
    BlDocument* document = bl_document_new_from_file(file);

    // Load document
    return bluedit_window_open_document(window, document);

}

void bluedit_window_close_document (BlueditWindow* window, BlDocument* document)
{
    window->open_documents = g_list_remove(window->open_documents, document);

    g_debug("Closed File");

    gtk_header_bar_set_subtitle (window->header_bar, "");

    // This instructs all open editors to check that their file
    // is still open. If not, they will close it.
    g_signal_emit (window, signals[DOC_CLOSED], 0);
}

static void
cb_run_open_dialogue(GtkButton *button, BlueditWindow *window)
{
    // We don't need GtkButton, this can be NULL
    g_assert(BLUEDIT_IS_WINDOW(window));

    GtkWidget *dialogue;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint result;

    dialogue = gtk_file_chooser_dialog_new("Open File",
                                           GTK_WINDOW(window), action,
                                           "Cancel", GTK_RESPONSE_CANCEL,
                                           "Open", GTK_RESPONSE_ACCEPT,
                                           NULL);

    result = gtk_dialog_run(GTK_DIALOG(dialogue));
    if (result == GTK_RESPONSE_ACCEPT)
    {
        char *path;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialogue);
        path = gtk_file_chooser_get_filename(chooser);
        GFile *file = g_file_new_for_path (path);
        bluedit_window_open_document_from_file (window, file);
        g_free(path);
    }

    gtk_widget_destroy(dialogue);
}

static void
cb_save_active(GtkButton* button, BlueditWindow* self)
{
    BlMultiEditor* multi = BL_MULTI_EDITOR (bluedit_window_get_multi (self));
    BlEditor *editor = bl_multi_get_active_editor (multi);
    bl_editor_save_file (editor);
}

static void
cb_new_file (GtkButton* button, BlueditWindow* self)
{
    g_debug ("Creating new file");
    BlDocument *doc = bluedit_window_new_document (self);
    BlMultiEditor *multi = bluedit_window_get_multi (self);
    bl_multi_editor_open (multi, doc);
}

GList* bluedit_window_get_open_documents (BlueditWindow* window)
{
    return window->open_documents;
}

void update_tree(BlueditWindow* window)
{
    g_debug("File added or removed");
    GList* documents = bluedit_window_get_open_documents(window);
    GtkTreeIter iter;
    GtkListStore* store = gtk_list_store_new(2,
                                             G_TYPE_STRING,
                                             BL_TYPE_DOCUMENT);

    // Iterate
    GList* elem;
    for (elem = documents; elem != NULL; elem = elem->next)
    {
        BlDocument* doc = elem->data;

        // This will either be the file name,
        // or "Untitled Document' depending on
        // whether the file actually exists.
        gchar* basename = bl_document_get_basename (doc);

        g_debug("%s", basename);

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            0, basename,
                            1, doc,
                            -1);
    }

    gtk_tree_view_set_model(window->tree, GTK_TREE_MODEL(store));
}

// Returns GObject to fix nasty circular dependency
GObject* bluedit_window_get_multi(BlueditWindow* self)
{
    return G_OBJECT(self->multi_editor);
}

static gboolean
cb_close_window (GtkWidget *widget,
                 GdkEvent  *event,
                 gpointer   null_ptr)
{
    BlueditWindow *self = BLUEDIT_WINDOW (widget);

    GList *docs = bluedit_window_get_open_documents (self);
    GList *unsaved = NULL;

    for (GList *elem = docs; elem != NULL; elem = elem->next)
    {
        if (bl_document_unsaved_changes (BL_DOCUMENT (elem->data)))
        {
            unsaved = g_list_prepend (unsaved, BL_DOCUMENT (elem->data));
        }
    }

    if (unsaved != NULL)
    {
        // Show dialogue
        guint length = g_list_length (unsaved);
        GtkWidget *dialogue, *content_area, *content_box;

        if (length == 1)
        {
            // One File
            BlDocument *doc = BL_DOCUMENT (unsaved->data);
            gchar *filename = bl_document_get_basename (doc);

            dialogue = gtk_message_dialog_new (GTK_WINDOW (self),
                                               GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_WARNING,
                                               GTK_BUTTONS_NONE,
                                               "Save changes to %s before closing?",
                                               filename);

            gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (dialogue),
                                                      "The file <b>%s</b> has not been saved. Would you like to save this file before closing?",
                                                      filename);
        }
        else
        {
            // Multiple files
            dialogue = gtk_message_dialog_new (GTK_WINDOW (self),
                                               GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_WARNING,
                                               GTK_BUTTONS_NONE,
                                               "There are %d documents with unsaved changes. Save changes before closing?",
                                               length);

            gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (dialogue),
                                                        "Select the files you would like to save:");

            content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialogue));
            content_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

            GtkWidget *list_box = gtk_list_box_new ();
            gtk_box_pack_start (GTK_BOX (content_box), list_box, FALSE, FALSE, 0);

            for (GList *elem = unsaved; elem != NULL; elem = elem->next)
            {
                BlDocument *doc = BL_DOCUMENT (elem->data);
                gchar *filename = bl_document_get_basename (doc);

                GtkWidget *check_box = gtk_check_button_new_with_label (filename);
                gtk_list_box_insert (GTK_LIST_BOX (list_box), check_box, -1);
            }

            gtk_container_add (GTK_CONTAINER (content_area), content_box);
            gtk_widget_show_all (content_box);
        }

        // Buttons
        GtkWidget *close_btn = gtk_dialog_add_button (GTK_DIALOG (dialogue), "_Discard Changes", GTK_RESPONSE_CLOSE);
        gtk_dialog_add_button (GTK_DIALOG (dialogue), "_Cancel", GTK_RESPONSE_CANCEL);
        gtk_dialog_add_button (GTK_DIALOG (dialogue), "_Save", GTK_RESPONSE_OK);
        gtk_dialog_set_default_response (GTK_DIALOG (dialogue), GTK_RESPONSE_CANCEL);
        helper_set_widget_css_class (close_btn, "destructive-action");

        int result = gtk_dialog_run (GTK_DIALOG (dialogue));
        gtk_widget_destroy (dialogue);

        switch (result)
        {
            case GTK_RESPONSE_OK:
            {
                // Save the files
                if (length == 1)
                {
                }
                return TRUE;
                break;
            }

            case GTK_RESPONSE_CLOSE:
                return FALSE;
                break;

            default:
            case GTK_RESPONSE_CANCEL:
                return TRUE;
                break;
        }
    }

    // Close the window
    return FALSE;
}

void cb_tree_changed(GtkTreeView* widget, GtkTreePath* path,
                     GtkTreeViewColumn* col, BlueditWindow* window)
{
    GtkTreeModel* store = gtk_tree_view_get_model(widget);

    BlMultiEditor* multi = BL_MULTI_EDITOR (bluedit_window_get_multi (window));

    GtkTreeIter iter;
    gchar* string;
    BlDocument* doc;

    gtk_tree_model_get_iter (store, &iter, path);

    // Terminate calls to tree_model_get with -1
    gtk_tree_model_get (store, &iter,
                       0, &string,
                       1, &doc,
                       -1);

    // Sanity checks
    g_assert(BL_IS_DOCUMENT(doc));

    // Open the document
    g_debug("Opening document");
    bl_multi_editor_open(multi, doc);
}

void cb_focus_changed(BlMultiEditor* multi, BlueditWindow* self)
{
    BlDocument* doc = bl_multi_get_active_document (multi);
    gchar* basename = bl_document_get_basename(doc);
    gtk_header_bar_set_subtitle (self->header_bar, basename);
}

// Set the active BlDocument as the drag data
static void cb_drag_data_get (GtkTreeView      *view,
                              GdkDragContext   *context,
                              GtkSelectionData *data,
                              guint             info,
                              guint             time,
                              gpointer          user_data)
{
    // Retrieve current selection from tree view
    GtkTreeSelection *selection = gtk_tree_view_get_selection (view);
    GtkTreeModel *model = gtk_tree_view_get_model (view);
    GtkTreeIter iter;
    gtk_tree_selection_get_selected (selection, &model, &iter);

    // Cast as BlDocument
    BlDocument *doc;
    gtk_tree_model_get(model, &iter, 1, &doc, -1); // 1 here is the BlDocument column

    // If this is not a BlDocument, then gracefully exit
    // TODO: Can we do this?
    if (!BL_IS_DOCUMENT (doc))
        gtk_drag_finish (context, FALSE, FALSE, time);

    // Info is the format of the drag and drop operation. The drag destination
    // decides which format it wants and sets this variable accordingly.
    switch (info)
    {
        case BL_TARGET_TEXT:
        {
            // Case 0: Plain Text
            // We simply get the contents of the editor and
            // set it as the drag data.
            // TODO: In future we may want to convert this
            // to rich text (e.g. with pandoc).
            gchar *contents = bl_document_get_contents (doc);
            gtk_selection_data_set_text (data, contents, -1);
            break;
        }
        case BL_TARGET_URI:
        {
            // Case 1: Uri List
            // This is the most likely target to be requested. Here
            // we provide a URI-link to the file to the requesting
            // program, so it can load it from disk. It is up to the
            // calling program to ensure the URI is valid.
            gchar *uri = bl_document_get_uri (doc);
            gchar *uris[] = { uri, NULL };
            gtk_selection_data_set_uris (data, uris);
            break;
        }
        case BL_TARGET_DOC:
        {
            // Case 2: Internal BlDocument object.
            // This will be used to drag and drop internal representations of
            // files within the programme. This is particularly useful in the
            // case of "Untitled Files", in which we cannot simply transfer
            // data by path on disk, as it does not yet exist.
            gtk_selection_data_set (data,
                                    gdk_atom_intern_static_string ("BL_DOCUMENT"),
                                    8,
                                    (void *)&doc,
                                    sizeof (gpointer));
        }
    }
}

static void
bluedit_window_init (BlueditWindow *self)
{
    // Init template
    gtk_widget_init_template (GTK_WIDGET (self));

    // Init open_documents GList
    self->open_documents = NULL;

    // Manager singleton for splitscreen editing
    // This does not implement the actual split screen
    // itself (that is done by libsplit and BlWorkspace),
    // but rather keeps track of the active editor and provides
    // many utility and state-related functions.
    BlMultiEditor* multi_editor = bl_multi_editor_new (self);
    self->multi_editor = multi_editor;

    // Bind focus-changed signal
    g_signal_connect (G_OBJECT(multi_editor), "focus-changed",
                      G_CALLBACK(cb_focus_changed), self);

    // Bind open file button
    g_signal_connect(G_OBJECT(self->open_btn), "clicked",
                     G_CALLBACK(cb_run_open_dialogue), self);

    // Bind save file button
    g_signal_connect(G_OBJECT(self->save_btn), "clicked",
                     G_CALLBACK(cb_save_active), self);

    // Bind new file button
    g_signal_connect(G_OBJECT(self->new_btn), "clicked",
                     G_CALLBACK(cb_new_file), self);

    // Bind on delete signal
    g_signal_connect(G_OBJECT(self), "delete-event",
                     G_CALLBACK(cb_close_window), NULL);

    // Create GUI
    GtkWidget* paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

    // Sidebar
    // TODO: This box only has one thing in it
    GtkWidget* sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget* tree = gtk_tree_view_new();
    gtk_box_pack_start(GTK_BOX(sidebar), tree, TRUE, TRUE, 0);
    self->tree = GTK_TREE_VIEW(tree);

    // Tree View rendering
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Open Files",
                                                       renderer,
                                                       "text", 0,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

    g_signal_connect(G_OBJECT(tree), "row-activated",
                     G_CALLBACK(cb_tree_changed), self);

    // Tree View drag and drop
    GtkTargetList *list = gtk_target_list_new (NULL, 0);
    gtk_target_list_add (list, gdk_atom_intern_static_string ("BL_DOCUMENT"),
                         GTK_TARGET_SAME_APP, BL_TARGET_DOC);
    gtk_target_list_add (list, gdk_atom_intern_static_string ("text/uri-list"),
                         0, BL_TARGET_URI);
    gtk_target_list_add (list, gdk_atom_intern_static_string ("text/plain"),
                         0, BL_TARGET_TEXT);

    gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW(tree),
                                            GDK_BUTTON1_MASK,
                                            NULL, 0,
                                            GDK_ACTION_COPY);

    gtk_drag_source_set_target_list (tree, list);


    // Enable drag and drop signals
    g_signal_connect(G_OBJECT(tree), "drag-data-get", G_CALLBACK(cb_drag_data_get), NULL);

    // Window file management signals
    g_signal_connect(G_OBJECT(self), "doc-added", G_CALLBACK(update_tree), NULL);
    g_signal_connect(G_OBJECT(self), "doc-closed", G_CALLBACK(update_tree), NULL);

    // Convenience wrapper around SplWorkspace from libsplit
    // This is fairly self contained and contains basically all of
    // the UI related code. See 'bl-workspace.c' for more.
    GtkWidget *workspace = g_object_new (BL_TYPE_WORKSPACE, NULL);

    // CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen (display);
    gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_css_provider_load_from_resource(GTK_CSS_PROVIDER(provider),"/com/mattjakeman/bluedit/style.css");

    // Add to dual panel
    gtk_paned_add1 (GTK_PANED(paned), sidebar);
    gtk_paned_add2 (GTK_PANED(paned), workspace);

    gtk_container_add(GTK_CONTAINER(self), paned);

    gtk_widget_show_all(GTK_WIDGET(self));
}
