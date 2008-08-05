SUBDIRS = gtkparasite


moduledir = $(libdir)/gtk-2.0/modules


libgtkparasite_la_SOURCES = \
	module.c

libgtkparasite_la_LDFLAGS = \
	-rpath $(moduledir) -module -avoid-version \
	$(GTK_LIBS) \
	$(PYTHON_LIBS) \
	@LDFLAGS@


module_LTLIBRARIES = libgtkparasite.la


INCLUDES = \
	$(GTK_CFLAGS) \
	$(PYTHON_INCLUDES) \
	-DPY_LIB_LOC="\"$(PYTHON_LIB_LOC)\"" \
	-DPYTHON_VERSION="\"$(PYTHON_VERSION)\""
