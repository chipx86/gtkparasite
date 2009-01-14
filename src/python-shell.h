#ifndef _PARASITE_PYTHON_SHELL_H_
#define _PARASITE_PYTHON_SHELL_H_

typedef struct _ParasitePythonShell      ParasitePythonShell;
typedef struct _ParasitePythonShellClass ParasitePythonShellClass;

#include <gtk/gtk.h>

#define PARASITE_TYPE_PYTHON_SHELL (parasite_python_shell_get_type())
#define PARASITE_PYTHON_SHELL(obj) \
		(G_TYPE_CHECK_INSTANCE_CAST((obj), PARASITE_TYPE_PYTHON_SHELL, ParasitePythonShell))
#define PARASITE_PYTHON_SHELL_CLASS(klass) \
		(G_TYPE_CHECK_CLASS_CAST((klass), PARASITE_TYPE_PYTHON_SHELL, ParasitePythonShellClass))
#define PARASITE_IS_PYTHON_SHELL(obj) \
		(G_TYPE_CHECK_INSTANCE_TYPE((obj), PARASITE_TYPE_PYTHON_SHELL))
#define PARASITE_IS_PYTHON_SHELL_CLASS(klass) \
		(G_TYPE_CHECK_CLASS_TYPE((klass), PARASITE_TYPE_PYTHON_SHELL))
#define PARASITE_PYTHON_SHELL_GET_CLASS(obj) \
		(G_TYPE_INSTANCE_GET_CLASS ((obj), PARASITE_TYPE_PYTHON_SHELL, ParasitePythonShellClass))


struct _ParasitePythonShell
{
	GtkVBox parent_object;

	void (*gtk_reserved1)(void);
	void (*gtk_reserved2)(void);
	void (*gtk_reserved3)(void);
	void (*gtk_reserved4)(void);
};

struct _ParasitePythonShellClass
{
	GtkVBoxClass parent_class;

	void (*gtk_reserved1)(void);
	void (*gtk_reserved2)(void);
	void (*gtk_reserved3)(void);
	void (*gtk_reserved4)(void);
};

G_BEGIN_DECLS

GType parasite_python_shell_get_type(void);

GtkWidget *parasite_python_shell_new(void);
void parasite_python_shell_append_text(ParasitePythonShell *python_shell,
                                       const char *str,
                                       const char *tag);
void parasite_python_shell_focus(ParasitePythonShell *python_shell);

G_END_DECLS

#endif // _PARASITE_PYTHON_SHELL_H_
