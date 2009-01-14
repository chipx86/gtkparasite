#include "parasite.h"
#include "prop-list.h"
#include "widget-tree.h"


static void
on_widget_tree_selection_changed(ParasiteWidgetTree *widget_tree,
                                 ParasiteWindow *parasite)
{
    GtkWidget *selected = parasite_widget_tree_get_selected_widget(widget_tree);
    parasite_proplist_set_widget(PARASITE_PROPLIST(parasite->prop_list), selected);

    /* Flash the widget. */
    gtkparasite_flash_widget(parasite, selected);
}


static GtkWidget *
create_widget_list_pane(ParasiteWindow *parasite)
{
    GtkWidget *swin;

    swin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swin),
                                        GTK_SHADOW_IN);

    parasite->widget_tree = parasite_widget_tree_new();
    gtk_widget_show(parasite->widget_tree);
    gtk_container_add(GTK_CONTAINER(swin), parasite->widget_tree);

    g_signal_connect(G_OBJECT(parasite->widget_tree),
                     "widget-changed",
                     G_CALLBACK(on_widget_tree_selection_changed),
                     parasite);

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

    parasite->prop_list = parasite_proplist_new();
    gtk_widget_show(parasite->prop_list);
    gtk_container_add(GTK_CONTAINER(swin), parasite->prop_list);

    return swin;
}

static void
on_edit_mode_toggled(GtkWidget *toggle_button,
                     ParasiteWindow *parasite)
{
    gboolean active =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_button));

    parasite->edit_mode_enabled = active;
    parasite_widget_tree_set_edit_mode(PARASITE_WIDGET_TREE(parasite->widget_tree),
                                       active);
}

static void
on_show_graphic_updates_toggled(GtkWidget *toggle_button,
                                ParasiteWindow *parasite)
{
    gdk_window_set_debug_updates(
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_button)));
}

static GtkWidget *
create_widget_tree(ParasiteWindow *parasite)
{
    GtkWidget *vbox;
    GtkWidget *bbox;
    GtkWidget *button;
    GtkWidget *swin;
    GtkWidget *hpaned;

    vbox = gtk_vbox_new(FALSE, 6);
    gtk_widget_show(vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);

    bbox = gtk_hbutton_box_new();
    gtk_widget_show(bbox);
    gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);
    gtk_box_set_spacing(GTK_BOX(bbox), 6);

    button = gtkparasite_inspect_button_new(parasite);
    gtk_widget_show(button);
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);

    button = gtk_toggle_button_new_with_mnemonic("_Edit Mode");
    gtk_widget_show(button);
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(button), "toggled",
                     G_CALLBACK(on_edit_mode_toggled), parasite);

    button = gtk_toggle_button_new_with_mnemonic("_Show Graphic Updates");
    gtk_widget_show(button);
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(button), "toggled",
                     G_CALLBACK(on_show_graphic_updates_toggled), parasite);

    hpaned = gtk_hpaned_new();
    gtk_widget_show(hpaned);
    gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);

    swin = create_widget_list_pane(parasite);
    gtk_widget_show(swin);
    gtk_paned_pack1(GTK_PANED(hpaned), swin, TRUE, TRUE);

    swin = create_prop_list_pane(parasite);
    gtk_widget_show(swin);
    gtk_paned_pack2(GTK_PANED(hpaned), swin, FALSE, TRUE);

    return vbox;
}

static GtkWidget *
create_action_list(ParasiteWindow *parasite)
{
   GtkWidget *vbox;
   GtkWidget *swin;

   vbox = gtk_vbox_new(FALSE, 6);
   gtk_widget_show(vbox);
   gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);

   swin = gtk_scrolled_window_new(NULL, NULL);
   gtk_widget_show(swin);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_ALWAYS);
   gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swin),
                                       GTK_SHADOW_IN);
   gtk_box_pack_start(GTK_BOX(vbox), swin, TRUE, TRUE, 0);

   parasite->action_list = parasite_actionlist_new(parasite);
   gtk_widget_show(parasite->action_list);
   gtk_container_add(GTK_CONTAINER(swin), parasite->action_list);

   return vbox;
}

void
gtkparasite_window_create()
{
    ParasiteWindow *window;
    GtkWidget *notebook;
    char *title;

    window = g_new0(ParasiteWindow, 1);

    /*
     * Create the top-level window.
     */
    window->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window->window), 1000, 500);
    gtk_widget_show(window->window);

    title = g_strdup_printf("Parasite - %s", g_get_application_name());
    gtk_window_set_title(GTK_WINDOW(window->window), title);
    g_free(title);

    notebook = gtk_notebook_new();
    gtk_widget_show(notebook);
    gtk_container_add(GTK_CONTAINER(window->window), notebook);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             create_widget_tree(window),
                             gtk_label_new("Widget Tree"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             create_action_list(window),
                             gtk_label_new("Action List"));
}

// vim: set et sw=4 ts=4:
