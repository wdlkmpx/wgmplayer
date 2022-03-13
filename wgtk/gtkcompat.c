/*
 * Copyright (C) 2020
 *
 * gtkcompat, GTK2+ compatibility layer
 * 
 * 2020-08-25
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 */

#include "gtkcompat.h"

/* ================================================== */
/*                   GTK < 3.0                        */
/* ================================================== */

#if ! GTK_CHECK_VERSION (3, 0, 0)

void gtk_widget_set_halign (GtkWidget *widget, GtkAlign align)
{ // from gtk_misc_set_alignment ()
	GtkMisc * misc = GTK_MISC (widget);
	gfloat xalign = 0.0;
	switch (align)
	{
		case GTK_ALIGN_FILL:
		case GTK_ALIGN_START:  xalign = 0.0; break;
		case GTK_ALIGN_CENTER: xalign = 0.5; break;
		case GTK_ALIGN_END:    xalign = 1.0; break;
	}
	if (xalign != misc->xalign)
	{
		g_object_freeze_notify (G_OBJECT (misc));
		g_object_notify (G_OBJECT (misc), "xalign");
		misc->xalign = xalign;
		if (gtk_widget_is_drawable (widget))
			gtk_widget_queue_draw (widget);
		g_object_thaw_notify (G_OBJECT (misc));
	}
}

void gtk_widget_set_valign (GtkWidget *widget, GtkAlign align)
{ // from gtk_misc_set_alignment ()
	GtkMisc * misc = GTK_MISC (widget);
	gfloat yalign = 0.0;
	switch (align)
	{
		case GTK_ALIGN_FILL:
		case GTK_ALIGN_START:  yalign = 0.0; break;
		case GTK_ALIGN_CENTER: yalign = 0.5; break;
		case GTK_ALIGN_END:    yalign = 1.0; break;
	}
	if (yalign != misc->yalign)
	{
		g_object_freeze_notify (G_OBJECT (misc));
		g_object_notify (G_OBJECT (misc), "yalign");
		misc->yalign = yalign;
		if (gtk_widget_is_drawable (widget))
			gtk_widget_queue_draw (widget);
		g_object_thaw_notify (G_OBJECT (misc));
	}
}

void gtk_widget_set_margin_start  (GtkWidget *widget, gint margin)
{ // from gtk_misc_set_padding ()
	// deprecated since 3.12+: gtk_widget_set_margin_left
	// deprecated since 3.12+: gtk_widget_set_margin_right
	GtkRequisition *requisition;
	int xpad = margin;
	GtkMisc * misc = GTK_MISC (widget);
	if (xpad < 0) xpad = 0;
	if (xpad != misc->xpad)
	{
		g_object_freeze_notify (G_OBJECT (misc));
		if (xpad != misc->xpad)
			g_object_notify (G_OBJECT (misc), "xpad");
		requisition = &(GTK_WIDGET (misc)->requisition);
		requisition->width -= misc->xpad * 2;
		misc->xpad = xpad;
		requisition->width += misc->xpad * 2;
		if (gtk_widget_is_drawable (widget))
			gtk_widget_queue_resize (GTK_WIDGET (misc));
		g_object_thaw_notify (G_OBJECT (misc));
	}
}

void gtk_widget_set_margin_end (GtkWidget *widget, gint margin)
{
	// does nothing
}

void gtk_widget_set_margin_top (GtkWidget *widget, gint margin)
{ // from gtk_misc_set_padding ()
	GtkRequisition *requisition;
	int ypad = margin;
	GtkMisc * misc = GTK_MISC (widget);
	if (ypad < 0) ypad = 0;
	if (ypad != misc->ypad)
	{
		g_object_freeze_notify (G_OBJECT (misc));
		if (ypad != misc->ypad)
			g_object_notify (G_OBJECT (misc), "ypad");
		requisition = &(GTK_WIDGET (misc)->requisition);
		requisition->height -= misc->ypad * 2;
		misc->ypad = ypad;
		requisition->height += misc->ypad * 2;
		if (gtk_widget_is_drawable (widget))
			gtk_widget_queue_resize (GTK_WIDGET (misc));
		g_object_thaw_notify (G_OBJECT (misc));
	}
}

void gtk_widget_set_margin_bottom (GtkWidget *widget, gint margin)
{
	// does nothing
}

#endif

