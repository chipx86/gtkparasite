#include "parasite.h"
#include "prop-list.h"
#include "widget-tree.h"

#include <string.h>
#include <gdk/gdkx.h>


enum
{
    WIDGET,
    WIDGET_TYPE,
    WIDGET_NAME,
    WIDGET_REALIZED,
    WIDGET_VISIBLE,
    WIDGET_MAPPED,
    WIDGET_WINDOW,
    WIDGET_ADDRESS,
    ROW_COLOR,
    NUM_COLUMNS
};


enum
{
    WIDGET_CHANGED,
    LAST_SIGNAL
};


struct _ParasiteWidgetTreePrivate
{
    GtkTreeStore *model;
    gboolean edit_mode_enabled;
};

#define PARASITE_WIDGET_TREE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PARASITE_TYPE_WIDGET_TREE, ParasiteWidgetTreePrivate))

static GtkTreeViewClass *parent_class;
static guint widget_tree_signals[LAST_SIGNAL] = { 0 };


static void
parasite_widget_tree_on_widget_selected(GtkTreeSelection *selection,
                                        ParasiteWidgetTree *widget_tree)
{
    g_signal_emit(widget_tree, widget_tree_signals[WIDGET_CHANGED], 0);
}


static void
handle_toggle(GtkCellRendererToggle *toggle,
              char *path_str,
              ParasiteWidgetTree *widget_tree,
              int column,
              void (*enable_func)(GtkWidget*),
              void (*disable_func)(GtkWidget*))
{
    GtkTreeIter iter;
    GtkWidget *widget;
    gboolean new_active = !gtk_cell_renderer_toggle_get_active(toggle);

    if (!widget_tree->priv->edit_mode_enabled)
        return;

    gtk_tree_model_get_iter(GTK_TREE_MODEL(widget_tree->priv->model),
                            &iter,
                            gtk_tree_path_new_from_string(path_str));
    gtk_tree_model_get(GTK_TREE_MODEL(widget_tree->priv->model), &iter,
                       WIDGET, &widget,
                       -1);

    if (new_active)
        enable_func(widget);
    else
        disable_func(widget);

    gtk_tree_store_set(widget_tree->priv->model, &iter,
                       column, new_active,
                       -1);
}


static void
on_toggle_realize(GtkCellRendererToggle *toggle,
                  char *path_str,
                  GtkWidget *treeview)
{
    handle_toggle(toggle,
                  path_str,
                  PARASITE_WIDGET_TREE(treeview),
                  WIDGET_REALIZED,
                  gtk_widget_realize,
                  gtk_widget_unrealize);
}


static void
on_toggle_visible(GtkCellRendererToggle *toggle,
                  char *path_str,
                  GtkWidget *treeview)
{
    handle_toggle(toggle,
                  path_str,
                  PARASITE_WIDGET_TREE(treeview),
                  WIDGET_VISIBLE,
                  gtk_widget_show,
                  gtk_widget_hide);
}


static void
on_toggle_map(GtkCellRendererToggle *toggle,
              char *path_str,
              GtkWidget *treeview)
{
    handle_toggle(toggle,
                  path_str,
                  PARASITE_WIDGET_TREE(treeview),
                  WIDGET_MAPPED,
                  gtk_widget_map,
                  gtk_widget_unmap);
}


static void
parasite_widget_tree_init(ParasiteWidgetTree *widget_tree,
                          ParasiteWidgetTreeClass *klass)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;

    widget_tree->priv = PARASITE_WIDGET_TREE_GET_PRIVATE(widget_tree);

    widget_tree->priv->model = gtk_tree_store_new(
        NUM_COLUMNS,
        G_TYPE_POINTER, // WIDGET
        G_TYPE_STRING,  // WIDGET_NAME
        G_TYPE_STRING,  // WIDGET_NAME
        G_TYPE_BOOLEAN, // WIDGET_REALIZED
        G_TYPE_BOOLEAN, // WIDGET_VISIBLE
        G_TYPE_BOOLEAN, // WIDGET_MAPPED
        G_TYPE_STRING,  // WIDGET_WINDOW
        G_TYPE_STRING,  // WIDGET_ADDRESS
        G_TYPE_STRING); // ROW_COLOR

    widget_tree->priv->edit_mode_enabled = FALSE;

    gtk_tree_view_set_model(GTK_TREE_VIEW(widget_tree),
                            GTK_TREE_MODEL(widget_tree->priv->model));
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(widget_tree), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(widget_tree), WIDGET_NAME);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget_tree));
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(parasite_widget_tree_on_widget_selected),
                     widget_tree);

    // Widget column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "scale", TREE_TEXT_SCALE, NULL);
    column = gtk_tree_view_column_new_with_attributes("Widget", renderer,
                                                      "text", WIDGET_TYPE,
                                                      "foreground", ROW_COLOR,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget_tree), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    // Name column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "scale", TREE_TEXT_SCALE, NULL);
    column = gtk_tree_view_column_new_with_attributes("Name", renderer,
                                                      "text", WIDGET_NAME,
                                                      "foreground", ROW_COLOR,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget_tree), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    // Realized column
    renderer = gtk_cell_renderer_toggle_new();
    g_object_set(G_OBJECT(renderer),
                 "activatable", TRUE,
                 "indicator-size", TREE_CHECKBOX_SIZE,
                 NULL);
    column = gtk_tree_view_column_new_with_attributes("Realized",
                                                      renderer,
                                                      "active", WIDGET_REALIZED,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget_tree), column);
    g_signal_connect(G_OBJECT(renderer), "toggled",
                     G_CALLBACK(on_toggle_realize), widget_tree);

    // Mapped column
    renderer = gtk_cell_renderer_toggle_new();
    g_object_set(G_OBJECT(renderer),
                 "activatable", TRUE,
                 "indicator-size", TREE_CHECKBOX_SIZE,
                 NULL);
    column = gtk_tree_view_column_new_with_attributes("Mapped",
                                                      renderer,
                                                      "active", WIDGET_MAPPED,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget_tree), column);
    //g_signal_connect(G_OBJECT(renderer), "toggled",
    //                 G_CALLBACK(on_toggle_map), widget_tree);

    // Visible column
    renderer = gtk_cell_renderer_toggle_new();
    g_object_set(G_OBJECT(renderer),
                 "activatable", TRUE,
                 "indicator-size", TREE_CHECKBOX_SIZE,
                 NULL);
    column = gtk_tree_view_column_new_with_attributes("Visible",
                                                      renderer,
                                                      "active", WIDGET_VISIBLE,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget_tree), column);
    //g_signal_connect(G_OBJECT(renderer), "toggled",
    //                 G_CALLBACK(on_toggle_visible), widget_tree);

    // X Window column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer),
                 "scale", TREE_TEXT_SCALE,
                 "family", "monospace",
                 NULL);
    column = gtk_tree_view_column_new_with_attributes("X Window",
                                                      renderer,
                                                      "text", WIDGET_WINDOW,
                                                      "foreground", ROW_COLOR,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget_tree), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    // Poinder Address column
    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer),
                 "scale", TREE_TEXT_SCALE,
                 "family", "monospace",
                 NULL);
    column = gtk_tree_view_column_new_with_attributes("Pointer Address",
                                                      renderer,
                                                      "text", WIDGET_ADDRESS,
                                                      "foreground", ROW_COLOR,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget_tree), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
}


static void
parasite_widget_tree_class_init(ParasiteWidgetTreeClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    klass->widget_changed = NULL;

    parent_class = g_type_class_peek_parent(klass);

    widget_tree_signals[WIDGET_CHANGED] =
        g_signal_new("widget-changed",
                     G_OBJECT_CLASS_TYPE(klass),
                     G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
                     G_STRUCT_OFFSET(ParasiteWidgetTreeClass, widget_changed),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__VOID,
                     G_TYPE_NONE, 0);

    g_type_class_add_private(object_class, sizeof(ParasiteWidgetTreePrivate));
}


GType
parasite_widget_tree_get_type()
{
    static GType type = 0;

    if (type == 0)
    {
        static const GTypeInfo info =
        {
            sizeof(ParasiteWidgetTreeClass),
            NULL, // base_init
            NULL, // base_finalize
            (GClassInitFunc) parasite_widget_tree_class_init,
            NULL,
            NULL, // class_data
            sizeof(ParasiteWidgetTree),
            0, // n_preallocs
            (GInstanceInitFunc) parasite_widget_tree_init,
        };

        type = g_type_register_static(GTK_TYPE_TREE_VIEW,
                                      "ParasiteWidgetTree",
                                      &info, 0);
    }

    return type;
}


GtkWidget *
parasite_widget_tree_new()
{
    return GTK_WIDGET(g_object_new(PARASITE_TYPE_WIDGET_TREE, NULL));
}


GtkWidget *
parasite_widget_tree_get_selected_widget(ParasiteWidgetTree *widget_tree)
{
    GtkTreeIter iter;
    GtkTreeSelection *sel;
    GtkTreeModel *model;

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget_tree));

    if (gtk_tree_selection_get_selected(sel, &model, &iter))
    {
        GtkWidget *widget;

        gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
                           WIDGET, &widget,
                           -1);
        return widget;
    }
    return NULL;
}


static void
append_widget(GtkTreeStore *model,
              GtkWidget *widget,
              GtkTreeIter *parent_iter)
{
    GtkTreeIter iter;
    const char *class_name = G_OBJECT_CLASS_NAME(GTK_WIDGET_GET_CLASS(widget));
    const char *name;
    const char *row_color;
    char *window_info;
    char *address;
    gboolean realized;
    gboolean mapped;
    gboolean visible;
    GList *l;

    name = gtk_widget_get_name(widget);
    if (name == NULL || strcmp(name, class_name) == 0) {
        if (GTK_IS_LABEL(widget))
        {
            name = gtk_label_get_text(GTK_LABEL(widget));
        }
        else if (GTK_IS_BUTTON(widget))
        {
            name = gtk_button_get_label(GTK_BUTTON(widget));
        }
        else if (GTK_IS_WINDOW(widget))
        {
            name = gtk_window_get_title(GTK_WINDOW(widget));
        }
        else
        {
            name = "";
        }
    }

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

    realized = GTK_WIDGET_REALIZED(widget);
    mapped = GTK_WIDGET_MAPPED(widget);
    visible = GTK_WIDGET_VISIBLE(widget);

    row_color = (realized && mapped && visible) ? "black" : "grey";

    gtk_tree_store_append(model, &iter, parent_iter);
    gtk_tree_store_set(model, &iter,
                       WIDGET, widget,
                       WIDGET_TYPE, class_name,
                       WIDGET_NAME, name,
                       WIDGET_REALIZED, realized,
                       WIDGET_MAPPED, mapped,
                       WIDGET_VISIBLE, visible,
                       WIDGET_WINDOW, window_info,
                       WIDGET_ADDRESS, address,
                       ROW_COLOR, row_color,
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
parasite_widget_tree_scan(ParasiteWidgetTree *widget_tree,
                          GtkWidget *window)
{
    gtk_tree_store_clear(widget_tree->priv->model);
    append_widget(widget_tree->priv->model, window, NULL);
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(widget_tree));
}


void
parasite_widget_tree_set_edit_mode(ParasiteWidgetTree *widget_tree,
                                   gboolean edit)
{
    widget_tree->priv->edit_mode_enabled = edit;
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
parasite_widget_tree_select_widget(ParasiteWidgetTree *widget_tree,
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


// vim: set et sw=4 ts=4:
