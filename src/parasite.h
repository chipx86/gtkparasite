/*
 * Copyright (c) 2008-2009  Christian Hammond
 * Copyright (c) 2008-2009  David Trowbridge
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _GTKPARASITE_H_
#define _GTKPARASITE_H_


#include <gtk/gtk.h>


#define TREE_TEXT_SCALE 0.8
#define TREE_CHECKBOX_SIZE (gint)(0.8 * 13)


typedef struct
{
    GtkWidget *window;
    GtkWidget *widget_tree;
    GtkWidget *prop_list;
    GtkWidget *class_tree;
    GtkWidget *action_list;
    GtkWidget *python_shell;

    GtkWidget *grab_window;
    GtkWidget *highlight_window;

    GtkWidget *widget_popup;
    GtkWidget *action_popup;

    GdkWindow *selected_window;

    gboolean edit_mode_enabled;

    int flash_count;
    int flash_cnx;

} ParasiteWindow;


void gtkparasite_window_create();

void gtkparasite_flash_widget(ParasiteWindow *parasite, GtkWidget *widget);

void gtkparasite_inspect_button_connect(ParasiteWindow *parasite, GtkWidget *button);


#endif // _GTKPARASITE_H_

// vim: set et sw=4 ts=4:
