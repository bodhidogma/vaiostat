/*
 * File:        vaiostat.h
 * Author:      Paul McAvoy <paulmcav@queda.net>
 * 
 * $Id: vaiostat.h,v 1.2 2002-01-14 08:08:12 paulmcav Exp $             
 * 
 * Copyright (C) 2002 Paul McAvoy <paulmcav@queda.net>
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
*/

#ifndef _VAIOSTAT_H_
#define _VAIOSTAT_H_

#ifdef __KERNEL__

/* ioports used for brightness and events */
#define SONYPI_DATA_IOPORT      0x62
#define SONYPI_CST_IOPORT       0x66

#define wait_on_command(command) { \
	unsigned int n = 10000; \
	while (--n && (command)) \
	        udelay(100); \
	if (!n) \
	        printk(KERN_WARNING "vaio command failed at " __FILE__ " : " __FUNCTION__ "(line %d)\n", __LINE__); \
}

#define VAIOSTAT_MAJORVERSION        1
#define VAIOSTAT_MINORVERSION        1

#endif

#endif
