#include <glib.h>

#include "config.h"
#include "parasite.h"
#include "python-hooks.h"


int
gtk_module_init(gint argc, char *argv[])
{
#ifdef ENABLE_PYTHON
    parasite_python_init();
#endif

    gtkparasite_window_create();
    return FALSE;

    return 0;
}

// vim: set et sw=4 ts=4:
