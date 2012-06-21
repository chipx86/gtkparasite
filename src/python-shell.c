/*
 * Copyright (c) 2008-2009  Christian Hammond
 * Copyright (c) 2008-2009  David Trowbridge
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
#include <gdk/gdkkeysyms.h>
#include <string.h>

#include "python-hooks.h"
#include "python-shell.h"

#define MAX_HISTORY_LENGTH 20

#define PARASITE_PYTHON_SHELL_GET_PRIVATE(obj) \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj), PARASITE_TYPE_PYTHON_SHELL, \
                                 ParasitePythonShellPrivate))

typedef struct
{
    GtkWidget *textview;

    GtkTextMark *scroll_mark;
    GtkTextMark *line_start_mark;

    GQueue *history;
    GList *cur_history_item;

    GString *pending_command;
    gboolean in_block;

} ParasitePythonShellPrivate;

enum
{
    LAST_SIGNAL
};


/* Widget functions */
static void parasite_python_shell_finalize(GObject *obj);

/* Python integration */
static void parasite_python_shell_write_prompt(GtkWidget *python_shell);
static char *parasite_python_shell_get_input(GtkWidget *python_shell);

/* Callbacks */
static gboolean parasite_python_shell_key_press_cb(GtkWidget *textview,
                                                   GdkEventKey *event,
                                                   GtkWidget *python_shell);


static GtkVBoxClass *parent_class = NULL;
//static guint signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE(ParasitePythonShell, parasite_python_shell, GTK_TYPE_VBOX);


static void
parasite_python_shell_class_init(ParasitePythonShellClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    parent_class = g_type_class_peek_parent(klass);

    object_class->finalize = parasite_python_shell_finalize;

    g_type_class_add_private(klass, sizeof(ParasitePythonShellPrivate));
}

static void
parasite_python_shell_init(ParasitePythonShell *python_shell)
{
    ParasitePythonShellPrivate *priv =
        PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);
    GtkWidget *swin;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    PangoFontDescription *font_desc;

    priv->history = g_queue_new();

    gtk_box_set_spacing(GTK_BOX(python_shell), 6);

    swin = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(swin);
    gtk_box_pack_start(GTK_BOX(python_shell), swin, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swin),
                                        GTK_SHADOW_IN);

    priv->textview = gtk_text_view_new();
    gtk_widget_show(priv->textview);
    gtk_container_add(GTK_CONTAINER(swin), priv->textview);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv->textview), TRUE);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(priv->textview), 3);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(priv->textview), 3);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(priv->textview), 3);

    g_signal_connect(G_OBJECT(priv->textview), "key_press_event",
                     G_CALLBACK(parasite_python_shell_key_press_cb),
                     python_shell);

    /* Make the textview monospaced */
    font_desc = pango_font_description_from_string("monospace");
    pango_font_description_set_size(font_desc, 8 * PANGO_SCALE);
    gtk_widget_modify_font(priv->textview, font_desc);
    pango_font_description_free(font_desc);

    /* Create the end-of-buffer mark */
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));
    gtk_text_buffer_get_end_iter(buffer, &iter);
    priv->scroll_mark = gtk_text_buffer_create_mark(buffer, "scroll_mark",
                                                    &iter, FALSE);

    /* Create the beginning-of-line mark */
    priv->line_start_mark = gtk_text_buffer_create_mark(buffer,
                                                        "line_start_mark",
                                                        &iter, TRUE);

    /* Register some tags */
    gtk_text_buffer_create_tag(buffer, "stdout", NULL);
    gtk_text_buffer_create_tag(buffer, "stderr",
                               "foreground", "red",
                               "paragraph-background", "#FFFFE0",
                               NULL);
    gtk_text_buffer_create_tag(buffer, "prompt",
                               "foreground", "blue",
                               NULL);

    parasite_python_shell_write_prompt(GTK_WIDGET(python_shell));
}

static void
parasite_python_shell_finalize(GObject *python_shell)
{
    ParasitePythonShellPrivate *priv =
        PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);

    g_queue_free(priv->history);
}

static void
parasite_python_shell_log_stdout(const char *text, gpointer python_shell)
{
    parasite_python_shell_append_text(PARASITE_PYTHON_SHELL(python_shell),
                                      text, "stdout");
}

static void
parasite_python_shell_log_stderr(const char *text, gpointer python_shell)
{
    parasite_python_shell_append_text(PARASITE_PYTHON_SHELL(python_shell),
                                      text, "stderr");
}

static void
parasite_python_shell_write_prompt(GtkWidget *python_shell)
{
    ParasitePythonShellPrivate *priv =
        PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));
    GtkTextIter iter;
    const char *prompt = (priv->pending_command == NULL ? ">>> " : "... ");

    parasite_python_shell_append_text(PARASITE_PYTHON_SHELL(python_shell),
                                      prompt, "prompt");

    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_move_mark(buffer, priv->line_start_mark, &iter);
}

static void
parasite_python_shell_process_line(GtkWidget *python_shell)
{
    ParasitePythonShellPrivate *priv =
        PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);

    char *command = parasite_python_shell_get_input(python_shell);
    char last_char;

    parasite_python_shell_append_text(PARASITE_PYTHON_SHELL(python_shell),
                                      "\n", NULL);

    if (*command != '\0')
    {
        /* Save this command in the history. */
        g_queue_push_head(priv->history, command);
        priv->cur_history_item = NULL;

        if (g_queue_get_length(priv->history) > MAX_HISTORY_LENGTH)
            g_free(g_queue_pop_tail(priv->history));
    }

    last_char = command[MAX(0, strlen(command) - 1)];

    if (last_char == ':' || last_char == '\\' ||
        (priv->in_block && g_ascii_isspace(command[0])))
    {
        printf("in block.. %c, %d, %d\n",
               last_char, priv->in_block,
               g_ascii_isspace(command[0]));
        /* This is a multi-line expression */
        if (priv->pending_command == NULL)
            priv->pending_command = g_string_new(command);
        else
            g_string_append(priv->pending_command, command);

        g_string_append_c(priv->pending_command, '\n');

        if (last_char == ':')
            priv->in_block = TRUE;
    }
    else
    {
        if (priv->pending_command != NULL)
        {
            g_string_append(priv->pending_command, command);
            g_string_append_c(priv->pending_command, '\n');

            /* We're not actually leaking this. It's in the history. */
            command = g_string_free(priv->pending_command, FALSE);
        }

        parasite_python_run(command,
                            parasite_python_shell_log_stdout,
                            parasite_python_shell_log_stderr,
                            python_shell);

        if (priv->pending_command != NULL)
        {
            /* Now do the cleanup. */
            g_free(command);
            priv->pending_command = NULL;
            priv->in_block = FALSE;
        }
    }

    parasite_python_shell_write_prompt(python_shell);
}

static void
parasite_python_shell_replace_input(GtkWidget *python_shell,
                                    const char *text)
{
    ParasitePythonShellPrivate *priv =
        PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);

    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));
    GtkTextIter start_iter;
    GtkTextIter end_iter;

    gtk_text_buffer_get_iter_at_mark(buffer, &start_iter,
                                     priv->line_start_mark);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);

    gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
    gtk_text_buffer_insert(buffer, &end_iter, text, -1);
}

static char *
parasite_python_shell_get_input(GtkWidget *python_shell)
{
    ParasitePythonShellPrivate *priv =
        PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));
    GtkTextIter start_iter;
    GtkTextIter end_iter;

    gtk_text_buffer_get_iter_at_mark(buffer, &start_iter,
                                     priv->line_start_mark);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);

    return gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, FALSE);
}

static const char *
parasite_python_shell_get_history_back(GtkWidget *python_shell)
{
    ParasitePythonShellPrivate *priv =
        PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);

    if (priv->cur_history_item == NULL)
    {
        priv->cur_history_item = g_queue_peek_head_link(priv->history);

        if (priv->cur_history_item == NULL)
            return "";
    }
    else if (priv->cur_history_item->next != NULL)
        priv->cur_history_item = priv->cur_history_item->next;

    return (const char *)priv->cur_history_item->data;
}

static const char *
parasite_python_shell_get_history_forward(GtkWidget *python_shell)
{
    ParasitePythonShellPrivate *priv =
        PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);

    if (priv->cur_history_item == NULL || priv->cur_history_item->prev == NULL)
    {
        priv->cur_history_item = NULL;
        return "";
    }

    priv->cur_history_item = priv->cur_history_item->prev;

    return (const char *)priv->cur_history_item->data;
}

static gboolean
parasite_python_shell_key_press_cb(GtkWidget *textview,
                                   GdkEventKey *event,
                                   GtkWidget *python_shell)
{
    if (event->keyval == GDK_KEY_Return)
    {
        parasite_python_shell_process_line(python_shell);
        return TRUE;
    }
    else if (event->keyval == GDK_KEY_Up)
    {
        parasite_python_shell_replace_input(python_shell,
            parasite_python_shell_get_history_back(python_shell));
        return TRUE;
    }
    else if (event->keyval == GDK_KEY_Down)
    {
        parasite_python_shell_replace_input(python_shell,
            parasite_python_shell_get_history_forward(python_shell));
        return TRUE;
    }
    else if (event->string != NULL)
    {
        ParasitePythonShellPrivate *priv =
            PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);
        GtkTextBuffer *buffer =
            gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));
        GtkTextMark *insert_mark = gtk_text_buffer_get_insert(buffer);
        GtkTextMark *selection_mark =
            gtk_text_buffer_get_selection_bound(buffer);
        GtkTextIter insert_iter;
        GtkTextIter selection_iter;
        GtkTextIter start_iter;
        gint cmp_start_insert;
        gint cmp_start_select;
        gint cmp_insert_select;

        gtk_text_buffer_get_iter_at_mark(buffer, &start_iter,
                                         priv->line_start_mark);
        gtk_text_buffer_get_iter_at_mark(buffer, &insert_iter, insert_mark);
        gtk_text_buffer_get_iter_at_mark(buffer, &selection_iter,
                                         selection_mark);

        cmp_start_insert = gtk_text_iter_compare(&start_iter, &insert_iter);
        cmp_start_select = gtk_text_iter_compare(&start_iter, &selection_iter);
        cmp_insert_select = gtk_text_iter_compare(&insert_iter,
                                                  &selection_iter);

        if (cmp_start_insert == 0 && cmp_start_select == 0 &&
            (event->keyval == GDK_KEY_BackSpace ||
             event->keyval == GDK_KEY_Left))
        {
            return TRUE;
        }
        if (cmp_start_insert <= 0 && cmp_start_select <= 0)
        {
            return FALSE;
        }
        else if (cmp_start_insert > 0 && cmp_start_select > 0)
        {
            gtk_text_buffer_place_cursor(buffer, &start_iter);
        }
        else if (cmp_insert_select < 0)
        {
            gtk_text_buffer_move_mark(buffer, insert_mark, &start_iter);
        }
        else if (cmp_insert_select > 0)
        {
            gtk_text_buffer_move_mark(buffer, selection_mark, &start_iter);
        }
    }

    return FALSE;
}

GtkWidget *
parasite_python_shell_new(void)
{
    return g_object_new(PARASITE_TYPE_PYTHON_SHELL, NULL);
}

void
parasite_python_shell_append_text(ParasitePythonShell *python_shell,
                                  const char *str,
                                  const char *tag)
{
    ParasitePythonShellPrivate *priv =
        PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);

    GtkTextIter end;
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));
    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);

    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_move_mark(buffer, mark, &end);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &end, str, -1, tag, NULL);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(priv->textview), mark,
                                 0, TRUE, 0, 1);
}

void
parasite_python_shell_focus(ParasitePythonShell *python_shell)
{
   gtk_widget_grab_focus(PARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell)->textview);
}

// vim: set et ts=4:
