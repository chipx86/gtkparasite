#include "parasite.h"

#include <gdk/gdkx.h>


enum
{
    WIDGET,
    WIDGET_TYPE,
    WIDGET_DETAIL,
    WIDGET_REALIZED,
    WIDGET_VISIBLE,
    WIDGET_MAPPED,
    WIDGET_WINDOW,
    WIDGET_ADDRESS,
    NUM_COLUMNS
};


static void
on_widget_selected(GtkTreeSelection *selection,
                   ParasiteWindow *parasite)
{
    GtkTreeIter iter;
    GtkTreeSelection *sel;
    GtkTreeModel *model;

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(parasite->widget_tree));

    if (gtk_tree_selection_get_selected(sel, &model, &iter))
    {
        GtkWidget *widget;

        gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
                           WIDGET, &widget,
                           -1);

        gtkparasite_prop_list_set_widget(parasite->prop_list, widget);

        /* Flash the widget. */
        gtkparasite_flash_widget(parasite, widget);
    }
}

GtkWidget *
gtkparasite_widget_tree_new(ParasiteWindow *parasite)
{
    GtkTreeStore *model;
    GtkWidget *treeview;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeSelection *sel;

    model = gtk_tree_store_new(NUM_COLUMNS,
                               G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING,
                               G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
                               G_TYPE_STRING, G_TYPE_STRING);

    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(treeview), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeview), WIDGET_DETAIL);

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    g_signal_connect(G_OBJECT(sel), "changed",
                     G_CALLBACK(on_widget_selected), parasite);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Widget", renderer,
                                                      "text", WIDGET_TYPE,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Detail", renderer,
                                                      "text", WIDGET_DETAIL,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("Realized",
                                                      renderer,
                                                      "active", WIDGET_REALIZED,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);


    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("Mapped",
                                                      renderer,
                                                      "active", WIDGET_MAPPED,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("Visible",
                                                      renderer,
                                                      "active", WIDGET_VISIBLE,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("X Window",
                                                      renderer,
                                                      "text", WIDGET_WINDOW,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Pointer address",
                                                      renderer,
                                                      "text", WIDGET_ADDRESS,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    return treeview;
}

static void
append_widget(GtkTreeStore *model,
              GtkWidget *widget,
              GtkTreeIter *parent_iter)
{
    GtkTreeIter iter;
    const char *class_name = G_OBJECT_CLASS_NAME(GTK_WIDGET_GET_CLASS(widget));
    const char *detail = "";
    char *window_info;
    char *address;
    GList *l;

    if (GTK_IS_LABEL(widget))
        detail = gtk_label_get_text(GTK_LABEL(widget));
    else if (GTK_IS_BUTTON(widget))
        detail = gtk_button_get_label(GTK_BUTTON(widget));
    else if (GTK_IS_WINDOW(widget))
        detail = gtk_window_get_title(GTK_WINDOW(widget));

    if (widget->window)
    {
        window_info = g_strdup_printf("%p (XID 0x%x)", widget->window,
                                      (int)GDK_WINDOW_XID(widget->window));
    }
    else
    {
        window_info = g_strdup("");
    }

    address = g_strdup_printf("%p", widget);

    gtk_tree_store_append(model, &iter, parent_iter);
    gtk_tree_store_set(model, &iter,
                       WIDGET, widget,
                       WIDGET_TYPE, class_name,
                       WIDGET_DETAIL, detail,
                       WIDGET_REALIZED, GTK_WIDGET_REALIZED(widget),
                       WIDGET_MAPPED, GTK_WIDGET_MAPPED(widget),
                       WIDGET_VISIBLE, GTK_WIDGET_VISIBLE(widget),
                       WIDGET_WINDOW, window_info,
                       WIDGET_ADDRESS, address,
                       -1);

    g_free(window_info);
    g_free(address);

    if (GTK_IS_CONTAINER(widget))
    {
        for (l = gtk_container_get_children(GTK_CONTAINER(widget));
             l != NULL;
             l = l->next)
        {
            append_widget(model, GTK_WIDGET(l->data), &iter);
        }
    }
}

void
gtkparasite_widget_tree_scan(GtkWidget *widget_tree,
                             GtkWidget *window)
{
    GtkTreeStore *model =
        GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget_tree)));

    gtk_tree_store_clear(model);
    append_widget(model, window, NULL);

    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(widget_tree));
}

static GList *
get_parents(GtkWidget *widget,
            GList *parents)
{
    GtkWidget *parent = gtk_widget_get_parent(widget);

    parents = g_list_prepend(parents, widget);

    if (parent != NULL)
        return get_parents(parent, parents);

    return parents;
}

void
gtkparasite_widget_tree_select_widget(GtkWidget *widget_tree,
                                      GtkWidget *widget)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget_tree));
    GList *parents = get_parents(widget, NULL);
    GList *l;
    GtkTreeIter iter, parent_iter = {0};
    gboolean found = FALSE;
    gboolean in_root = TRUE;

    for (l = parents; l != NULL; l = l->next)
    {
        GtkWidget *cur_widget = GTK_WIDGET(l->data);
        gboolean valid;
        found = FALSE;

        for (valid = gtk_tree_model_iter_children(model, &iter,
                                                  in_root ? NULL
                                                  : &parent_iter);
              valid;
              valid = gtk_tree_model_iter_next(model, &iter))
        {
            GtkWidget *iter_widget;

            gtk_tree_model_get(model, &iter,
                               WIDGET, &iter_widget,
                               -1);

            if (iter_widget == cur_widget)
            {
                parent_iter = iter;
                in_root = FALSE;
                found = TRUE;
                break;
            }
        }

        if (!found)
        {
            /* No good. Bail.. */
            break;
        }
    }

    if (found)
    {
        GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
        gtk_tree_view_expand_to_path(GTK_TREE_VIEW(widget_tree), path);
        gtk_tree_selection_select_iter(
            gtk_tree_view_get_selection(GTK_TREE_VIEW(widget_tree)),
            &iter);
        gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(widget_tree), path, NULL,
                                     FALSE, 0, 0);
    }

    g_list_free(parents);
}

// vim: set et ts=4:
