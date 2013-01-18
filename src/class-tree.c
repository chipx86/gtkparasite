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
#include "class-tree.h"

enum
{
    CLASS,
    CLASS_NAME,
    CLASS_ADDRESS,
    NUM_COLUMNS
};

struct _ParasiteClassTreePrivate
{
    GtkWidget *widget;
    GtkTreeStore *model;
};

#define PARASITE_CLASSTREE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PARASITE_TYPE_CLASSTREE, ParasiteClassTreePrivate))

G_DEFINE_TYPE(ParasiteClassTree, parasite_classtree, GTK_TYPE_TREE_VIEW);

static void
parasite_classtree_init(ParasiteClassTree *self)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    self->priv = PARASITE_CLASSTREE_GET_PRIVATE(self);

    self->priv->model = gtk_tree_store_new(NUM_COLUMNS,
                                           G_TYPE_POINTER,
                                           G_TYPE_STRING,
                                           G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(self),
                            GTK_TREE_MODEL(self->priv->model));
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(self), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(self), CLASS_NAME);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "scale", TREE_TEXT_SCALE, NULL);
    column = gtk_tree_view_column_new_with_attributes("Class", renderer,
                                                      "text", CLASS_NAME,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "scale", TREE_TEXT_SCALE,
                 "family", "monospace", NULL);
    column = gtk_tree_view_column_new_with_attributes("Address", renderer,
                                                      "text", CLASS_ADDRESS,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), column);
    gtk_tree_view_column_set_resizable(column, TRUE);
}

static void
parasite_classtree_class_init(ParasiteClassTreeClass *klass)
{
    g_type_class_add_private(klass, sizeof(ParasiteClassTreePrivate));
}

GtkWidget *
parasite_classtree_new()
{
    return GTK_WIDGET(g_object_new(PARASITE_TYPE_CLASSTREE, NULL));
}

void
parasite_classtree_set_widget(ParasiteClassTree* self, GtkWidget *widget)
{
    GObjectClass *klass = (GObjectClass *)GTK_WIDGET_GET_CLASS(widget);
    const gchar *class_name;
    gchar *address;
    GtkTreeIter i1, i2;
    GtkTreeIter *iter = NULL;
    GtkTreeIter *child = &i1;

    self->priv->widget = widget;
    gtk_tree_store_clear(self->priv->model);

    do
    {
        class_name = G_OBJECT_CLASS_NAME(klass);
        address = g_strdup_printf("%p", klass);

        gtk_tree_store_append(self->priv->model, child, iter);
        gtk_tree_store_set(self->priv->model, child,
                           CLASS, klass,
                           CLASS_NAME, class_name,
                           CLASS_ADDRESS, address,
                           -1);
        g_free(address);

        if (g_strcmp0(class_name, "GtkWidget") == 0)
            break;
        klass = g_type_class_peek_parent(klass);
        iter = child;
        child = (child == &i1) ? &i2 : &i1;
    } while (1);

    gtk_tree_view_expand_all(GTK_TREE_VIEW(self));
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(self));
}
