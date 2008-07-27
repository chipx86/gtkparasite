#include "parasite.h"


static GtkWidget *
create_widget_list_pane(ParasiteWindow *parasite)
{
   GtkWidget *swin;
   GtkTreeSelection *sel;

   swin = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
   gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swin),
                                       GTK_SHADOW_IN);

   parasite->widget_tree = gtkparasite_widget_tree_new(parasite);
   gtk_widget_show(parasite->widget_tree);
   gtk_container_add(GTK_CONTAINER(swin), parasite->widget_tree);

   return swin;
}

static GtkWidget *
create_prop_list_pane(ParasiteWindow *parasite)
{
   GtkWidget *swin;

   swin = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
   gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swin),
                                       GTK_SHADOW_IN);
   gtk_widget_set_size_request(swin, 250, -1);

   parasite->prop_list = gtkparasite_prop_list_new(parasite);
   gtk_widget_show(parasite->prop_list);
   gtk_container_add(GTK_CONTAINER(swin), parasite->prop_list);

   return swin;
}

static void
create_top_pane(ParasiteWindow *parasite, GtkWidget *paned)
{
   GtkWidget *vbox;
   GtkWidget *bbox;
   GtkWidget *button;
   GtkWidget *swin;
   GtkWidget *hpaned;

   vbox = gtk_vbox_new(FALSE, 6);
   gtk_widget_show(vbox);
   gtk_paned_pack1(GTK_PANED(paned), vbox, TRUE, TRUE);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);

   bbox = gtk_hbutton_box_new();
   gtk_widget_show(bbox);
   gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);
   gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);

   button = gtkparasite_inspect_button_new(parasite);
   gtk_widget_show(button);
   gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);

   hpaned = gtk_hpaned_new();
   gtk_widget_show(hpaned);
   gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);

   swin = create_widget_list_pane(parasite);
   gtk_widget_show(swin);
   gtk_paned_pack1(GTK_PANED(hpaned), swin, TRUE, TRUE);

   swin = create_prop_list_pane(parasite);
   gtk_widget_show(swin);
   gtk_paned_pack2(GTK_PANED(hpaned), swin, FALSE, TRUE);
}


void
gtkparasite_window_create()
{
   ParasiteWindow *window;
   GtkWidget *main_vbox;
   GtkWidget *vpaned;

   window = g_new0(ParasiteWindow, 1);

   /*
    * Create the top-level window.
    */
   window->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_default_size(GTK_WINDOW(window->window), 1000, 500);
   gtk_window_set_title(GTK_WINDOW(window->window), "Parasite");
   gtk_widget_show(window->window);

   main_vbox = gtk_vbox_new(FALSE, 0);
   gtk_widget_show(main_vbox);
   gtk_container_add(GTK_CONTAINER(window->window), main_vbox);

   vpaned = gtk_vpaned_new();
   gtk_widget_show(vpaned);
   gtk_box_pack_start(GTK_BOX(main_vbox), vpaned, TRUE, TRUE, 0);

   create_top_pane(window, vpaned);
}
