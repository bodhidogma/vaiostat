// File:        vaiolcd.c
// Author:      Paul McAvoy <paulmcav@queda.net>
// 
// $Id: vaiolcd.c,v 1.2 2002-06-21 17:57:53 paulmcav Exp $
/*
 * Sony Vaio LCD status / control gkrellm module.
 * Copyright (C) 2002 Paul McAvoy <paulmcav@queda.net>
 *
 * vaiolcd allows a user to see / set the brightness of the LCD screen.
 *
 * Requires the use of vaiostat kernel module to read Sony Vaio information.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#include <gkrellm/gkrellm.h>

#define CONFIG_NAME "VaioLcd"
#define STYLE_NAME  "VaioLcd"

/* the MAX_LCD value should match the kernel module LCD_NUM_STEPS value. */
#define MIN_LCD 0
#define MAX_LCD 8

static Monitor *monitor;
static Krell *krell;
static Panel *panel;
static Decal *text;

static int slider_in_motion;
static int last_lcd;

static gint     style_id;

// ------------------------------------------------------------------
/**
 * Read the current LCD brightness level.
 * We want to make sure that if something external set the value, we will
 * correctly display the change.
*/

int
read_vaio_lcd( void )
{
	FILE *fh;
	char  buff[256];
	int   lvl = 0;

	if ( !(fh = fopen("/proc/vaio/status", "r")) ){
		return 0;
	}
	
	fgets( buff, 128, fh ); // lcd level
	sscanf( buff, "%*s : %d\n", &lvl );

	fclose( fh );
	return lvl;
}

// ------------------------------------------------------------------
/**
 * write out users chosen value for the LCD brightness
 * 
 * \param LCD display brightness level (0-8)
*/

int
write_vaio_lcd( int lvl )
{
	FILE *fh;

	if ( !(fh = fopen("/proc/vaio/lcd", "w")) ){
		return 0;
	}

	fprintf( fh, "%d\n", lvl );
	last_lcd = -1;

	fclose( fh );
	return lvl;
}

// ------------------------------------------------------------------
/**
 * Callback when our module was exposed
 * 
 * \param widget our widget
 * \param event event information
 * \param data some extra data
*/

static gint
plug_expose_event( GtkWidget *widget, GdkEventExpose *event, gpointer data )
{
	if ( widget == panel->drawing_area ) {
		gdk_draw_pixmap( widget->window,
			   widget->style->fg_gc[ GTK_WIDGET_STATE(widget)],
			   panel->pixmap,
			   event->area.x, event->area.y,
			   event->area.x, event->area.y,
			   event->area.width, event->area.height
			   );
	}
	return FALSE;
}

// ------------------------------------------------------------------
/**
 * Callback when the mouse button was pressed
 * 
 * \param widget our widget
 * \param ev event information
*/

static void
panel_button_press( GtkWidget *widget, GdkEventButton *ev )
{
	gint x;
	
	if ( ev->button != 1 ) return;
	
	slider_in_motion = 1;

	x = ev->x * krell->full_scale / (gkrellm_chart_width() - 1);
	if ( x < MIN_LCD ) x = MIN_LCD;
	else if ( x > MAX_LCD ) x = MAX_LCD;

	write_vaio_lcd( x );
	
	gkrellm_update_krell( panel, krell, (gulong)x );
	gkrellm_draw_layers( panel );
}

// ------------------------------------------------------------------
/**
 * Callback when the mouse button was let go
 * 
 * \param widget this thing
 * \param ev event information
*/

static void
panel_button_release( GtkWidget *widget, GdkEventButton *ev )
{
	if ( ev->state & GDK_BUTTON1_MASK ){
		slider_in_motion = 0;
	}
}

// ------------------------------------------------------------------
/**
 * Callback to track the motion of the LCD slider
 * 
 * \param widget our widget?
 * \param ev event information
 * \param data some extra data
*/

static void
slider_motion( GtkWidget *widget, GdkEventButton *ev, gpointer data )
{
	gint x;
	GdkModifierType state;
	
	if ( !slider_in_motion ) return;

	state = ev->state;
	if ( !(state & GDK_BUTTON1_MASK)) {
		slider_in_motion = 0;
		return;
	}

	x = ev->x * krell->full_scale / (gkrellm_chart_width() - 1);
	if ( x < MIN_LCD ) x = MIN_LCD;
	else if ( x > MAX_LCD ) x = MAX_LCD;
	
	write_vaio_lcd( x );
	
	gkrellm_update_krell( panel, krell, (gulong)x );
	gkrellm_draw_layers( panel );
}

// ------------------------------------------------------------------
/**
 * Update the slider (LCD value) position
*/

static void
update_slider(void)
{
	int bright;
   
	if ( GK.ten_second_tick || (last_lcd<0) ) {
		bright = read_vaio_lcd();

		gkrellm_update_krell( panel, krell, bright );
	
		gkrellm_draw_decal_text( panel, text, "LCD", -1 );

		gkrellm_draw_layers( panel );
		last_lcd = bright;
	}
}

// ------------------------------------------------------------------
/**
 * Create our plugin setting up callbacks and such
 * 
 * \param vbox where to draw our objects
 * \param first_create if we are a poppin fresh krell
*/

static void
create_plugin( GtkWidget *vbox, gint first_create )
{
	GdkImlibImage   *krell_image;
	TextStyle		*ts, *ts_alt;
	Style 			*panel_style,
   					*slider_style;

	if ( first_create )
		panel = gkrellm_panel_new0();

	panel_style = gkrellm_meter_style( style_id );
	
	slider_style = gkrellm_krell_slider_style();
	krell_image = gkrellm_krell_slider_image();
	krell = gkrellm_create_krell( panel, krell_image, slider_style );

	gkrellm_monotonic_krell_values( krell, FALSE );
	gkrellm_set_krell_full_scale( krell, MAX_LCD, 1 );

	gkrellm_panel_configure( panel, NULL, panel_style );
	gkrellm_panel_create( vbox, monitor, panel );
	
	ts     = gkrellm_meter_textstyle( style_id );
	ts_alt = gkrellm_meter_alt_textstyle( style_id );

	text = gkrellm_create_decal_text( panel, "LCD",
			ts_alt, panel_style, 3, -1, -1 );
	
	if ( first_create ){
		gtk_signal_connect( GTK_OBJECT(panel->drawing_area),
				"expose_event", (GtkSignalFunc)plug_expose_event,
			   	NULL );
		gtk_signal_connect( GTK_OBJECT(panel->drawing_area),
				"button_press_event", (GtkSignalFunc)panel_button_press,
			   	NULL );
		gtk_signal_connect( GTK_OBJECT(panel->drawing_area),
				"button_release_event", (GtkSignalFunc)panel_button_release,
			   	NULL );
		gtk_signal_connect( GTK_OBJECT(panel->drawing_area),
				"motion_notify_event", (GtkSignalFunc)slider_motion,
			   	NULL );
	}
}

// ------------------------------------------------------------------

static Monitor plugin_mon = 
{
	CONFIG_NAME,        /* Name, for config tab.    */
	0,					/* Id,  0 if a plugin       */
	create_plugin,		/* The create function      */
	update_slider,		/* The update function      */
	NULL,				/* The config tab create function   */
	NULL,				/* Apply the config function        */

	NULL,				/* Save user config			*/
	NULL,				/* Load user config			*/
	NULL,				/* config keyword			*/

	NULL,				/* Undefined 2	*/
	NULL,				/* Undefined 1	*/
	NULL,				/* private		*/

	MON_APM,			/* Insert plugin before this monitor			*/

	NULL,				/* Handle if a plugin, filled in by GKrellM     */
	NULL				/* path if a plugin, filled in by GKrellM       */
};

// ------------------------------------------------------------------
/**
 * Init our gkrellm plugin module
*/

Monitor *
init_plugin(void)
{
	last_lcd = -1;
	style_id = gkrellm_add_meter_style( &plugin_mon, STYLE_NAME );
	monitor = &plugin_mon;

	return &plugin_mon;
}

