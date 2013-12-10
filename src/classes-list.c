/*
 * Copyright (c) 2013 Intel Corporation
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

#include "classes-list.h"
#include "parasite.h"

enum
{
  COLUMN_ENABLED,
  COLUMN_NAME,
  NUM_COLUMNS
};

typedef struct
{
  gboolean enabled;
} ParasiteClassesListByContext;

struct _ParasiteClassesListPrivate
{
  GtkListStore *model;
  GHashTable *contexts;
  GtkStyleContext *current_context;
};

G_DEFINE_TYPE_WITH_PRIVATE (ParasiteClassesList, parasite_classeslist, GTK_TYPE_TREE_VIEW)

static void
enabled_toggled (GtkCellRendererToggle *renderer, gchar *path, ParasiteClassesList *cl)
{
  GtkTreeIter iter;
  gboolean enabled;
  GHashTable *context;
  ParasiteClassesListByContext *c;
  gchar *name;

  if (!gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (cl->priv->model), &iter, path))
    {
      g_warning ("Parasite: Couldn't find the css class path for %s.", path);
      return;
    }

  gtk_tree_model_get (GTK_TREE_MODEL (cl->priv->model), &iter,
                      COLUMN_ENABLED, &enabled,
                      COLUMN_NAME, &name,
                      -1);
  enabled = !enabled;
  gtk_list_store_set (cl->priv->model, &iter,
                      COLUMN_ENABLED, enabled,
                      -1);

  context = g_hash_table_lookup (cl->priv->contexts, cl->priv->current_context);
  if (context)
    {
      c = g_hash_table_lookup (context, name);
      if (c)
        {
          c->enabled = enabled;
          if (enabled)
            {
              gtk_style_context_add_class (cl->priv->current_context, name);
            }
          else
            {
              gtk_style_context_remove_class (cl->priv->current_context, name);
            }
        }
      else
        {
          g_warning ("Parasite: Couldn't find the css class %s in the class hash table.", name);
        }
    }
  else
    {
      g_warning ("Parasite: Couldn't find the hash table for the style context for css class %s.", name);
    }
}

static void
parasite_classeslist_init (ParasiteClassesList *cl)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  cl->priv = parasite_classeslist_get_instance_private (cl);

  cl->priv->contexts = g_hash_table_new (g_direct_hash, g_direct_equal);

  cl->priv->model = gtk_list_store_new (NUM_COLUMNS,
                                        G_TYPE_BOOLEAN,  // COLUMN_ENABLED
                                        G_TYPE_STRING);   // COLUMN_NAME
  gtk_tree_view_set_model (GTK_TREE_VIEW (cl),
                           GTK_TREE_MODEL (cl->priv->model));

  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (renderer, "toggled", G_CALLBACK (enabled_toggled), cl);
  column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                     "active", COLUMN_ENABLED,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (cl), column);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer, "scale", TREE_TEXT_SCALE, NULL);
  column = gtk_tree_view_column_new_with_attributes ("Name", renderer,
                                                     "text", COLUMN_NAME,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (cl), column);
}

static void
parasite_classeslist_class_init (ParasiteClassesListClass *klass)
{
}

GtkWidget *
parasite_classeslist_new ()
{
    return GTK_WIDGET (g_object_new (PARASITE_TYPE_CLASSESLIST, NULL));
}

void
parasite_classeslist_set_widget (ParasiteClassesList *cl,
                                 GtkWidget *widget)
{
  GtkStyleContext *widget_context;
  GHashTable *hash_context;
  GtkTreeIter tree_iter;
  ParasiteClassesListByContext *c;

  gtk_list_store_clear (cl->priv->model);
  widget_context = gtk_widget_get_style_context (widget);
  cl->priv->current_context = widget_context;

  hash_context = g_hash_table_lookup (cl->priv->contexts, widget_context);
  if (hash_context)
    {
      GHashTableIter hash_iter;
      gchar *name;

      g_hash_table_iter_init (&hash_iter, hash_context);
      while (g_hash_table_iter_next (&hash_iter, (gpointer *)&name, (gpointer *)&c))
        {
          gtk_list_store_append (cl->priv->model, &tree_iter);
          gtk_list_store_set (cl->priv->model, &tree_iter,
                             COLUMN_ENABLED, c->enabled,
                             COLUMN_NAME, name,
                            -1);
        }
    }
  else
    {
      GList *l, *classes;

      hash_context = g_hash_table_new (g_str_hash, g_str_equal);
      classes = gtk_style_context_list_classes (widget_context);

      for (l = classes; l; l = l->next)
        {
          c = g_new0 (ParasiteClassesListByContext, 1);
          c->enabled = TRUE;
          g_hash_table_insert (hash_context, l->data, c);

          gtk_list_store_append (cl->priv->model, &tree_iter);
          gtk_list_store_set (cl->priv->model, &tree_iter,
                             COLUMN_ENABLED, TRUE,
                             COLUMN_NAME, l->data,
                            -1);
        }
      g_list_free (classes);
      g_hash_table_insert (cl->priv->contexts, widget_context, hash_context);
    }
}

// vim: set et sw=4 ts=4:
