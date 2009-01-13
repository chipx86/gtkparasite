moduledir = $(libdir)/gtk-2.0/modules

SOURCES = \
	inspect-button.c \
	module.c \
	parasite.h \
	prop-list.c \
	widget-tree.c \
	window.c

if ENABLE_PYTHON
SOURCES += \
	python-shell.c \
	python-shell.h
endif

libgtkparasite_la_SOURCES = $(SOURCES)

libgtkparasite_la_LDFLAGS = \
	-rpath $(moduledir) -module -avoid-version \
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
