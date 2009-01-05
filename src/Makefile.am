moduledir = $(libdir)/gtk-2.0/modules


libgtkparasite_la_SOURCES = \
	action-list.c \
	inspect-button.c \
	module.c \
	parasite.h \
	prop-list.c \
	widget-tree.c \
	window.c

libgtkparasite_la_LDFLAGS = \
	-rpath $(moduledir) -module -avoid-version \
	$(GTK_LIBS) \
	@LDFLAGS@


module_LTLIBRARIES = libgtkparasite.la


INCLUDES = \
	$(GTK_CFLAGS)
