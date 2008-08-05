#include <glib.h>
#include <gmodule.h>
#include <Python.h>


int
gtk_module_init(gint argc, char *argv[])
{
    PyObject *mdict;
    PyObject *func;
    PyObject *module;
    int retcode = 1;
    char *new_argv[] = {"gtkparasite", NULL};
    GModule *libpython;

    libpython = g_module_open(PY_LIB_LOC "/libpython" PYTHON_VERSION "."
                              G_MODULE_SUFFIX, 0);

    if (libpython == NULL)
    {
        g_warning("Unable to open libpython: %s", g_module_error());
    }

    Py_Initialize();

    if (PyErr_Occurred())
    {
        PyErr_Print();
        return 1;
    }

    PySys_SetArgv(1, new_argv);

    module = PyImport_ImportModule("pygtk");

    if (module == NULL)
    {
        PyErr_Print();
        return 1;
    }

    mdict = PyModule_GetDict(module);
    func = PyDict_GetItemString(mdict, "require");
    PyObject_CallObject(func,
                        Py_BuildValue("(S)", PyString_FromString("2.0")));

    if (PyErr_Occurred())
    {
        PyErr_Print();
        return 1;
    }

    module = PyImport_ImportModule("gtkparasite");

    if (module == NULL)
    {
        g_warning("Unable to load Python module gtkparasite\n");
    }
    else
    {
        mdict = PyModule_GetDict(module);
        func = PyDict_GetItemString(mdict, "run");

        if (func == NULL || !PyCallable_Check(func))
        {
            g_warning("Unable to locate function 'run' in Python module "
                      "gtkparasite\n");
        }
        else
        {
            PyObject *result = PyObject_CallFunction(func, "(s)",
                                                     g_get_application_name());

            if (result != NULL)
            {
                retcode = PyInt_AsLong(result);
                Py_XDECREF(result);
            }
        }

        Py_XDECREF(module);
    }

    if (Py_IsInitialized())
        Py_Finalize();

    return retcode;
}

// vim: set et ts=4:
