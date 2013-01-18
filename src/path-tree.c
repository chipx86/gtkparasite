/* -*- c-file-style: "gnu"; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (c) 2012  Peter Hurley <peter@hurleysoftware.com>
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
#include "path-tree.h"

enum
{
    SELECTOR_NAME,
    NUM_COLUMNS
};

struct _ParasitePathTreePrivate
{
    GtkWidget *widget;
    GtkTreeStore *model;
};

#define PARASITE_PATHTREE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PARASITE_TYPE_PATHTREE, ParasitePathTreePrivate))

G_DEFINE_TYPE(ParasitePathTree, parasite_pathtree, GTK_TYPE_TREE_VIEW);

static void
parasite_pathtree_init(ParasitePathTree *self)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    self->priv = PARASITE_PATHTREE_GET_PRIVATE(self);

    self->priv->model = gtk_tree_store_new(NUM_COLUMNS,
                                           G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(self),
                            GTK_TREE_MODEL(self->priv->model));
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(self), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(self), SELECTOR_NAME);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "scale", TREE_TEXT_SCALE, NULL);
    column = gtk_tree_view_column_new_with_attributes("Selector", renderer,
                                                      "text", SELECTOR_NAME,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
}

static void
parasite_pathtree_class_init(ParasitePathTreeClass *klass)
{
    g_type_class_add_private(klass, sizeof(ParasitePathTreePrivate));
}

GtkWidget *
parasite_pathtree_new()
{
    return GTK_WIDGET(g_object_new(PARASITE_TYPE_PATHTREE, NULL));
}

void
parasite_pathtree_set_widget(ParasitePathTree* self, GtkWidget *widget)
{
    GtkWidgetPath *path = gtk_widget_path_copy(gtk_widget_get_path(widget));
    gint path_len = gtk_widget_path_length(path);
    GtkTreeIter i1, i2;
    GtkTreeIter *iter = NULL;
    GtkTreeIter *child = &i1;
    gint pos;
    const gchar *type, *name;
    GSList *regions, *region, *style_classes, *style_class;

    self->priv->widget = widget;
    gtk_tree_store_clear(self->priv->model);

    for (pos = path_len - 1; pos >= 0; --pos) {
        type = g_type_name(gtk_widget_path_iter_get_object_type(path, pos));
        name = gtk_widget_path_iter_get_name(path, pos);
        if (name)
            type = g_strdup_printf("%s#%s", type, name);

        gtk_tree_store_append(self->priv->model, child, iter);
        gtk_tree_store_set(self->priv->model, child,
                           SELECTOR_NAME, type,
                           -1);
        if (name)
            g_free((gchar *)type);

        iter = child;
        child = (child == &i1) ? &i2 : &i1;

        regions = gtk_widget_path_iter_list_regions(path, pos);
        for (region = regions; region != NULL; region = g_slist_next(region)) {
            gtk_tree_store_append(self->priv->model, child, iter);
            gtk_tree_store_set(self->priv->model, child,
                               SELECTOR_NAME, region->data,
                               -1);
        }
        g_slist_free(regions);

        style_classes = gtk_widget_path_iter_list_classes(path, pos);
        for (style_class = style_classes; style_class != NULL; style_class = g_slist_next(style_class)) {
            gchar *tmp = g_strdup_printf(".%s", style_class->data);

            gtk_tree_store_append(self->priv->model, child, iter);
            gtk_tree_store_set(self->priv->model, child,
                               SELECTOR_NAME, tmp,
                               -1);
            g_free(tmp);
        }
        g_slist_free(style_classes);
    }

    gtk_tree_view_expand_all(GTK_TREE_VIEW(self));
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(self));

    gtk_widget_path_free(path);
}
