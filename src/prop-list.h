#ifndef _GTKPARASITE_PROPLIST_H_
#define _GTKPARASITE_PROPLIST_H_


#include <gtk/gtk.h>


#define PARASITE_TYPE_PROPLIST            (parasite_proplist_get_type())
#define PARASITE_PROPLIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PARASITE_TYPE_PROPLIST, ParasitePropList))
#define PARASITE_PROPLIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PARASITE_TYPE_PROPLIST, ParasitePropListClass))
#define PARASITE_IS_PROPLIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PARASITE_TYPE_PROPLIST))
#define PARASITE_IS_PROPLIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PARASITE_TYPE_PROPLIST))
#define PARASITE_PROPLIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PARASITE_TYPE_PROPLIST, ParasitePropListClass))


typedef struct _ParasitePropListPrivate ParasitePropListPrivate;

typedef struct _ParasitePropList {
   GtkTreeView parent;

   // Private
   ParasitePropListPrivate *priv;
} ParasitePropList;

typedef struct _ParasitePropListClass {
   GtkTreeViewClass parent;

   // Padding for future expansion
   void (*reserved0)(void);
   void (*reserved1)(void);
   void (*reserved2)(void);
   void (*reserved3)(void);
} ParasitePropListClass;


G_BEGIN_DECLS


GType parasite_proplist_get_type();
GtkWidget *parasite_proplist_new();
void parasite_proplist_set_widget(ParasitePropList* proplist,
                                  GtkWidget *widget);


G_END_DECLS


#endif // _GTKPARASITE_PROPLIST_H_

// vim: set et sw=4 ts=4:
