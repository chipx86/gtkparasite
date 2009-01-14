#ifndef _GTKPARASITE_H_
#define _GTKPARASITE_H_


#include <gtk/gtk.h>


#define TREE_TEXT_SCALE 0.8
#define TREE_CHECKBOX_SIZE (gint)(0.8 * 13)


typedef struct
{
    GtkWidget *window;
    GtkWidget *widget_tree;
    GtkWidget *prop_list;
    GtkWidget *python_shell;

    GtkWidget *grab_window;
    GtkWidget *highlight_window;

    GtkWidget *widget_popup;

    GdkWindow *selected_window;

    gboolean edit_mode_enabled;

    int flash_count;
    int flash_cnx;

} ParasiteWindow;


void gtkparasite_window_create();

void gtkparasite_flash_widget(ParasiteWindow *parasite, GtkWidget *widget);

GtkWidget *gtkparasite_inspect_button_new(ParasiteWindow *parasite);


#endif // _GTKPARASITE_H_

// vim: set et sw=4 ts=4:
