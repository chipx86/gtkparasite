#include <glib.h>
#include <Python.h>


int
gtk_module_init(gint argc, char *argv[])
{
    PyObject *module_name;
    PyObject *module;
    int retcode = 1;

    Py_Initialize();

    module_name = PyString_FromString("gtkparasite");
    module = PyImport_Import(module_name);

    if (module == NULL)
    {
        g_warning("Unable to load Python module gtkparasite\n");
    }
    else
    {
        PyObject *mdict = PyModule_GetDict(module);
        PyObject *func = PyDict_GetItemString(mdict, "run");

        if (func == NULL || !PyCallable_Check(func))
        {
            g_warning("Unable to locate function 'run' in Python module "
                      "gtkparasite\n");
        }
        else
        {
            PyObject *result = PyObject_CallFunction(func, "");

            if (result != NULL)
            {
                retcode = PyInt_AsLong(result);
                Py_XDECREF(result);
            }
        }

        Py_XDECREF(module);
    }

    Py_XDECREF(module_name);
    Py_Finalize();

    return retcode;
}

// vim: set et ts=4:
