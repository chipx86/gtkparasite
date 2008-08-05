import gtk

from gtkparasite.ui.window import ParasiteWindow


def run(appname):
    """
    Displays the main Parasite window.
    """
    print "Run"
    window = ParasiteWindow(appname)
    window.show()

    return 0
