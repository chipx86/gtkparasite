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
#include "action-list.h"
#include "parasite.h"


enum
{
   ACTION_LABEL,
   ACTION_NAME,
   ACTION_ICON,
   ROW_COLOR,
   SORT_NAME,
   ADDRESS,
   NUM_COLUMNS
};


struct _ParasiteActionListPrivate
{
    GtkTreeStore *model;
    GSList *uimanagers;
    guint update_timeout;
};

#define PARASITE_ACTIONLIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PARASITE_TYPE_ACTIONLIST, ParasiteActionListPrivate))

static GtkTreeViewClass *parent_class;


gboolean
update(ParasiteActionList *actionlist)
{
    GSList *i;

    gtk_tree_store_clear(actionlist->priv->model);

    for (i = actionlist->priv->uimanagers; i != NULL; i = g_slist_next(i))
    {
        GtkUIManager *uimanager;
        GList *action_groups;
        GList *j;
        gchar *name;

        uimanager = GTK_UI_MANAGER(i->data);

        GtkTreeIter i_iter;
        gtk_tree_store_append(actionlist->priv->model, &i_iter, NULL);

        name = g_strdup_printf("UIManager at %p", uimanager);
        gtk_tree_store_set(actionlist->priv->model, &i_iter,
                           ACTION_LABEL, name,
                           SORT_NAME, name,
                           ADDRESS, uimanager,
                           -1);
        g_free(name);

        action_groups = gtk_ui_manager_get_action_groups(uimanager);
        for (j = action_groups; j != NULL; j = g_list_next(j))
        {
            GtkActionGroup *action_group;
            GtkTreeIter j_iter;
            GList *actions;
            GList *k;

            action_group = GTK_ACTION_GROUP(j->data);

            gtk_tree_store_append(actionlist->priv->model, &j_iter, &i_iter);

            name = (gchar*) gtk_action_group_get_name(action_group);
            gtk_tree_store_set(actionlist->priv->model, &j_iter,
                               ACTION_LABEL, name,
                               SORT_NAME, name,
                               ROW_COLOR, gtk_action_group_get_sensitive(action_group)
                                              ? "black" : "grey",
                               ADDRESS, action_group,
                               -1);

            actions = gtk_action_group_list_actions(action_group);
            for (k = actions; k != NULL; k = g_list_next(k))
            {
                GtkTreeIter k_iter;
                GtkAction *action;
                gchar *action_label;
                gchar *action_name;
                gchar *action_stock;
                gchar *sort_name;

                action = GTK_ACTION(k->data);
                g_object_get(action,
                             "label",    &action_label,
                             "name",     &action_name,
                             "stock-id", &action_stock,
                             NULL);

                sort_name = g_strdup_printf("%s%s", name, action_name);

                gtk_tree_store_append(actionlist->priv->model, &k_iter, &j_iter);
                // FIXME: format the mnemonic
                gtk_tree_store_set(actionlist->priv->model, &k_iter,
                                   ACTION_LABEL, action_label,
                                   ACTION_NAME, action_name,
                                   ACTION_ICON, action_stock,
                                   ROW_COLOR, gtk_action_is_sensitive(action)
                                                  ? "black" : "grey",
                                   SORT_NAME, sort_name,
                                   ADDRESS, action,
                                   -1);

                g_free(sort_name);
                g_free(action_stock);
                g_free(action_name);
                g_free(action_label);
            }
        }
    }

    // FIXME: I'm undecided about this, but I also don't really want to try to
    // preserve the exsting expansion state of the whole tree.
    gtk_tree_view_expand_all(GTK_TREE_VIEW(actionlist));

    actionlist->priv->update_timeout = 0;

    return FALSE;
}


void
uimanager_dispose_cb(gpointer data,
                     GObject *object)
{
    ParasiteActionList *actionlist = PARASITE_ACTIONLIST(data);
    actionlist->priv->uimanagers =
        g_slist_remove(actionlist->priv->uimanagers, object);

    if (actionlist->priv->update_timeout == 0) {
        actionlist->priv->update_timeout =
            g_timeout_add(20, (GSourceFunc) update, actionlist);
    }
}


gboolean
actions_changed_cb(GSignalInvocationHint *hint,
                   guint n_param_values,
                   const GValue *param_values,
                   gpointer data)
{
    ParasiteActionList *actionlist = PARASITE_ACTIONLIST(data);
    GtkUIManager *uimanager;
    GSList *i;

    uimanager = GTK_UI_MANAGER(g_value_get_object(&param_values[0]));

    i = g_slist_find(actionlist->priv->uimanagers, uimanager);
    if (i == NULL) {
        actionlist->priv->uimanagers =
            g_slist_prepend(actionlist->priv->uimanagers, uimanager);
        g_object_weak_ref(G_OBJECT(uimanager), uimanager_dispose_cb, data);
    }

    if (actionlist->priv->update_timeout == 0) {
        actionlist->priv->update_timeout =
            g_timeout_add(20, (GSourceFunc) update, actionlist);
    }

    return TRUE;
}


static void
parasite_actionlist_init(ParasiteActionList *actionlist,
                         ParasiteActionListClass *klass)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GTypeClass *uimanager_type;
    guint uimanager_signal;

    actionlist->priv = PARASITE_ACTIONLIST_GET_PRIVATE(actionlist);
    actionlist->priv->uimanagers = NULL;

    actionlist->priv->model =
        gtk_tree_store_new(NUM_COLUMNS,
                           G_TYPE_STRING,   // ACTION_LABEL
                           G_TYPE_STRING,   // ACTION_NAME
                           G_TYPE_STRING,   // ACTION_ICON
                           G_TYPE_STRING,   // ROW_COLOR,
                           G_TYPE_STRING,   // SORT_NAME
                           G_TYPE_POINTER); // ADDRESS
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

    // Listen to all "actions-changed" signal emissions
    uimanager_type = g_type_class_ref(GTK_TYPE_UI_MANAGER);
    uimanager_signal = g_signal_lookup("actions-changed", GTK_TYPE_UI_MANAGER);
    g_signal_add_emission_hook(uimanager_signal, 0,
                               actions_changed_cb,
                               actionlist,
                               NULL);
    g_type_class_unref(uimanager_type);
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


gpointer
parasite_actionlist_get_selected_object(ParasiteActionList *actionlist)
{
    GtkTreeIter iter;
    GtkTreeSelection *sel;
    GtkTreeModel *model;

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(actionlist));

    if (gtk_tree_selection_get_selected(sel, &model, &iter))
    {
        gpointer pointer;

        gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
                           ADDRESS, &pointer,
                           -1);
        return pointer;
    }
    return NULL;
}


// vim: set et sw=4 ts=4:
