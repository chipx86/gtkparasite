#ifndef _GTKPARASITE_PYTHON_MODULE_H_
#define _GTKPARASITE_PYTHON_MODULE_H_

#include <glib.h>


typedef void (*GtkParasitePythonLogger)(const char *text, gpointer user_data);

void gtkparasite_python_init(void);
void gtkparasite_python_run(const char *command,
							GtkParasitePythonLogger stdout_logger,
							GtkParasitePythonLogger stderr_logger,
							gpointer user_data);

#endif // _GTKPARASITE_PYTHON_MODULE_H_
