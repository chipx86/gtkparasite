/*
 * Copyright (c) 2008-2009  Christian Hammond
 * Copyright (c) 2008-2009  David Trowbridge
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "parasite.h"
#include "prop-list.h"
#include "property-cell-renderer.h"


enum
{
    COLUMN_NAME,
    COLUMN_VALUE,
    COLUMN_OBJECT,
    NUM_COLUMNS
};


struct _ParasitePropListPrivate
{
    GtkWidget *widget;
    GtkListStore *model;
    GHashTable *prop_iters;
    GList *signal_cnxs;
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
    proplist->priv->prop_iters =
        g_hash_table_new_full(g_str_hash, g_str_equal,
                              NULL, (GDestroyNotify)gtk_tree_iter_free);

    proplist->priv->model =
        gtk_list_store_new(NUM_COLUMNS,
                           G_TYPE_STRING,  // COLUMN_NAME
                           G_TYPE_STRING,  // COLUMN_VALUE
                           G_TYPE_OBJECT); // COLUMN_OBJECT
    gtk_tree_view_set_model(GTK_TREE_VIEW(proplist),
                            GTK_TREE_MODEL(proplist->priv->model));

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "scale", TREE_TEXT_SCALE, NULL);
    column = gtk_tree_view_column_new_with_attributes("Property", renderer,
                                                     "text", COLUMN_NAME,
                                                     NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(proplist), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_sort_order(column, GTK_SORT_ASCENDING);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_NAME);

    renderer = parasite_property_cell_renderer_new();
    g_object_set(G_OBJECT(renderer), "scale", TREE_TEXT_SCALE, NULL);
    g_object_set(G_OBJECT(renderer), "editable", TRUE, NULL);
    column = gtk_tree_view_column_new_with_attributes(
        "Value", renderer,
        "text", COLUMN_VALUE,
        "object", COLUMN_OBJECT,
        "name", COLUMN_NAME,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(proplist), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    gtk_tree_sortable_set_sort_column_id(
        GTK_TREE_SORTABLE(proplist->priv->model),
        COLUMN_NAME, GTK_SORT_ASCENDING);
}


static void
parasite_proplist_class_init(ParasitePropListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    parent_class = g_type_class_peek_parent(klass);

    g_type_class_add_private(object_class, sizeof(ParasitePropListPrivate));
}

static void
parasite_prop_list_update_prop(ParasitePropList *proplist,
                               GtkTreeIter *iter,
                               GParamSpec *prop)
{
    GValue gvalue = {0};
    char *value;

    g_value_init(&gvalue, prop->value_type);
    g_object_get_property(G_OBJECT(proplist->priv->widget),
                          prop->name, &gvalue);

    if (G_VALUE_HOLDS_ENUM(&gvalue))
    {
        GEnumClass *enum_class = G_PARAM_SPEC_ENUM(prop)->enum_class;
        GEnumValue *enum_value = g_enum_get_value(enum_class,
            g_value_get_enum(&gvalue));

        value = g_strdup(enum_value->value_name);
    }
    else
    {
        value = g_strdup_value_contents(&gvalue);
    }

    gtk_list_store_set(proplist->priv->model, iter,
                       COLUMN_NAME, prop->name,
                       COLUMN_VALUE, value,
                       COLUMN_OBJECT, proplist->priv->widget,
                       -1);

    g_free(value);
    g_value_unset(&gvalue);
}

static void
parasite_proplist_prop_changed_cb(GObject *pspec,
                                  GParamSpec *prop,
                                  ParasitePropList *proplist)
{
    GtkTreeIter *iter = g_hash_table_lookup(proplist->priv->prop_iters,
                                            prop->name);

    if (iter != NULL)
        parasite_prop_list_update_prop(proplist, iter, prop);
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
    GList *l;

    proplist->priv->widget = widget;

    for (l = proplist->priv->signal_cnxs; l != NULL; l = l->next)
    {
        gulong id = GPOINTER_TO_UINT(l->data);

        if (g_signal_handler_is_connected(widget, id))
            g_signal_handler_disconnect(widget, id);
    }

    g_list_free(proplist->priv->signal_cnxs);
    proplist->priv->signal_cnxs = NULL;

    g_hash_table_remove_all(proplist->priv->prop_iters);
    gtk_list_store_clear(proplist->priv->model);

    props = g_object_class_list_properties(G_OBJECT_GET_CLASS(widget),
                                           &num_properties);

    for (i = 0; i < num_properties; i++)
    {
        GParamSpec *prop = props[i];
        char *signal_name;

        if (!(prop->flags & G_PARAM_READABLE))
            continue;

        gtk_list_store_append(proplist->priv->model, &iter);
        parasite_prop_list_update_prop(proplist, &iter, prop);

        g_hash_table_insert(proplist->priv->prop_iters, prop->name,
                            gtk_tree_iter_copy(&iter));

        /* Listen for updates */
        signal_name = g_strdup_printf("notify::%s", prop->name);

        proplist->priv->signal_cnxs =
            g_list_prepend(proplist->priv->signal_cnxs, GINT_TO_POINTER(
                g_signal_connect(G_OBJECT(widget), signal_name,
                                 G_CALLBACK(parasite_proplist_prop_changed_cb),
                                 proplist)));

        g_free(signal_name);
    }
}


// vim: set et sw=4 ts=4:
