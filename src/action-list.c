#include "action-list.h"
#include "parasite.h"


enum
{
   ACTION_LABEL,
   ACTION_NAME,
   ACTION_ICON,
   ROW_COLOR,
   SORT_NAME,
   NUM_COLUMNS
};


struct _ParasiteActionListPrivate
{
    GtkTreeStore *model;
};

#define PARASITE_ACTIONLIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PARASITE_TYPE_ACTIONLIST, ParasiteActionListPrivate))

static GtkTreeViewClass *parent_class;


static void
parasite_actionlist_init(ParasiteActionList *actionlist,
                         ParasiteActionListClass *klass)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    actionlist->priv = PARASITE_ACTIONLIST_GET_PRIVATE(actionlist);

    actionlist->priv->model =
        gtk_tree_store_new(NUM_COLUMNS,
                           G_TYPE_STRING,  // ACTION_LABEL
                           G_TYPE_STRING,  // ACTION_NAME
                           G_TYPE_STRING,  // ACTION_ICON
                           G_TYPE_STRING,  // ROW_COLOR,
                           G_TYPE_STRING); // SORT_NAME
    gtk_tree_view_set_model(GTK_TREE_VIEW(actionlist),
                            GTK_TREE_MODEL(actionlist->priv->model));

   column = gtk_tree_view_column_new();
   gtk_tree_view_append_column(GTK_TREE_VIEW(actionlist), column);
   gtk_tree_view_column_set_title(column, "Label");

   renderer = gtk_cell_renderer_pixbuf_new();
   gtk_tree_view_column_pack_start(column, renderer, FALSE);
   gtk_tree_view_column_set_attributes(column, renderer,
                                       "stock-id", ACTION_ICON,
                                       NULL);

   renderer = gtk_cell_renderer_text_new();
   gtk_tree_view_column_pack_start(column, renderer, FALSE);
   gtk_tree_view_column_set_attributes(column, renderer,
                                       "text", ACTION_LABEL,
                                       "foreground", ROW_COLOR,
                                       NULL);

   renderer = gtk_cell_renderer_text_new();
   column = gtk_tree_view_column_new_with_attributes("Action",
                                                     renderer,
                                                     "text", ACTION_NAME,
                                                     "foreground", ROW_COLOR,
                                                     NULL);
   gtk_tree_view_append_column(GTK_TREE_VIEW(actionlist), column);

    gtk_tree_sortable_set_sort_column_id(
        GTK_TREE_SORTABLE(actionlist->priv->model),
        SORT_NAME, GTK_SORT_ASCENDING);
}


static void
parasite_actionlist_class_init(ParasiteActionListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    parent_class = g_type_class_peek_parent(klass);

    g_type_class_add_private(object_class, sizeof(ParasiteActionListPrivate));
}


GType
parasite_actionlist_get_type()
{
    static GType type = 0;

    if (type == 0)
    {
        static const GTypeInfo info =
        {
            sizeof(ParasiteActionListClass),
            NULL, // base_init
            NULL, // base_finalize
            (GClassInitFunc) parasite_actionlist_class_init,
            NULL,
            NULL, // class_data
            sizeof(ParasiteActionList),
            0, // n_preallocs
            (GInstanceInitFunc) parasite_actionlist_init,
        };

        type = g_type_register_static(GTK_TYPE_TREE_VIEW,
                                      "ParasiteActionList",
                                      &info, 0);
    }

    return type;
}


GtkWidget *
parasite_actionlist_new()
{
    return GTK_WIDGET(g_object_new(PARASITE_TYPE_ACTIONLIST, NULL));
}


// vim: set et sw=4 ts=4:
