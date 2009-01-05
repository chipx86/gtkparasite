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


GtkWidget *
gtkparasite_action_list_new(ParasiteWindow *parasite)
{
   GtkTreeStore *model;
   GtkWidget *treeview;
   GtkCellRenderer *renderer;
   GtkTreeViewColumn *column;

   model = gtk_tree_store_new(NUM_COLUMNS,
                              G_TYPE_STRING,  // ACTION_LABEL
                              G_TYPE_STRING,  // ACTION_NAME
                              G_TYPE_STRING,  // ACTION_ICON
                              G_TYPE_STRING,  // ROW_COLOR
                              G_TYPE_STRING); // SORT_NAME

   treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
   gtk_tree_view_set_enable_search(GTK_TREE_VIEW(treeview), TRUE);
   gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeview), ACTION_NAME);
   g_object_set_data(G_OBJECT(treeview), "parasite", parasite);

   column = gtk_tree_view_column_new();
   gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
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
   gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

   return treeview;
}
