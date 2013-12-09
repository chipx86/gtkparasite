moduledir = $(libdir)/gtk-3.0/modules

libgtkparasite_la_SOURCES = \
	inspect-button.c \
	module.c \
	parasite.h \
	prop-list.h \
	prop-list.c \
	property-cell-renderer.c \
	property-cell-renderer.h \
	python-hooks.c \
	python-hooks.h \
	python-shell.c \
	python-shell.h \
	widget-tree.h \
	widget-tree.c \
	window.c \
	button-path.h \
	button-path.c

libgtkparasite_la_LDFLAGS = \
	-rpath $(moduledir) -module -avoid-version -no-undefined \
	$(GTK_LIBS) \
	$(PYGTK_LIBS) \
	$(PYTHON_LIBS) \
	@LDFLAGS@

libgtkparasite_la_LIBADD =


module_LTLIBRARIES = libgtkparasite.la


AM_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(PYTHON_CFLAGS) \
	$(PYGTK_CFLAGS) \
	-DPYTHON_SHARED_LIB="\"$(PYTHON_SHARED_LIB)\""

-include $(top_srcdir)/git.mk
