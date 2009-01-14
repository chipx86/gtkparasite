#ifndef _GTKPARASITE_PYTHON_MODULE_H_
#define _GTKPARASITE_PYTHON_MODULE_H_

#include <glib.h>


typedef void (*ParasitePythonLogger)(const char *text, gpointer user_data);

void parasite_python_init(void);
void parasite_python_run(const char *command,
                         ParasitePythonLogger stdout_logger,
                         ParasitePythonLogger stderr_logger,
                         gpointer user_data);

#endif // _GTKPARASITE_PYTHON_MODULE_H_
