import gtk

from gtkparasite.ui.trees import WidgetTree, WidgetPropertyList


class ParasiteWindow(gtk.Window):
    def __init__(self, appname):
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
        self.set_default_size(1000, 500)
        self.set_title("Parasite - %s" % appname)

        self.main_vbox = gtk.VBox(False, 0)
        self.main_vbox.show()
        self.add(self.main_vbox)

        self.__create_top_vpaned()

    def __create_top_vpaned(self):
        vpaned = gtk.VPaned()
        vpaned.show()
        self.main_vbox.pack_start(vpaned, True, True, 0)

        vbox = gtk.VBox(False, 6)
        vbox.show()
        vpaned.pack1(vbox, True, True)
        vbox.set_border_width(12)

        bbox = self.__create_toolbar()
        bbox.show()
        vbox.pack_start(bbox, False, False, 0)

        hpaned = gtk.HPaned()
        hpaned.show()
        vbox.pack_start(hpaned, True, True, 0)

        swin = gtk.ScrolledWindow()
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
        swin.set_shadow_type(gtk.SHADOW_IN)
        swin.show()
        hpaned.pack1(swin, True, True)

        self.widget_tree = WidgetTree()
        self.widget_tree.show()
        swin.add(self.widget_tree)

        swin = gtk.ScrolledWindow()
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
        swin.set_shadow_type(gtk.SHADOW_IN)
        swin.show()
        hpaned.pack2(swin, False, True)

        self.prop_list = WidgetPropertyList()
        self.prop_list.show()
        swin.add(self.prop_list)

    def __create_toolbar(self):
        bbox = gtk.HButtonBox()
        bbox.show()
        bbox.set_layout(gtk.BUTTONBOX_START)
        bbox.set_spacing(6)

        button = gtk.Button("_Inspect")
        button.show()
        bbox.pack_start(button, False, False, 0)
        button.connect('button-release-event', self.on_inspect_clicked)

        button = gtk.ToggleButton("_Edit Mode")
        button.show()
        bbox.pack_start(button, False, False, 0)

        return bbox

    def on_inspect_clicked(self, button, event):
        print "click"
