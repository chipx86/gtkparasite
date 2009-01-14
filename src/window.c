#include "action-list.h"
#include "parasite.h"
#include "prop-list.h"
#include "widget-tree.h"
#include "python-shell.h"

#include "config.h"


static void
on_widget_tree_selection_changed(ParasiteWidgetTree *widget_tree,
                                 ParasiteWindow *parasite)
{
    GtkWidget *selected = parasite_widget_tree_get_selected_widget(widget_tree);
    if (selected != NULL) {
        parasite_proplist_set_widget(PARASITE_PROPLIST(parasite->prop_list),
                                     selected);

        /* Flash the widget. */
        gtkparasite_flash_widget(parasite, selected);
    }
}


#ifdef ENABLE_PYTHON
static gboolean
on_widget_tree_button_press(ParasiteWidgetTree *widget_tree,
                            GdkEventButton *event,
                            ParasiteWindow *parasite)
{
    if (event->button == 3)
    {
        gtk_menu_popup(GTK_MENU(parasite->widget_popup), NULL, NULL,
                       NULL, NULL, event->button, event->time);
    }

    return FALSE;
}


static gboolean
on_action_list_button_press(ParasiteActionList *actionlist,
                            GdkEventButton *event,
                            ParasiteWindow *parasite)
{
    if (event->button == 3)
    {
        gtk_menu_popup(GTK_MENU(parasite->action_popup), NULL, NULL,
                       NULL, NULL, event->button, event->time);
    }

    return FALSE;
}


static void
on_send_widget_to_shell_activate(GtkWidget *menuitem,
                                 ParasiteWindow *parasite)
{
    GtkWidget *widget = parasite_widget_tree_get_selected_widget(
        PARASITE_WIDGET_TREE(parasite->widget_tree));
    if (widget != NULL) {
        char *str = g_strdup_printf("parasite.gobj(%p)", widget);

        parasite_python_shell_append_text(
            PARASITE_PYTHON_SHELL(parasite->python_shell),
            str, NULL);

        g_free(str);
        parasite_python_shell_focus(PARASITE_PYTHON_SHELL(parasite->python_shell));
    }
}


static void
on_send_action_to_shell_activate(GtkWidget *menuitem,
                                 ParasiteWindow *parasite)
{
    gpointer selection = parasite_actionlist_get_selected_object(
        PARASITE_ACTIONLIST(parasite->action_list));
    if (selection != NULL) {
        char *str = g_strdup_printf("parasite.gobj(%p)", selection);

        parasite_python_shell_append_text(
            PARASITE_PYTHON_SHELL(parasite->python_shell),
            str, NULL);

        g_free(str);

        parasite_python_shell_focus(PARASITE_PYTHON_SHELL(parasite->python_shell));
    }
}
#endif // ENABLE_PYTHON


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

#ifdef ENABLE_PYTHON
    g_signal_connect(G_OBJECT(parasite->widget_tree),
                     "button-press-event",
                     G_CALLBACK(on_widget_tree_button_press),
                     parasite);
#endif

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

#ifdef ENABLE_PYTHON
    g_signal_connect(G_OBJECT(parasite->action_list),
                     "button-press-event",
                     G_CALLBACK(on_action_list_button_press),
                     parasite);
#endif

   return vbox;
}

void
gtkparasite_window_create()
{
    ParasiteWindow *window;
    GtkWidget *vpaned;
    GtkWidget *notebook;
#ifdef ENABLE_PYTHON
    GtkWidget *menuitem;
#endif
    char *title;

    window = g_new0(ParasiteWindow, 1);

    /*
     * Create the top-level window.
     */
    window->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window->window), 1000, 500);
    gtk_container_set_border_width(GTK_CONTAINER(window->window), 12);
    gtk_widget_show(window->window);

    title = g_strdup_printf("Parasite - %s", g_get_application_name());
    gtk_window_set_title(GTK_WINDOW(window->window), title);
    g_free(title);

    vpaned = gtk_vpaned_new();
    gtk_widget_show(vpaned);
    gtk_container_add(GTK_CONTAINER(window->window), vpaned);

    notebook = gtk_notebook_new();
    gtk_widget_show(notebook);
    gtk_paned_pack1(GTK_PANED(vpaned), notebook, TRUE, FALSE);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             create_widget_tree(window),
                             gtk_label_new("Widget Tree"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                             create_action_list(window),
                             gtk_label_new("Action List"));

#ifdef ENABLE_PYTHON
    window->python_shell = parasite_python_shell_new();
    gtk_widget_show(window->python_shell);
    gtk_paned_pack2(GTK_PANED(vpaned), window->python_shell, FALSE, FALSE);

    /*
     * XXX Eventually we'll want to put more in here besides the menu
     *     item we define below. At that point, we'll need to make this
     *     more generic.
     */
    window->widget_popup = gtk_menu_new();
    gtk_widget_show(window->widget_popup);

    menuitem = gtk_menu_item_new_with_label("Send Widget to Shell");
    gtk_widget_show(menuitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->widget_popup), menuitem);

    g_signal_connect(G_OBJECT(menuitem), "activate",
                     G_CALLBACK(on_send_widget_to_shell_activate), window);

    window->action_popup = gtk_menu_new();
    gtk_widget_show(window->action_popup);

    menuitem = gtk_menu_item_new_with_label("Send Object to Shell");
    gtk_widget_show(menuitem);
    gtk_menu_shell_append(GTK_MENU_SHELL(window->action_popup), menuitem);

    g_signal_connect(G_OBJECT(menuitem), "activate",
                     G_CALLBACK(on_send_action_to_shell_activate), window);
#endif // ENABLE_PYTHON
}

// vim: set et sw=4 ts=4:
