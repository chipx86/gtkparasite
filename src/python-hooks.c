#include <dlfcn.h>
#include <Python.h>
#include <pygobject.h>
#include <pygtk/pygtk.h>

#include "python-hooks.h"


static GString *captured_stdout = NULL;
static GString *captured_stderr = NULL;


static PyObject *
capture_stdout(PyObject *self, PyObject *args)
{
    char *str = NULL;

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    g_string_append(captured_stdout, str);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
capture_stderr(PyObject *self, PyObject *args)
{
    char *str = NULL;

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    g_string_append(captured_stderr, str);

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

void
gtkparasite_python_init(void)
{
    PyObject *module;

    /* This prevents errors such as "undefined symbol: PyExc_ImportError" */
    if (!dlopen(PYTHON_SHARED_LIB, RTLD_NOW | RTLD_GLOBAL))
    {
        g_error("%s\n", dlerror());
        return;
    }

    captured_stdout = g_string_new("");
    captured_stderr = g_string_new("");

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
    );

    init_pygobject();
    init_pygtk();

    module = PyImport_ImportModule("gobject");
}

void
gtkparasite_python_run(const char *command,
                       GtkParasitePythonLogger stdout_logger,
                       GtkParasitePythonLogger stderr_logger,
                       gpointer user_data)
{
    PyRun_SimpleString("old_stdout = sys.stdout\n"
                       "old_stderr = sys.stderr\n"
                       "sys.stdout = StdoutCatcher()\n"
                       "sys.stderr = StderrCatcher()\n");

    PyRun_SimpleString(command);

    PyRun_SimpleString("sys.stdout = old_stdout\n"
                       "sys.stderr = old_stderr\n");

    if (stdout_logger != NULL)
        stdout_logger(captured_stdout->str, user_data);

    if (stderr_logger != NULL)
        stderr_logger(captured_stderr->str, user_data);

    g_string_erase(captured_stdout, 0, -1);
    g_string_erase(captured_stderr, 0, -1);
}

// vim: set et ts=4:
