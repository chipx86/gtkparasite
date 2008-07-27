#include "parasite.h"


enum
{
    NAME,
    VALUE,
    NUM_COLUMNS
};


GtkWidget *
gtkparasite_prop_list_new(ParasiteWindow *parasite)
{
    GtkListStore *model;
    GtkWidget *treeview;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    model = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(treeview), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeview), NAME);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Property", renderer,
                                                      "text", NAME,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_sort_order(column, GTK_SORT_ASCENDING);
    gtk_tree_view_column_set_sort_column_id(column, NAME);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Value", renderer,
                                                      "text", VALUE,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
                                         NAME, GTK_SORT_ASCENDING);

    return treeview;
}

void
gtkparasite_prop_list_set_widget(GtkWidget *prop_list,
                                 GtkWidget *widget)
{
    GtkListStore *model =
        GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(prop_list)));
    GtkTreeIter iter;
    GParamSpec **props;
    guint num_properties;
    guint i;

    gtk_list_store_clear(model);

    props = g_object_class_list_properties(G_OBJECT_GET_CLASS(widget),
                                           &num_properties);

    for (i = 0; i < num_properties; i++)
    {
        GParamSpec *prop = props[i];
        GValue gvalue = {0};
        char *value;

        if (!(prop->flags & G_PARAM_READABLE))
            continue;

        g_value_init(&gvalue, prop->value_type);
        g_object_get_property(G_OBJECT(widget), prop->name, &gvalue);
        value = g_strdup_value_contents(&gvalue);

        gtk_list_store_append(model, &iter);
        gtk_list_store_set(model, &iter,
                           NAME, prop->name,
                           VALUE, value,
                           -1);

        g_free(value);
        g_value_unset(&gvalue);
    }
}

// vim: set et ts=4:
