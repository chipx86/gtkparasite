moduledir = $(libdir)/gtk-$(GTK_API_VERSION)/modules

libgtkparasite_la_SOURCES = \
	action-list.c \
	action-list.h \
	inspect-button.c \
	module.c \
	parasite.h \
	prop-list.h \
	prop-list.c \
	class-tree.h \
	class-tree.c \
	property-cell-renderer.c \
	property-cell-renderer.h \
	python-hooks.c \
	python-hooks.h \
	python-shell.c \
	python-shell.h \
	widget-tree.h \
	widget-tree.c \
	window.c

libgtkparasite_la_LDFLAGS = \
	-rpath $(moduledir) -module -avoid-version -no-undefined \
	$(GTK_LIBS) \
	$(PYGTK_LIBS) \
	$(PYTHON_LIBS) \
	@LDFLAGS@

libgtkparasite_la_LIBADD =


module_LTLIBRARIES = libgtkparasite.la


INCLUDES = \
	$(GTK_CFLAGS) \
	$(PYTHON_CFLAGS) \
	$(PYGTK_CFLAGS) \
	-DPYTHON_SHARED_LIB="\"$(PYTHON_SHARED_LIB)\""

if HAVE_X11
INCLUDES += -DHAVE_X11
endif
