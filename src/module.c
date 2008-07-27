#include <glib.h>

#include "parasite.h"


int
gtk_module_init(gint argc, char *argv[])
{
    if (g_getenv("ENABLE_PARASITE"))
    {
        gtkparasite_window_create();
    }

    return 0;
}

// vim: set et ts=4:
