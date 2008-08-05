import gtk


class WidgetPropertyList(gtk.TreeView):
    def __init__(self):
        gtk.TreeView.__init__(self)


class WidgetTree(gtk.TreeView):
    class Columns:
        WIDGET = 0
        WIDGET_TYPE = 1
        WIDGET_DETAIL = 2
        WIDGET_REALIZED = 3
        WIDGET_VISIBLE = 4
        WIDGET_MAPPED = 5
        WIDGET_WINDOW = 6
        WIDGET_ADDRESS = 7

    def __init__(self):
        model = gtk.TreeStore(object, str, str, bool, bool, bool, str, str)
        gtk.TreeView.__init__(self, model)
        self.set_enable_search(True)
        self.set_search_column(self.Columns.WIDGET_DETAIL)

        sel = self.get_selection()
        sel.connect('changed', self.on_widget_selected, None)

        # Widget column
        column = gtk.TreeViewColumn("Widget")
        self.append_column(column)
        column.set_resizable(True)

        renderer = gtk.CellRendererText()
        column.pack_start(renderer, True)
        column.add_attribute(renderer, "text", self.Columns.WIDGET_TYPE)

        # Detail column
        column = gtk.TreeViewColumn("Detail")
        self.append_column(column)
        column.set_resizable(True)

        renderer = gtk.CellRendererText()
        column.pack_start(renderer, True)
        column.add_attribute(renderer, "text", self.Columns.WIDGET_DETAIL)

        # Realized column
        column = gtk.TreeViewColumn("Realized")
        self.append_column(column)

        renderer = gtk.CellRendererToggle()
        renderer.activatable = True
        column.pack_start(renderer, True)
        column.add_attribute(renderer, "active", self.Columns.WIDGET_REALIZED)

        # Mapped column
        column = gtk.TreeViewColumn("Mapped")
        self.append_column(column)

        renderer = gtk.CellRendererToggle()
        renderer.activatable = True
        column.pack_start(renderer, True)
        column.add_attribute(renderer, "active", self.Columns.WIDGET_MAPPED)

        # Visible column
        column = gtk.TreeViewColumn("Visible")
        self.append_column(column)

        renderer = gtk.CellRendererToggle()
        renderer.activatable = True
        column.pack_start(renderer, True)
        column.add_attribute(renderer, "active", self.Columns.WIDGET_VISIBLE)

    def scan_window(self, window):
        pass

    def select_widget(self, widget):
        pass

    def on_widget_selected(self, selection, user_data):
        pass
