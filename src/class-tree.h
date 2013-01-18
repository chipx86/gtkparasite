/* -*- c-file-style: "gnu"; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (c) 2012  Peter Hurley <peter@hurleysoftware.com>
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
#ifndef _GTKPARASITE_CLASSTREE_H_
#define _GTKPARASITE_CLASSTREE_H_

#include <gtk/gtk.h>

#define PARASITE_TYPE_CLASSTREE		   (parasite_classtree_get_type())
#define PARASITE_CLASSTREE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PARASITE_TYPE_CLASSTREE, ParasiteClassTree))
#define PARASITE_CLASSTREE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  PARASITE_TYPE_CLASSTREE, ParasiteClassTreeClass))
#define PARASITE_IS_CLASSTREE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PARASITE_TYPE_CLASSTREE))
#define PARASITE_IS_CLASSTREE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  PARASITE_TYPE_CLASSTREE))
#define PARASITE_CLASSTREE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  PARASITE_TYPE_CLASSTREE, ParasiteClassTreeClass))


typedef struct _ParasiteClassTreePrivate ParasiteClassTreePrivate;

typedef struct _ParasiteClassTree {
    GtkTreeView parent;

    ParasiteClassTreePrivate *priv;
} ParasiteClassTree;

typedef struct _ParasiteClassTreeClass {
    GtkTreeViewClass parent_class;

    void (*reserved0)(void);
    void (*reserved1)(void);
    void (*reserved2)(void);
    void (*reserved3)(void);
} ParasiteClassTreeClass;


G_BEGIN_DECLS

GType parasite_classtree_get_type();
GtkWidget *parasite_classtree_new();
void parasite_classtree_set_widget(ParasiteClassTree* self, GtkWidget *widget);

G_END_DECLS

#endif
