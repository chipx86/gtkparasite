/*
 * Copyright (c) 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "css-editor.h"
#include "parasite.h"
#include <gtksourceview/gtksource.h>

enum
{
  COLUMN_ENABLED,
  COLUMN_NAME,
  COLUMN_USER,
  NUM_COLUMNS
};

typedef struct
{
  gboolean enabled;
  gboolean user;
} ParasiteCssEditorByContext;

struct _ParasiteCssEditorPrivate
{
  GtkWidget *toolbar;
  GtkSourceBuffer *text;
  GtkCssProvider *provider;
};

G_DEFINE_TYPE_WITH_PRIVATE (ParasiteCssEditor, parasite_csseditor, GTK_TYPE_BOX)

static void
disable_toggled (GtkToggleToolButton *button, ParasiteCssEditor *editor)
{
  if (gtk_toggle_tool_button_get_active (button))
    {
      gtk_style_context_remove_provider_for_screen (gdk_screen_get_default (),
                                                    GTK_STYLE_PROVIDER (editor->priv->provider));
    }
  else
    {
      gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                                 GTK_STYLE_PROVIDER (editor->priv->provider),
                                                 GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
}

static void
create_toolbar (ParasiteCssEditor *editor)
{
  GtkWidget *toolbar, *button;

  editor->priv->toolbar = g_object_new (GTK_TYPE_TOOLBAR,
                                        "icon-size", GTK_ICON_SIZE_SMALL_TOOLBAR,
                                        NULL);
  gtk_container_add (GTK_CONTAINER (editor), editor->priv->toolbar);

  button = g_object_new (GTK_TYPE_TOGGLE_TOOL_BUTTON,
                         "icon-name", "media-playback-pause",
                         "tooltip-text", "Disable this custom css",
                         NULL);
  g_signal_connect (button, "toggled", G_CALLBACK (disable_toggled), editor);
  gtk_container_add (GTK_CONTAINER (editor->priv->toolbar), button);
}

static void
apply_system_font (GtkWidget *widget)
{
  GSettings *s = g_settings_new ("org.gnome.desktop.interface");
  gchar *font_name = g_settings_get_string (s, "monospace-font-name");
  PangoFontDescription *font_desc = pango_font_description_from_string (font_name);

  gtk_widget_override_font (widget, font_desc);

  pango_font_description_free (font_desc);
  g_free (font_name);
  g_object_unref (s);
}

static void
text_changed (GtkTextBuffer *buffer, ParasiteCssEditor *editor)
{
  GtkTextIter start, end;
  char *text;

  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_remove_all_tags (buffer, &start, &end);

  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  gtk_css_provider_load_from_data (editor->priv->provider, text, -1, NULL);
  g_free (text);

  gtk_style_context_reset_widgets (gdk_screen_get_default ());
}

static void
show_parsing_error (GtkCssProvider    *provider,
                    GtkCssSection     *section,
                    const GError      *error,
                    ParasiteCssEditor *editor)
{
  GtkTextIter start, end;
  const char *tag_name;
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (editor->priv->text);

  gtk_text_buffer_get_iter_at_line_index (buffer,
                                          &start,
                                          gtk_css_section_get_start_line (section),
                                          gtk_css_section_get_start_position (section));
  gtk_text_buffer_get_iter_at_line_index (buffer,
                                          &end,
                                          gtk_css_section_get_end_line (section),
                                          gtk_css_section_get_end_position (section));

  if (g_error_matches (error, GTK_CSS_PROVIDER_ERROR, GTK_CSS_PROVIDER_ERROR_DEPRECATED))
    tag_name = "warning";
  else
    tag_name = "error";

  gtk_text_buffer_apply_tag_by_name (buffer, tag_name, &start, &end);
}

static void
create_text_widget (ParasiteCssEditor *editor)
{
  GtkWidget *sw, *view;
  GtkSourceLanguage *lang;
  const gchar *initial_text = "/*\n"
                              "You can type here any CSS rule recognized by GTK+.\n\n"
                              "Changes are applied instantly and globally, for the whole application.\n"
                              "You can temporarily disable this custom CSS by clicking on the \"Pause\" button above.\n\n"
                              "Happy hacking!\n"
                              "*/\n\n";

  editor->priv->text = gtk_source_buffer_new (NULL);
  lang = gtk_source_language_manager_get_language (gtk_source_language_manager_get_default (), "css");
  gtk_source_buffer_set_language (editor->priv->text, lang);
  gtk_text_buffer_set_text (GTK_TEXT_BUFFER (editor->priv->text), initial_text, -1);
  g_signal_connect (editor->priv->text, "changed", G_CALLBACK (text_changed), editor);

  gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (editor->priv->text),
                              "warning",
                              "underline", PANGO_UNDERLINE_SINGLE,
                              NULL);
  gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (editor->priv->text),
                              "error",
                              "underline", PANGO_UNDERLINE_ERROR,
                              NULL);

  sw = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                     "expand", TRUE,
                     NULL);
  gtk_container_add (GTK_CONTAINER (editor), sw);

  view = g_object_new (GTK_SOURCE_TYPE_VIEW,
                       "buffer", editor->priv->text,
                       "wrap-mode", GTK_WRAP_WORD,
                       "show-line-numbers", TRUE,
                       NULL);
  apply_system_font (view);
  gtk_container_add (GTK_CONTAINER (sw), view);
}

static void
create_provider (ParasiteCssEditor *editor)
{
  editor->priv->provider = gtk_css_provider_new ();
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (editor->priv->provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
  g_signal_connect (editor->priv->provider,
                    "parsing-error",
                    G_CALLBACK (show_parsing_error),
                    editor);
}

static void
parasite_csseditor_init (ParasiteCssEditor *editor)
{

  g_object_set (editor, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
  editor->priv = parasite_csseditor_get_instance_private (editor);

  create_toolbar (editor);
  create_provider (editor);
  create_text_widget (editor);
}

static void
parasite_csseditor_class_init (ParasiteCssEditorClass *klass)
{
}

GtkWidget *
parasite_csseditor_new ()
{
    return GTK_WIDGET (g_object_new (PARASITE_TYPE_CSSEDITOR, NULL));
}

// vim: set et sw=4 ts=4:
