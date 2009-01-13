#include <gdk/gdkkeysyms.h>

#include <dlfcn.h>
#include <Python.h>
#include <pygobject.h>
#include <pygtk/pygtk.h>

#include "python-shell.h"

#define GTKPARASITE_PYTHON_SHELL_GET_PRIVATE(obj) \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj), GTKPARASITE_TYPE_PYTHON_SHELL, \
                                 GtkParasitePythonShellPrivate))

typedef struct
{
    GtkWidget *textview;

    GtkTextMark *scroll_mark;
    GtkTextMark *line_start_mark;

    int history_pos;
    int cursor_pos;

} GtkParasitePythonShellPrivate;

enum
{
    LAST_SIGNAL
};


/* Widget functions */
static void gtkparasite_python_shell_finalize(GObject *obj);

/* Python integration */
static void gtkparasite_python_shell_handle_stdout(GtkWidget *python_shell,
                                                   const char *str);
static void gtkparasite_python_shell_handle_stderr(GtkWidget *python_shell,
                                                   const char *str);
static char *gtkparasite_python_shell_get_input(GtkWidget *python_shell);

/* Callbacks */
static gboolean gtkparasite_python_shell_key_press_cb(GtkWidget *textview,
                                                      GdkEventKey *event,
                                                      GtkWidget *python_shell);


static GtkVBoxClass *parent_class = NULL;
//static guint signals[LAST_SIGNAL] = {0};
static GtkWidget *cur_python_shell = NULL;

G_DEFINE_TYPE(GtkParasitePythonShell, gtkparasite_python_shell, GTK_TYPE_VBOX);


static PyObject *
capture_stdout(PyObject *self, PyObject *args)
{
    char *str = NULL;

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    if (cur_python_shell != NULL)
        gtkparasite_python_shell_handle_stdout(cur_python_shell, str);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
capture_stderr(PyObject *self, PyObject *args)
{
    char *str = NULL;

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    if (cur_python_shell != NULL)
        gtkparasite_python_shell_handle_stderr(cur_python_shell, str);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
wrap_gobj(PyObject *self, PyObject *args)
{
    void *addr;
    GObject *obj;

    if (!PyArg_ParseTuple(args, "l", &addr))
        return NULL;

    if (!G_IS_OBJECT(addr))
        return NULL; // XXX

    obj = G_OBJECT(addr);

    if (!obj)
        return NULL; // XXX

    return pygobject_new(obj);
}

static PyMethodDef gtkparasite_python_methods[] = {
    {"capture_stdout", capture_stdout, METH_VARARGS, "Captures stdout"},
    {"capture_stderr", capture_stderr, METH_VARARGS, "Captures stderr"},
    {"gobj", wrap_gobj, METH_VARARGS, "Wraps a C GObject"},
    {NULL, NULL, 0, NULL}
};


static void
gtkparasite_python_shell_class_init(GtkParasitePythonShellClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    parent_class = g_type_class_peek_parent(klass);

    object_class->finalize = gtkparasite_python_shell_finalize;

    g_type_class_add_private(klass, sizeof(GtkParasitePythonShellPrivate));
}

static void
gtkparasite_python_shell_init_python(void)
{
    PyObject *module;

    /* This prevents errors such as "undefined symbol: PyExc_ImportError" */
    if (!dlopen(PYTHON_SHARED_LIB, RTLD_NOW | RTLD_GLOBAL))
    {
        g_error("%s\n", dlerror());
        return;
    }

    Py_Initialize();

    Py_InitModule("parasite", gtkparasite_python_methods);
    PyRun_SimpleString(
        "import parasite\n"
        "import sys\n"
        "\n"
        "class StdoutCatcher:\n"
        "    def write(self, str):\n"
        "        parasite.capture_stdout(str)\n"
        "\n"
        "class StderrCatcher:\n"
        "    def write(self, str):\n"
        "        parasite.capture_stderr(str)\n"
        "\n"
        "sys.stdout = StdoutCatcher()\n"
        "sys.stderr = StderrCatcher()\n"
    );

    init_pygobject();
    init_pygtk();

    module = PyImport_ImportModule("gobject");
}

static void
gtkparasite_python_shell_init(GtkParasitePythonShell *python_shell)
{
    GtkParasitePythonShellPrivate *priv =
        GTKPARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);
    GtkWidget *swin;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    PangoFontDescription *font_desc;

    gtkparasite_python_shell_init_python();

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

    g_signal_connect(G_OBJECT(priv->textview), "key_press_event",
                     G_CALLBACK(gtkparasite_python_shell_key_press_cb),
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
}

static void
gtkparasite_python_shell_finalize(GObject *obj)
{
    Py_Finalize();
}

static void
gtkparasite_python_shell_append_text(GtkWidget *python_shell,
                                     const char *str)
{
    GtkParasitePythonShellPrivate *priv =
        GTKPARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);

    GtkTextIter end;
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));
    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);

    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_move_mark(buffer, mark, &end);
    gtk_text_buffer_insert(buffer, &end, str, -1);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(priv->textview), mark,
                                 0, TRUE, 0, 1);
}

static void
gtkparasite_python_shell_handle_stdout(GtkWidget *python_shell,
                                       const char *str)
{
    gtkparasite_python_shell_append_text(python_shell, str);
}

static void
gtkparasite_python_shell_handle_stderr(GtkWidget *python_shell,
                                       const char *str)
{
    gtkparasite_python_shell_append_text(python_shell, str);
}

static void
gtkparasite_python_shell_process_line(GtkWidget *python_shell)
{
    GtkParasitePythonShellPrivate *priv =
        GTKPARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);

    char *command = gtkparasite_python_shell_get_input(python_shell);

    gtkparasite_python_shell_append_text(python_shell, "\n");

    priv->history_pos = 0;
    cur_python_shell = python_shell;
    PyRun_SimpleString(command);
    cur_python_shell = NULL;

    g_free(command);
}

static char *
gtkparasite_python_shell_get_input(GtkWidget *python_shell)
{
    GtkParasitePythonShellPrivate *priv =
        GTKPARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview));
    GtkTextIter start_iter;
    GtkTextIter end_iter;

    gtk_text_buffer_get_iter_at_mark(buffer, &start_iter,
                                     priv->line_start_mark);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);

    return gtk_text_buffer_get_slice(buffer, &start_iter, &end_iter, FALSE);
}

static gboolean
gtkparasite_python_shell_key_press_cb(GtkWidget *textview,
                                      GdkEventKey *event,
                                      GtkWidget *python_shell)
{
    if (event->keyval == GDK_Return)
    {
        gtkparasite_python_shell_process_line(python_shell);
        return TRUE;
    }
    else if (event->keyval == GDK_Up)
    {
#if 0
        gtkparasite_python_shell_replace_input(python_shell,
            gtkparasite_python_shell_get_history_back(python_shell));
#endif
        return TRUE;
    }
    else if (event->keyval == GDK_Down)
    {
#if 0
        gtkparasite_python_shell_replace_input(python_shell,
            gtkparasite_python_shell_get_history_forward(python_shell));
#endif
        return TRUE;
    }
    else if (event->string != NULL)
    {
        GtkParasitePythonShellPrivate *priv =
            GTKPARASITE_PYTHON_SHELL_GET_PRIVATE(python_shell);
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
gtkparasite_python_shell_new(void)
{
    return g_object_new(GTKPARASITE_TYPE_PYTHON_SHELL, NULL);
}

// vim: set et ts=4:
