#ifndef _GTKPARASITE_ACTIONLIST_H_
#define _GTKPARASITE_ACTIONLIST_H_


#include <gtk/gtk.h>


#define PARASITE_TYPE_ACTIONLIST            (parasite_actionlist_get_type())
#define PARASITE_ACTIONLIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PARASITE_TYPE_ACTIONLIST, ParasiteActionList))
#define PARASITE_ACTIONLIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PARASITE_TYPE_ACTIONLIST, ParasiteActionListClass))
#define PARASITE_IS_ACTIONLIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PARASITE_TYPE_ACTIONLIST))
#define PARASITE_IS_ACTIONLIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PARASITE_TYPE_ACTIONLIST))
#define PARASITE_ACTIONLIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PARASITE_TYPE_ACTIONLIST, ParasiteActionListClass))


typedef struct _ParasiteActionListPrivate ParasiteActionListPrivate;

typedef struct _ParasiteActionList {
    GtkTreeView parent;

    // Private
    ParasiteActionListPrivate *priv;
} ParasiteActionList;

typedef struct _ParasiteActionListClass {
    GtkTreeViewClass parent;
} ParasiteActionListClass;


G_BEGIN_DECLS


GType parasite_actionlist_get_type();
GtkWidget *parasite_actionlist_new();
gpointer parasite_actionlist_get_selected_object(ParasiteActionList *actionlist);


G_END_DECLS


#endif // _GTKPARASITE_ACTIONLIST_H_

// vim: set et sw=4 ts=4:

