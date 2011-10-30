// File:        vaiobat.h
// Author:      Paul McAvoy <paulmcav@queda.net>
// 
// $Id: vaiobat.h,v 1.2 2002-11-11 07:37:19 paulmcav Exp $
/*
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

#ifndef _VAIOBAT_H_
#define _VAIOBAT_H_

/**
 * vaio_data structure holds various state information
*/

struct vaio_data {
	gint lcd_level;
	gint power_src;    // 1=bat1, 2=bat2, 4=ac
	gint power_pctt;   // percent bat total
	gint power_pct1;   // percent bat 1
	gint power_pct2;   // percent bat 2
	gint power_time;   // remaining seconds

	gint display_time;
};

/**
*/

#endif


