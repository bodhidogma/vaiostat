// File:        vaiobat.c
// Author:      Paul McAvoy <paulmcav@queda.net>
// 
// $Id: vaiobat.c,v 1.2 2002-11-11 07:37:19 paulmcav Exp $
/*
 * Vaio Battery / Power information gkrellm module.
 * Copyright (C) 2002 Paul McAvoy <paulmcav@queda.net>
 *
 * vaiobat displays power sources as well as battery % / time remaining.
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
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 * Version 1.2 2002/10/15 23:00:00 markus: Support for battery 2 enabled.
 *

*/

#include <gkrellm/gkrellm.h>

#include "vaiobat.h"

#define CONFIG_NAME "VaioBat"
#define STYLE_NAME  "VaioBat"

#define KRELL_FULL_SCALE    100

static Monitor  *monitor;
static Panel    *panel;

static Decal    *ac_decal,
				*batt1_decal,
				*batt2_decal,
				*apm_text;
static Krell    *apm_krell;

static gint     style_id;

static struct vaio_data _vaio;

#define UPDATE_INTERVAL 50   // seconds / 10

// ------------------------------------------------------------------
/**
 * Read information from the kernel module at /proc/vaio/status
 * 
 * @param  vdata pointer to vaio_data information
 * 
 * @return 0 = okay
*/

int
read_vaio_data( struct vaio_data *vdata )
{
	FILE *fh;
	char  buff[256], *ptr;

	vdata->lcd_level = 0;
	vdata->power_src = 0;
	vdata->power_pctt = 0;
	vdata->power_pct1 = 0;
	vdata->power_pct2 = 0;
	vdata->power_time = 0;
	
	if ( !(fh = fopen("/proc/vaio/status", "r")) ){
		return 0;
	}
	
	fgets( buff, sizeof(buff), fh ); // lcd level
	sscanf( buff, "%*s : %d", &vdata->lcd_level );

	fgets( buff, sizeof(buff), fh ); // power sources
	if ( (ptr = strchr( buff, '1' )) )
		vdata->power_src |= 1;
	if ( (ptr = strchr( buff, '2' )) )
		vdata->power_src |= 2;
	if ( (ptr = strchr( buff, 'C' )) )
		vdata->power_src |= 4;

	fgets( buff, sizeof(buff), fh ); // power level
	sscanf( buff, "%*s : %*d/%*d %*d/%*d %d%% %d%% %d%% %d"
			, &vdata->power_pct1, &vdata->power_pct2
			, &vdata->power_pctt, &vdata->power_time
		 );

	fclose( fh );
	return 0;
}

// ------------------------------------------------------------------
/** Callback when the panel has been exposed.
 * 
 * @param widget widget object
 * @param ev event message
 *
 * @return FALSE
*/

static gint
panel_expose_event( GtkWidget *widget, GdkEventExpose *ev )
{
	gdk_draw_pixmap( widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget) ],
			panel->pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
			ev->area.width, ev->area.height
			);
	return FALSE;
}

#define HR_TIME( x )	(int)(x / 3600)
#define MN_TIME( x )	(int)(((x / 3600.0) - (x / 3600)) * 60)

// ------------------------------------------------------------------
/** Callback to re-draw all the various stuff on the panel.
 * Includes drawing power sources, and battery % or time left.
*/

static void
update_plugin()
{
	static gint w;
	char   buff[16];

	if ( w == 0 )
		w = gkrellm_chart_width();

  if ((GK.timer_ticks % UPDATE_INTERVAL) == 0) {
	  
		read_vaio_data( &_vaio );
		
		gkrellm_update_krell( panel, apm_krell,
				_vaio.power_pctt	
				);
	
	if ( _vaio.power_src & 1 ){
		gkrellm_draw_decal_pixmap( panel, batt1_decal, D_MISC_BATTERY );
	}
	else {
		gkrellm_draw_decal_pixmap( panel, batt1_decal, D_MISC_BLANK );
	}

	if ( _vaio.power_src & 4 ){
		// ac power connected; ac symbol replaces batt2
		gkrellm_draw_decal_pixmap( panel, batt2_decal, D_MISC_BLANK );
		gkrellm_draw_decal_pixmap( panel, ac_decal, D_MISC_AC );
	}
	else {
		// no ac power, only batteries
		gkrellm_draw_decal_pixmap( panel, ac_decal, D_MISC_BLANK );
		if ( _vaio.power_src & 2 ){
			gkrellm_draw_decal_pixmap( panel, batt2_decal, D_MISC_BATTERY );
		}
		else {
			gkrellm_draw_decal_pixmap( panel, batt2_decal, D_MISC_BLANK );
		}
	}

	// battery capacity percentage is always shown, even with ac
	// remaining time only without ac power
	if ( _vaio.display_time && !( _vaio.power_src & 4) )
		sprintf( buff, "%d:%.2d"
		, HR_TIME( _vaio.power_time)
		, MN_TIME( _vaio.power_time) );
	else
		sprintf( buff, "%d%%", _vaio.power_pctt );

	gkrellm_draw_decal_text( panel, apm_text, buff, -1 );

	gkrellm_draw_panel_layers( panel );
  }
}

// ------------------------------------------------------------------
/** Callback when a mouse button was pressed on the panel.
 * 
 * @param widget some passed widget
 * @param ev     event message information
 *
 * @return       TRUE
*/

static gint
cb_panel_press( GtkWidget *widget, GdkEventButton *ev)
{
	char   buff[16];

	// don't care which button was pressed
	if ( ev->button ){

		// toggle time display
		if ( (_vaio.power_src & 0x4) && !_vaio.display_time ){
			gkrellm_message_window(_("GKrellM VaioBat"),
					_("When plugged into AC, time left is very long!"), NULL );
		}
		else {
			_vaio.display_time ^= 1;

			if ( _vaio.display_time )
				sprintf( buff, "%d:%.2d"
						, HR_TIME( _vaio.power_time)
						, MN_TIME( _vaio.power_time) );
			else
				sprintf( buff, "%d%%", _vaio.power_pctt );
			
			gkrellm_draw_decal_text( panel, apm_text, buff, -1 );

			gkrellm_draw_panel_layers( panel );
		}
	}
	return TRUE;
}

// ------------------------------------------------------------------
/** Create our plugin panel
 *
 * @param vbox widget to draw stuff in
 * @param first_create if we are a poppin fresh krellm
*/

static void
create_plugin( GtkWidget *vbox, gint first_create )
{
	Style         *style;
	TextStyle     *ts, *ts_alt;
	GdkPixmap     *pixmap;
	GdkBitmap     *mask;
	GdkImlibImage *krell_image;
	gint          x;

	int        bat_cnt;
	
	if ( first_create ) {
		memset( &_vaio, 0, sizeof(_vaio) );
		panel = gkrellm_panel_new0();
	}

	style = gkrellm_meter_style( style_id );

	krell_image = gkrellm_krell_meter_image( style_id );
	apm_krell = gkrellm_create_krell( panel, krell_image, style );

	gkrellm_monotonic_krell_values( apm_krell, FALSE );
	gkrellm_set_krell_full_scale( apm_krell, KRELL_FULL_SCALE, 1 );
	
	ts     = gkrellm_meter_textstyle( style_id );
	ts_alt = gkrellm_meter_alt_textstyle( style_id );

	pixmap = gkrellm_decal_misc_pixmap();
	mask = gkrellm_decal_misc_mask();

	read_vaio_data( &_vaio );
	
	bat_cnt = _vaio.power_src;

	x = 4;
	// create the battery image decals
	batt1_decal = gkrellm_create_decal_pixmap( panel, pixmap, mask
		, N_MISC_DECALS, NULL, x, 2 );
	gkrellm_draw_decal_pixmap( panel, batt1_decal, D_MISC_BATTERY );

	x = batt1_decal->x + batt1_decal->w + 4;
	batt2_decal = gkrellm_create_decal_pixmap( panel, pixmap, mask
		, N_MISC_DECALS, NULL, x, 2 );
	gkrellm_draw_decal_pixmap( panel, batt2_decal, D_MISC_BATTERY );
	
	x = batt2_decal->x + batt2_decal->w + 4;

	// create the AC image decal
	ac_decal = gkrellm_create_decal_pixmap( panel, pixmap, mask
		, N_MISC_DECALS, NULL, x, 2 );
	gkrellm_draw_decal_pixmap( panel, ac_decal, D_MISC_AC );
	
	x = ac_decal->x + ac_decal->w + 4;
	
	apm_text = gkrellm_create_decal_text( panel, "100",
			ts_alt, style, x, 3, -1 );
	
	// adjust positions
	apm_text->x = ac_decal->x;	// capacity percentage rightmost
	ac_decal->x = batt2_decal->x;	// batt2 or ac symbol in the middle

	gkrellm_panel_configure( panel, NULL, style );
	gkrellm_panel_create( vbox, monitor, panel );

	if ( first_create ) {
		gtk_signal_connect( GTK_OBJECT( panel->drawing_area )
				, "expose_event", (GtkSignalFunc) panel_expose_event, NULL );
		gtk_signal_connect( GTK_OBJECT( panel->drawing_area )
				, "button_press_event", (GtkSignalFunc) cb_panel_press, NULL );
	}
}

// ------------------------------------------------------------------

static Monitor plugin_mon = 
{
	CONFIG_NAME,        /* Name, for config tab.    */
	0,					/* Id,  0 if a plugin       */
	create_plugin,		/* The create function      */
	update_plugin,		/* The update function      */
	NULL,				/* The config tab create function   */
	NULL,				/* Apply the config function        */

	NULL,				/* Save user config			*/
	NULL,				/* Load user config			*/
	NULL,				/* config keyword			*/

	NULL,				/* Undefined 2	*/
	NULL,				/* Undefined 1	*/
	NULL,				/* private		*/

	MON_UPTIME,			/* Insert plugin before this monitor			*/

	NULL,				/* Handle if a plugin, filled in by GKrellM     */
	NULL				/* path if a plugin, filled in by GKrellM       */
};

// ------------------------------------------------------------------
/** Initialize the gkrellm plugin 
 *
 * @return Monitor struct pointer
*/

Monitor *
init_plugin()
{
	style_id = gkrellm_add_meter_style( &plugin_mon, STYLE_NAME );
	monitor = &plugin_mon;
	return &plugin_mon;
}

