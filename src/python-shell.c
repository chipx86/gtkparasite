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
    GtkWidget *entry;
    GtkWidget *textview;

} GtkParasitePythonShellPrivate;

enum
{
    LAST_SIGNAL
};


static void gtkparasite_python_shell_finalize(GObject *obj);
static void gtkparasite_python_shell_handle_stdout(GtkWidget *python_shell,
                                                   const char *str);
static void gtkparasite_python_shell_handle_stderr(GtkWidget *python_shell,
                                                   const char *str);
static void gtkparasite_python_shell_entry_activate_cb(GtkWidget *entry,
                                                       GtkWidget *python_shell);


static GtkVBoxClass *parent_class = NULL;
//static guint signals[LAST_SIGNAL] = {0};
static GtkWidget *cur_python_shell = NULL;

G_DEFINE_TYPE(GtkParasitePythonShell, gtkparasite_python_shell, GTK_TYPE_VBOX);


PyObject *
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

PyObject *
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

PyObject *
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
    GtkWidget *hbox;
    GtkWidget *label;
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
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv->textview), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->textview), FALSE);

    font_desc = pango_font_description_from_string("monospace");
    pango_font_description_set_size(font_desc, 8 * PANGO_SCALE);
    gtk_widget_modify_font(priv->textview, font_desc);

    hbox = gtk_hbox_new(FALSE, 6);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(python_shell), hbox, FALSE, FALSE, 0);

    label = gtk_label_new_with_mnemonic("_Python: ");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    priv->entry = gtk_entry_new();
    gtk_widget_show(priv->entry);
    gtk_box_pack_start(GTK_BOX(hbox), priv->entry, TRUE, TRUE, 0);
    gtk_widget_modify_font(priv->entry, font_desc);

    pango_font_description_free(font_desc);

    g_signal_connect(G_OBJECT(priv->entry), "activate",
                     G_CALLBACK(gtkparasite_python_shell_entry_activate_cb),
                     python_shell);

    // TODO: Set the font
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
gtkparasite_python_shell_entry_activate_cb(GtkWidget *entry,
                                           GtkWidget *python_shell)
{
    const char *command = gtk_entry_get_text(GTK_ENTRY(entry));

    cur_python_shell = python_shell;
    PyRun_SimpleString(command);
    cur_python_shell = NULL;

    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

GtkWidget *
gtkparasite_python_shell_new(void)
{
    return g_object_new(GTKPARASITE_TYPE_PYTHON_SHELL, NULL);
}

// vim: set et ts=4:
