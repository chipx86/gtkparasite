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
#include "widget-tree.h"

#include <string.h>
#if HAVE_X11
#include <gdk/gdkx.h>
#endif


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

struct ColumnState
{
    GtkCellRenderer *renderer;
#if GTK3
    GtkCheckMenuItem *menuitem;
#endif
    guint editable :1;
};

struct _ParasiteWidgetTreePrivate
{
    GtkTreeStore *model;
#if GTK3
    GtkMenu *menu;
#endif

    struct ColumnState columns[NUM_COLUMNS];
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

#if GTK3

static gboolean
on_header_button_press(GtkButton *header,
		       GdkEventButton *event,
		       ParasiteWidgetTree *widget_tree)
{
    if (event->button ==3)
    {
	gtk_menu_popup(GTK_MENU(widget_tree->priv->menu), NULL, NULL,
		       NULL, NULL, event->button, event->time);
	return TRUE;
    }
    return FALSE;
}

static gboolean
on_header_button3_only(GtkButton *header,
		       GdkEventButton *event,
		       ParasiteWidgetTree *widget_tree)
{
    if (event->button ==3)
    {
	gtk_menu_popup(GTK_MENU(widget_tree->priv->menu), NULL, NULL,
		       NULL, NULL, event->button, event->time);
    }
    return TRUE;
}

static void
on_showhide_column(GtkCheckMenuItem *menuitem, GtkTreeViewColumn *column)
{
    gtk_tree_view_column_set_visible(column, gtk_check_menu_item_get_active(menuitem));
}

static void
column_init_ui(ParasiteWidgetTree *widget_tree, GtkTreeViewColumn *column,
	       struct ColumnState *state,
	       const gchar *title, GCallback button_press_cb)
{
    GtkCheckMenuItem *menuitem;
    GtkWidget *header;

    state->menuitem = menuitem =
	    GTK_CHECK_MENU_ITEM(gtk_check_menu_item_new_with_label(title));
    gtk_check_menu_item_set_active(menuitem,
				   gtk_tree_view_column_get_visible(column));
    gtk_menu_shell_append(GTK_MENU_SHELL(widget_tree->priv->menu),
			  GTK_WIDGET(menuitem));
    g_signal_connect(G_OBJECT(menuitem), "toggled",
		     G_CALLBACK(on_showhide_column), column);

    header = gtk_tree_view_column_get_button(column);
    gtk_button_set_focus_on_click(GTK_BUTTON(header), FALSE);

    g_signal_connect(G_OBJECT(header), "button-press-event",
		     button_press_cb, widget_tree);
}

#endif

static void
on_toggle_column_clicked(GtkTreeViewColumn *column, struct ColumnState *state)
{
    GtkCellRendererToggle *renderer = GTK_CELL_RENDERER_TOGGLE(state->renderer);
    state->editable = !state->editable;
    gtk_cell_renderer_toggle_set_activatable(renderer, state->editable);
    gtk_tree_view_column_queue_resize(column);
}

static GtkTreeViewColumn *
_text_column_init(ParasiteWidgetTree *widget_tree,
		  GtkCellRenderer *renderer,
		  const gchar *title, int column_id)
{
    GtkTreeViewColumn *column;
    struct ColumnState *state = &widget_tree->priv->columns[column_id];

    column = gtk_tree_view_column_new_with_attributes(title, renderer,
						      "text", column_id,
						      "foreground", ROW_COLOR,
						      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget_tree), column);
    gtk_tree_view_column_set_resizable(column, TRUE);

#if GTK3
    gtk_tree_view_column_set_clickable(column, TRUE);
    column_init_ui(widget_tree, column, state, title,
		   G_CALLBACK(on_header_button3_only));
#endif

    return column;
}

static GtkTreeViewColumn *
text_column_init(ParasiteWidgetTree *widget_tree,
		 const gchar *title, int column_id)
{
    GtkCellRenderer *renderer;

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer), "scale", TREE_TEXT_SCALE, NULL);
    return _text_column_init(widget_tree, renderer, title, column_id);
}

static GtkTreeViewColumn *
monospace_text_column_init(ParasiteWidgetTree *widget_tree,
			   const gchar *title, int column_id)
{
    GtkCellRenderer *renderer;

    renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer),
		 "scale", TREE_TEXT_SCALE,
		 "family", "monospace", NULL);
    return _text_column_init(widget_tree, renderer, title, column_id);
}

static GtkTreeViewColumn *
toggle_column_init(ParasiteWidgetTree *widget_tree,
		   const gchar *title, int column_id,
		   gboolean editable, GCallback on_cell_toggle)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    struct ColumnState *state = &widget_tree->priv->columns[column_id];

    state->editable = editable;
    state->renderer = renderer = gtk_cell_renderer_toggle_new();
    g_object_set(G_OBJECT(renderer),
                 "activatable", editable,
                 "indicator-size", TREE_CHECKBOX_SIZE,
                 NULL);
    column = gtk_tree_view_column_new_with_attributes(title,
                                                      renderer,
                                                      "active", column_id,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget_tree), column);
    if (on_cell_toggle != NULL) {
	g_signal_connect(G_OBJECT(renderer), "toggled",
			 G_CALLBACK(on_cell_toggle), widget_tree);
    }

    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(G_OBJECT(column), "clicked",
		     G_CALLBACK(on_toggle_column_clicked), state);

#if GTK3
    column_init_ui(widget_tree, column, state, title,
		   G_CALLBACK(on_header_button_press));
#endif

    return column;
}

static void
parasite_widget_tree_init(ParasiteWidgetTree *widget_tree,
                          ParasiteWidgetTreeClass *klass)
{
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

    gtk_tree_view_set_model(GTK_TREE_VIEW(widget_tree),
                            GTK_TREE_MODEL(widget_tree->priv->model));
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(widget_tree), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(widget_tree), WIDGET_NAME);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget_tree));
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(parasite_widget_tree_on_widget_selected),
                     widget_tree);

#if GTK3
    widget_tree->priv->menu = GTK_MENU(gtk_menu_new());
#endif

    // Columns
    text_column_init(widget_tree, "Widget", WIDGET_TYPE);
    text_column_init(widget_tree, "Name", WIDGET_NAME);
    toggle_column_init(widget_tree, "Realized", WIDGET_REALIZED, TRUE,
		       G_CALLBACK(on_toggle_realize));
    toggle_column_init(widget_tree, "Mapped", WIDGET_MAPPED, FALSE, NULL);
    toggle_column_init(widget_tree, "Visible", WIDGET_VISIBLE, FALSE, NULL);
    monospace_text_column_init(widget_tree, "X Window", WIDGET_WINDOW);
    monospace_text_column_init(widget_tree, "Address", WIDGET_ADDRESS);

#if GTK3
    /* Don't allow hiding widget column */
    gtk_widget_set_sensitive(GTK_WIDGET(widget_tree->priv->columns[WIDGET_TYPE].menuitem), FALSE);

    gtk_widget_show_all(GTK_WIDGET(widget_tree->priv->menu));
#endif
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
on_container_forall(GtkWidget *widget, gpointer data)
{
    GList **list = (GList **)data;

    *list = g_list_append(*list, widget);
}

static void
append_widget(GtkTreeStore *model,
              GtkWidget *widget,
              GtkTreeIter *parent_iter)
{
    GtkTreeIter iter;
    const char *class_name = G_OBJECT_CLASS_NAME(GTK_WIDGET_GET_CLASS(widget));
    const char *name;
    char *row_color = NULL;
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

    if (gtk_widget_get_window(widget))
    {
#if HAVE_X11
	window_info = g_strdup_printf("%p (XID 0x%x)", widget->window,
	                              (int)GDK_WINDOW_XID(widget->window));
#else
	window_info = g_strdup("");
#endif
    }
    else
    {
        window_info = g_strdup("");
    }

    address = g_strdup_printf("%p", widget);

    realized = gtk_widget_get_realized(widget);
    mapped = gtk_widget_get_mapped(widget);
    visible = gtk_widget_get_visible(widget);

    if (!realized || !mapped || !visible)
    {
        GtkStyle* style = gtk_widget_get_style(GTK_WIDGET(widget));
        GdkColor color = style->fg[GTK_STATE_INSENSITIVE];

        row_color = gdk_color_to_string(&color);
    }

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
    g_free(row_color);

    if (GTK_IS_CONTAINER(widget))
    {
        GList* children = NULL;

        /* Pick up all children, including those that are internal. */
        gtk_container_forall(GTK_CONTAINER(widget),
                             on_container_forall, &children);

        for (l = children; l != NULL; l = l->next)
        {
            append_widget(model, GTK_WIDGET(l->data), &iter);
        }

        g_list_free(children);
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
