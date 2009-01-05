#include "parasite.h"
#include "prop-list.h"


enum
{
    NAME,
    VALUE,
    NUM_COLUMNS
};


struct _ParasitePropListPrivate
{
    GtkListStore *model;
};

#define PARASITE_PROPLIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PARASITE_TYPE_PROPLIST, ParasitePropListPrivate))

static GtkTreeViewClass *parent_class;


static void
parasite_proplist_init(ParasitePropList *proplist,
                       ParasitePropListClass *klass)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    proplist->priv = PARASITE_PROPLIST_GET_PRIVATE(proplist);

    proplist->priv->model =
        gtk_list_store_new(NUM_COLUMNS,
                           G_TYPE_STRING,  // NAME
                           G_TYPE_STRING); // VALUE
    gtk_tree_view_set_model(GTK_TREE_VIEW(proplist),
                            GTK_TREE_MODEL(proplist->priv->model));

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "scale", TREE_TEXT_SCALE, NULL);
    column = gtk_tree_view_column_new_with_attributes("Property", renderer,
                                                     "text", NAME,
                                                     NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(proplist), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_sort_order(column, GTK_SORT_ASCENDING);
    gtk_tree_view_column_set_sort_column_id(column, NAME);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "scale", TREE_TEXT_SCALE, NULL);
    column = gtk_tree_view_column_new_with_attributes("Value", renderer,
                                                      "text", VALUE,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(proplist), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    gtk_tree_sortable_set_sort_column_id(
        GTK_TREE_SORTABLE(proplist->priv->model),
        NAME, GTK_SORT_ASCENDING);
}


static void
parasite_proplist_class_init(ParasitePropListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    parent_class = g_type_class_peek_parent(klass);

    g_type_class_add_private(object_class, sizeof(ParasitePropListPrivate));
}


GType
parasite_proplist_get_type()
{
    static GType type = 0;

    if (type == 0)
    {
        static const GTypeInfo info =
        {
            sizeof(ParasitePropListClass),
            NULL, // base_init
            NULL, // base_finalize
            (GClassInitFunc) parasite_proplist_class_init,
            NULL,
            NULL, // class_data
            sizeof(ParasitePropList),
            0, // n_preallocs
            (GInstanceInitFunc) parasite_proplist_init,
        };

        type = g_type_register_static(GTK_TYPE_TREE_VIEW,
                                      "ParasitePropList",
                                      &info, 0);
    }

    return type;
}


GtkWidget *
parasite_proplist_new()
{
    return GTK_WIDGET(g_object_new(PARASITE_TYPE_PROPLIST, NULL));
}


void
parasite_proplist_set_widget(ParasitePropList* proplist,
                             GtkWidget *widget)
{
    GtkTreeIter iter;
    GParamSpec **props;
    guint num_properties;
    guint i;

    gtk_list_store_clear(proplist->priv->model);

    props = g_object_class_list_properties(G_OBJECT_GET_CLASS(widget),
                                           &num_properties);

    for (i = 0; i < num_properties; i++)
    {
        GParamSpec *prop = props[i];
        GValue gvalue = {0};
        char *value;

        if (!(prop->flags & G_PARAM_READABLE))
        {
            continue;
        }

        g_value_init(&gvalue, prop->value_type);
        g_object_get_property(G_OBJECT(widget), prop->name, &gvalue);
        value = g_strdup_value_contents(&gvalue);

        gtk_list_store_append(proplist->priv->model, &iter);
        gtk_list_store_set(proplist->priv->model, &iter,
                           NAME, prop->name,
                           VALUE, value,
                           -1);

        g_free(value);
        g_value_unset(&gvalue);
    }
}


// vim: set et sw=4 ts=4:
