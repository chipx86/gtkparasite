#ifndef _GTKPARASITE_WIDGET_TREE_H_
#define _GTKPARASITE_WIDGET_TREE_H_


#include <gtk/gtk.h>


#define PARASITE_TYPE_WIDGET_TREE            (parasite_widget_tree_get_type())
#define PARASITE_WIDGET_TREE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PARASITE_TYPE_WIDGET_TREE, ParasiteWidgetTree))
#define PARASITE_WIDGET_TREE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PARASITE_TYPE_WIDGET_TREE, ParasiteWidgetTreeClass))
#define PARASITE_IS_WIDGET_TREE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PARASITE_TYPE_WIDGET_TREE))
#define PARASITE_IS_WIDGET_TREE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PARASITE_TYPE_WIDGET_TREE))
#define PARASITE_WIDGET_TREE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PARASITE_TYPE_WIDGET_TREE, ParasiteWidgetTreeClass))


typedef struct _ParasiteWidgetTreePrivate ParasiteWidgetTreePrivate;

typedef struct _ParasiteWidgetTree {
   GtkTreeView parent;

   // Private
   ParasiteWidgetTreePrivate *priv;
} ParasiteWidgetTree;

typedef struct _ParasiteWidgetTreeClass {
   GtkTreeViewClass parent;

    void (*widget_changed)(ParasiteWidgetTree *tree);
} ParasiteWidgetTreeClass;


G_BEGIN_DECLS


GType parasite_widget_tree_get_type();
GtkWidget *parasite_widget_tree_new();
GtkWidget *parasite_widget_tree_get_selected_widget(ParasiteWidgetTree *widget_tree);
void parasite_widget_tree_scan(ParasiteWidgetTree *widget_tree,
                               GtkWidget *window);
void parasite_widget_tree_select_widget(ParasiteWidgetTree *widget_tree,
                                        GtkWidget *widget);
void parasite_widget_tree_set_edit_mode(ParasiteWidgetTree *widget_tree,
                                        gboolean edit);


G_END_DECLS


#endif // _GTKPARASITE_WIDGETTREE_H_

// vim: set et sw=4 ts=4:
