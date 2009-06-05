#include <dlfcn.h>

#include "config.h"

#ifdef ENABLE_PYTHON
#include <Python.h>
#include <pygobject.h>
#include <pygtk/pygtk.h>
#endif // ENABLE_PYTHON

#include <signal.h>

#include "python-hooks.h"


static gboolean python_enabled = FALSE;

#ifdef ENABLE_PYTHON
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

static PyMethodDef parasite_python_methods[] = {
    {"capture_stdout", capture_stdout, METH_VARARGS, "Captures stdout"},
    {"capture_stderr", capture_stderr, METH_VARARGS, "Captures stderr"},
    {"gobj", wrap_gobj, METH_VARARGS, "Wraps a C GObject"},
    {NULL, NULL, 0, NULL}
};


static gboolean
is_blacklisted(void)
{
    const char *prgname = g_get_prgname();

    return (!strcmp(prgname, "gimp"));
}
#endif // ENABLE_PYTHON

void
parasite_python_init(void)
{
#ifdef ENABLE_PYTHON
    int res;
    struct sigaction old_sigint;
    PyObject *pygtk;

    if (is_blacklisted())
        return;

    /* This prevents errors such as "undefined symbol: PyExc_ImportError" */
    if (!dlopen(PYTHON_SHARED_LIB, RTLD_NOW | RTLD_GLOBAL))
    {
        g_error("%s\n", dlerror());
        return;
    }

    captured_stdout = g_string_new("");
    captured_stderr = g_string_new("");

    /* Back up and later restore SIGINT so Python doesn't steal it from us. */
    res = sigaction(SIGINT, NULL, &old_sigint);

    if (!Py_IsInitialized())
        Py_Initialize();

    res = sigaction(SIGINT, &old_sigint, NULL);

    Py_InitModule("parasite", parasite_python_methods);
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

    if (!pygobject_init(-1, -1, -1))
        return;

    pygtk = PyImport_ImportModule("gtk");

    if (pygtk != NULL)
    {
        PyObject *module_dict = PyModule_GetDict(pygtk);
        PyObject *cobject = PyDict_GetItemString(module_dict, "_PyGtk_API");

        /*
         * This seems to be NULL when we're running a PyGTK program.
         * We really need to find out why.
         */
        if (cobject != NULL)
        {
            if (PyCObject_Check(cobject))
                _PyGtk_API = (struct _PyGtk_FunctionStruct*)
                PyCObject_AsVoidPtr(cobject);
            else {
                PyErr_SetString(PyExc_RuntimeError,
                                "could not find _PyGtk_API object");
                return;
            }
        }
    } else {
        PyErr_SetString(PyExc_ImportError, "could not import gtk");
        return;
    }

    python_enabled = TRUE;
#endif // ENABLE_PYTHON
}

void
parasite_python_run(const char *command,
                    ParasitePythonLogger stdout_logger,
                    ParasitePythonLogger stderr_logger,
                    gpointer user_data)
{
#ifdef ENABLE_PYTHON
    PyGILState_STATE gstate;
    PyObject *module;
    PyObject *dict;
    PyObject *obj;

    gstate = PyGILState_Ensure();

    module = PyImport_AddModule("__main__");
    dict = PyModule_GetDict(module);

    PyRun_SimpleString("old_stdout = sys.stdout\n"
                       "old_stderr = sys.stderr\n"
                       "sys.stdout = StdoutCatcher()\n"
                       "sys.stderr = StderrCatcher()\n");

    obj = PyRun_String(command, Py_single_input, dict, dict);

    PyRun_SimpleString("sys.stdout = old_stdout\n"
                       "sys.stderr = old_stderr\n");

    if (stdout_logger != NULL)
        stdout_logger(captured_stdout->str, user_data);

    if (stderr_logger != NULL)
        stderr_logger(captured_stderr->str, user_data);

    // Print any returned object
    if (obj != NULL && obj != Py_None) {
       PyObject *repr = PyObject_Repr(obj);
       if (repr != NULL) {
           char *string = PyString_AsString(repr);

           stdout_logger(string, user_data);
           stdout_logger("\n", user_data);
        }

        Py_XDECREF(repr);
    }
    Py_XDECREF(obj);

    PyGILState_Release(gstate);
    g_string_erase(captured_stdout, 0, -1);
    g_string_erase(captured_stderr, 0, -1);
#endif // ENABLE_PYTHON
}

gboolean
parasite_python_is_enabled(void)
{
    return python_enabled;
}

// vim: set et sw=4 ts=4:
