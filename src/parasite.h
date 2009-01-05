#ifndef _GTKPARASITE_H_
#define _GTKPARASITE_H_


#include <gtk/gtk.h>


typedef struct
{
    GtkWidget *window;
    GtkWidget *widget_tree;
    GtkWidget *prop_list;
    GtkWidget *action_list;

    GtkWidget *grab_window;
    GtkWidget *highlight_window;

    GdkWindow *selected_window;

    gboolean edit_mode_enabled;

    int flash_count;
    int flash_cnx;

} ParasiteWindow;


void gtkparasite_window_create();

void gtkparasite_flash_widget(ParasiteWindow *parasite, GtkWidget *widget);

GtkWidget *gtkparasite_inspect_button_new(ParasiteWindow *parasite);
GtkWidget *gtkparasite_action_list_new(ParasiteWindow *parasite);

GtkWidget *gtkparasite_widget_tree_new(ParasiteWindow *parasite);
void gtkparasite_widget_tree_scan(GtkWidget *widget_tree, GtkWidget *window);
void gtkparasite_widget_tree_select_widget(GtkWidget *widget_tree,
                                           GtkWidget *window);

GtkWidget *gtkparasite_prop_list_new(ParasiteWindow *parasite);
void gtkparasite_prop_list_set_widget(GtkWidget *prop_list,
                                      GtkWidget *widget);


#endif // _GTKPARASITE_H_

// vim: set et ts=4:
