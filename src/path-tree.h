/* -*- c-file-style: "gnu"; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (c) 2013  Peter Hurley <peter@hurleysoftware.com>
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
#ifndef _GTKPARASITE_PATHTREE_H_
#define _GTKPARASITE_PATHTREE_H_

#include <gtk/gtk.h>

#define PARASITE_TYPE_PATHTREE		  (parasite_pathtree_get_type())
#define PARASITE_PATHTREE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PARASITE_TYPE_PATHTREE, ParasitePathTree))
#define PARASITE_PATHTREE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  PARASITE_TYPE_PATHTREE, ParasitePathTreeClass))
#define PARASITE_IS_PATHTREE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PARASITE_TYPE_PATHTREE))
#define PARASITE_IS_PATHTREE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  PARASITE_TYPE_PATHTREE))
#define PARASITE_PATHTREE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  PARASITE_TYPE_PATHTREE, ParasitePathTreeClass))


typedef struct _ParasitePathTreePrivate ParasitePathTreePrivate;

typedef struct _ParasitePathTree {
    GtkTreeView parent;

    ParasitePathTreePrivate *priv;
} ParasitePathTree;

typedef struct _ParasitePathTreeClass {
    GtkTreeViewClass parent_class;

    void (*reserved0)(void);
    void (*reserved1)(void);
    void (*reserved2)(void);
    void (*reserved3)(void);
} ParasitePathTreeClass;


G_BEGIN_DECLS

GType parasite_pathtree_get_type();
GtkWidget *parasite_pathtree_new();
void parasite_pathtree_set_widget(ParasitePathTree* path_tree,
				  GtkWidget *widget);


G_END_DECLS

#endif
