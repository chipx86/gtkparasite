#ifndef _GTKPARASITE_PYTHON_SHELL_H_
#define _GTKPARASITE_PYTHON_SHELL_H_

typedef struct _GtkParasitePythonShell      GtkParasitePythonShell;
typedef struct _GtkParasitePythonShellClass GtkParasitePythonShellClass;

#include <gtk/gtk.h>

#define GTKPARASITE_TYPE_PYTHON_SHELL (gtkparasite_python_shell_get_type())
#define GTKPARASITE_PYTHON_SHELL(obj) \
		(G_TYPE_CHECK_INSTANCE_CAST((obj), GTKPARASITE_TYPE_PYTHON_SHELL, GtkParasitepython_shell))
#define GTKPARASITE_PYTHON_SHELL_CLASS(klass) \
		(G_TYPE_CHECK_CLASS_CAST((klass), GTKPARASITE_TYPE_PYTHON_SHELL, GtkParasitepython_shellClass))
#define GTKPARASITE_IS_PYTHON_SHELL(obj) \
		(G_TYPE_CHECK_INSTANCE_TYPE((obj), GTKPARASITE_TYPE_PYTHON_SHELL))
#define GTKPARASITE_IS_PYTHON_SHELL_CLASS(klass) \
		(G_TYPE_CHECK_CLASS_TYPE((klass), GTKPARASITE_TYPE_PYTHON_SHELL))
#define GTKPARASITE_PYTHON_SHELL_GET_CLASS(obj) \
		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTKPARASITE_TYPE_PYTHON_SHELL, GtkParasitepython_shellClass))


struct _GtkParasitePythonShell
{
	GtkVBox parent_object;

	void (*gtk_reserved1)(void);
	void (*gtk_reserved2)(void);
	void (*gtk_reserved3)(void);
	void (*gtk_reserved4)(void);
};

struct _GtkParasitePythonShellClass
{
	GtkVBoxClass parent_class;

	void (*gtk_reserved1)(void);
	void (*gtk_reserved2)(void);
	void (*gtk_reserved3)(void);
	void (*gtk_reserved4)(void);
};

G_BEGIN_DECLS

GType gtkparasite_python_shell_get_type(void);

GtkWidget *gtkparasite_python_shell_new(void);
void gtkparasite_python_shell_append_text(GtkWidget *python_shell,
										  const char *str,
										  const char *tag);

G_END_DECLS

#endif // _GTKPARASITE_PYTHON_SHELL_H_
