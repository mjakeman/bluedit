/* bl-preferences.c
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

#include "bl-preferences.h"

struct _BlPreferences
{
    GtkWindow parent_instance;
};

G_DEFINE_TYPE (BlPreferences, bl_preferences, GTK_TYPE_WINDOW)

enum
{
	CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


BlPreferences *
bl_preferences_new (void)
{
    return g_object_new (BL_TYPE_PREFERENCES, NULL);
}

static void
bl_preferences_finalize (GObject *object)
{
    BlPreferences *self = (BlPreferences *)object;

    G_OBJECT_CLASS (bl_preferences_parent_class)->finalize (object);
}

static void
bl_preferences_class_init (BlPreferencesClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = bl_preferences_finalize;

    signals[CHANGED] =
        g_signal_newv ("changed",
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

typedef struct {
    GSettings *gsettings;
    gchar *key;
    BlPreferences *prefs;
} BlGSettingsKeyPair;






// Font Button
static void
font_btn_on_changed (GtkFontButton      *self,
                     BlGSettingsKeyPair *pair)
{
    const gchar *fontname = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (self));

    g_debug ("Updating key %s with value %s (type=font_btn)", pair->key, fontname);

    // Update GSettings
    g_settings_set_value (pair->gsettings, pair->key,
                          g_variant_new_string (fontname));

    g_signal_emit (G_OBJECT (pair->prefs), signals[CHANGED], 0);
}

static HdyActionRow *
action_row_with_font_btn (BlPreferences *prefs,
                          GSettings     *gsettings,
                          gchar         *title,
                          gchar         *key)
{
    GtkWidget *font_btn = gtk_font_button_new ();
    HdyActionRow *font = hdy_action_row_new ();
    hdy_preferences_row_set_title (font, title);
    gtk_container_add (GTK_CONTAINER (font), font_btn);
    hdy_action_row_set_activatable_widget (font, font_btn);
    helper_set_widget_css_class (font_btn, "bl-prefs-button");

    // GSettings
    GVariant *default_font = g_settings_get_value (gsettings, key);
    const gchar *font_str = g_variant_get_string (default_font, NULL);
    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (font_btn), font_str);

    // On Changed
    BlGSettingsKeyPair *pair = g_malloc (sizeof (BlGSettingsKeyPair));
    pair->gsettings = gsettings;
    pair->key = key;
    pair->prefs = prefs;
    g_signal_connect (G_OBJECT (font_btn), "font-set",
                      (GCallback)font_btn_on_changed, pair);

    return font;
}





// Spin Button
static void
spin_btn_on_changed (GtkSpinButton      *self,
                     BlGSettingsKeyPair *pair)
{
    gdouble value = gtk_spin_button_get_value (self);

    g_debug ("Updating key %s with value %f (type=spin_btn)", pair->key, value);

    // Update GSettings
    g_settings_set_value (pair->gsettings, pair->key,
                          g_variant_new_double (value));

    g_signal_emit (G_OBJECT (pair->prefs), signals[CHANGED], 0);
}

static HdyActionRow *
action_row_with_spin_btn (BlPreferences *prefs,
                          GSettings *gsettings,
                          gchar     *title,
                          gchar     *key,
                          gdouble    min,
                          gdouble    max,
                          gdouble    step)
{
    GtkWidget *spin_btn = gtk_spin_button_new_with_range (min, max, step);
    HdyActionRow *spin = hdy_action_row_new ();
    hdy_preferences_row_set_title (spin, title);
    gtk_container_add (GTK_CONTAINER (spin), spin_btn);
    hdy_action_row_set_activatable_widget (spin, spin_btn);
    helper_set_widget_css_class (spin_btn, "bl-prefs-button");

    // GSettings
    GVariant *default_value = g_settings_get_value (gsettings, key);
    gdouble value = g_variant_get_double (default_value);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin_btn), value);

    // On Changed
    BlGSettingsKeyPair *pair = g_malloc (sizeof (BlGSettingsKeyPair));
    pair->gsettings = gsettings;
    pair->key = key;
    pair->prefs = prefs;
    g_signal_connect (G_OBJECT (spin_btn), "value-changed",
                      (GCallback)spin_btn_on_changed, pair);

    return spin;
}





// Check Button
static void
switch_on_changed (GtkSwitch          *self,
                   gboolean            state,
                   BlGSettingsKeyPair *pair)
{
    g_debug ("Updating key %s with value %s (type=spin_btn)", pair->key, state ? "true" : "false");

    // Update GSettings
    g_settings_set_value (pair->gsettings, pair->key,
                          g_variant_new_boolean (state));

    gtk_switch_set_state (self, state);
    g_signal_emit (G_OBJECT (pair->prefs), signals[CHANGED], 0);
}

static HdyActionRow *
action_row_with_switch (BlPreferences *prefs,
                        GSettings *gsettings,
                        gchar     *title,
                        gchar     *key)
{
    GtkWidget *switch_btn = gtk_switch_new ();
    HdyActionRow *switch_row = hdy_action_row_new ();
    hdy_preferences_row_set_title (switch_row, title);
    gtk_container_add (GTK_CONTAINER (switch_row), switch_btn);
    hdy_action_row_set_activatable_widget (switch_row, switch_btn);
    helper_set_widget_css_class (switch_btn, "bl-prefs-switch");

    // GSettings
    GVariant *default_state = g_settings_get_value (gsettings, key);
    gboolean state = g_variant_get_boolean (default_state);
    gtk_switch_set_active (GTK_SWITCH (switch_btn), state);

    // On Changed
    BlGSettingsKeyPair *pair = g_malloc (sizeof (BlGSettingsKeyPair));
    pair->gsettings = gsettings;
    pair->key = key;
    pair->prefs = prefs;
    g_signal_connect (G_OBJECT (switch_btn), "state-set",
                      (GCallback)switch_on_changed, pair);

    return switch_row;
}


static void
setup_editor_page (BlPreferences *self, GSettings *gsettings)
{
    HdyPreferencesPage *page = hdy_preferences_page_new ();
    hdy_preferences_page_set_title (page, "Editor");
    hdy_preferences_page_set_icon_name (page, "document-edit-symbolic");

    // # Appearance Category
    HdyPreferencesGroup *group1 = hdy_preferences_group_new ();
    hdy_preferences_group_set_title (group1, "Default Appearance");
    hdy_preferences_group_set_description (group1, "The default look and feel of the editor.");

    HdyActionRow *font = action_row_with_font_btn (self, gsettings, "Default Font", "default-font");
    HdyActionRow *spacing = action_row_with_spin_btn (self, gsettings, "Line Spacing", "line-spacing", 0, 2, 0.1);
    HdyActionRow *wrap = action_row_with_switch (self, gsettings, "Word Wrap", "word-wrap");

    // Add to Appearance Category
    gtk_container_add (GTK_CONTAINER (group1), GTK_WIDGET (font));
    gtk_container_add (GTK_CONTAINER (group1), GTK_WIDGET (spacing));
    gtk_container_add (GTK_CONTAINER (group1), GTK_WIDGET (wrap));


    // # Themes Category
    // TODO;

    // # Add All Categories to Page
    gtk_container_add (GTK_CONTAINER (page), GTK_WIDGET (group1));
    //gtk_container_add (GTK_CONTAINER (page), GTK_WIDGET (group2));

    gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (page));
}


static void
bl_preferences_init (BlPreferences *self)
{
    // Window Settings
    GtkWindow *win = GTK_WINDOW (self);
    gtk_window_set_modal (win, TRUE);
    gtk_window_set_default_size (win, 720, 520);
    gtk_window_set_title (win, "Preferences");

    // TODO: GSettings **Tests** (Check gsettings are the GVariants we expect)
    GSettings *gsettings = g_settings_new ("com.mattjakeman.bluedit");

    // Setup Pages
    setup_editor_page (self, gsettings);


    gtk_widget_show_all (GTK_WIDGET (self));
}
