/**
 * @file        vaiostat.c
 * Author:      Paul McAvoy <paulmcav@queda.net>
 * 
 * $Id: vaiostat.c,v 1.4 2002-11-09 08:28:31 paulmcav Exp $
 * 
 * Vaio status / control kernel module
 * Copyright (C) 2002 Paul McAvoy <paulmcav@queda.net>
 *
 * 'vaiostat' is a simple kernel module used to examine and control some
 * information about a Sony Vaio laptop.  This version includes support for:
 *  LCD brightness information.
 *  Battery / Power status information.
 *
 * The majority of the code used to get / set the information for the above
 * parameters was found in various sources including:
 * 
 *  linux/drivers/char/sonypi.h     Stelian Pop <stelian.pop@fr.alcove.com>
 *  picturebook/vaiobat.c           Andrew Tridgell <tridge@linuxcare.com>
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
 *
 *
 * 2002/10/15 22:30:00 Markus Ammer <IB-Ammer@t-online.de>
 *      Support for battery 2 added.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include "vaiostat.h"

#define BUFF_LEN 256
static char ctrl_msg[ BUFF_LEN ];

static int verbose = 0;

#define LCD_NUM_STEPS  8

#define LCD_LEVEL   0x96
#define PWR_SRCS    0x81

#define B0_PCTRM	0xa0
#define B0_MAXTK	0xb0
#define B0_LFTTK1	0xa2   // left capacity bat1
#define B0_LFTTK2	0xaa   // left capacity bat2
#define B0_FULTK1	0xb2   // full capacity bat1
#define B0_FULTK2	0xba   // full capacity bat2
#define B0_MAXRT1	0xa4	/* max run time (s) = # / 7 */
#define B0_MAXRT2	0xac	/* max run time (s) = # / 7 */

// ------------------------------------------------------------------
/** set 16 bit register value
 *
 * @param addr  register address
 * @param value value to set
*/

static void ecr_set(u16 addr, u16 value)
{
	wait_on_command( inw_p(SONYPI_CST_IOPORT) & 3 );
    outw_p(0x81, SONYPI_CST_IOPORT);
	wait_on_command( inw_p(SONYPI_CST_IOPORT) & 2 );
    outw_p(addr, SONYPI_DATA_IOPORT);
	wait_on_command( inw_p(SONYPI_CST_IOPORT) & 2 );
    outw_p(value, SONYPI_DATA_IOPORT);
	wait_on_command( inw_p(SONYPI_CST_IOPORT) & 2 );
}

// ------------------------------------------------------------------
/** get 8 bit register value
 *
 * @param addr register address
 *
 * @return 8 bit value
*/

static u8 ecr_get8(u8 addr)
{
	wait_on_command( inb_p(SONYPI_CST_IOPORT) & 3 );
	outb(0x80, SONYPI_CST_IOPORT);
	wait_on_command( inb_p(SONYPI_CST_IOPORT) & 2 );
	outb(addr, SONYPI_DATA_IOPORT);
	wait_on_command( inb_p(SONYPI_CST_IOPORT) & 2 );
	return inb_p(SONYPI_DATA_IOPORT);
}

// ------------------------------------------------------------------
/** get 16 bit register value
 *
 * @param addr register address
 *
 * @return 16 bit value
*/

static u16 ecr_get16(u8 addr)
{
	return ecr_get8(addr) | (ecr_get8(addr+1)<<8);
}

// ------------------------------------------------------------------
/** write message to proc file system
 *
 * @param page  output buffer
 * @param start
 * @param off
 * @param count
 * @param eof
 * @param data
 *
 * @return buffer length
*/

static int
write_status_info(
		char  *page,
		char **start,
		off_t  off,
		int    count,
		int   *eof,
		void  *data
			 )
{
	int len = 0;
	int v1,v11,v21,v12,v22,v3;
	int ac, mt1, mt2, sec_left, h_left, m_left;

	if ( off > 0 )
		return 0;

	/* get LCD brightness level */
	v1 = ecr_get8( LCD_LEVEL ) / (255/LCD_NUM_STEPS);
	len += sprintf( page+len,
			"lcd_lvl\t : %d\n"
			,v1
			);

	/* get power source / remaining information */
	v3 = ecr_get16( PWR_SRCS );

	//  battery 1
	mt1 = ecr_get16( B0_MAXRT1 ) / 7;
	v11 = ecr_get16( B0_LFTTK1 );
	v21 = ecr_get16( B0_FULTK1 );
	
	//  battery 2
	if (v3 & 0x2) {
		mt2 = ecr_get16( B0_MAXRT2 ) / 7;
		v12 = ecr_get16( B0_LFTTK2 );
		v22 = ecr_get16( B0_FULTK2 );
		}
	else {	
		mt2 = 0;
		v12 = 0;
		v22 = 1;
		}			
	
	ac = (v3 & 0x4);
	sec_left = (int) ((mt1 / (float)v21 * v11) + (mt2 / (float)v22 *v12 )) ;

	if ( ac ) {
		sec_left = 0;
		h_left = 0;
		m_left = 0;
	}
	else {
		h_left = sec_left / 3600;
		m_left = ((sec_left / 3600.0) - h_left) * 60;
	}

	len += sprintf( page+len,
			"pw_src\t : %s%s%s\n"
			"pw_lvl\t : %d/%d %d/%d %d%% %d%% %d%% %d %d:%.2d\n"
			, (ac ? "AC " : "" )
			, (v3 & 0x1 ? "BAT1 " : "" )
			, (v3 & 0x2 ? "BAT2 " : "" )
	//  bat1 remaining/full, bat2 remaining/full
			, v11, v21, v12, v22
	//  bat1 percent, bat2 percent, (bat1+bat2) percent, sec h min remaining
			, 100*v11/v21, 100*v12/v22, 100*(v11+v12)/(v21+v22)
			, sec_left, h_left, m_left
			);
	
	/* print out the status message buffer */
	if ( len <= off+count ) *eof = 1;
	*start = page + off;
	len -= off;
	if ( len > count ) len = count;
	if ( len < 0 ) len = 0;

	return len;
}

// ------------------------------------------------------------------
/** convert string to integer
 *
 * @param buff input buffer
 *
 * @return integer value
*/

int
atoi( char *buff )
{
	int val = 0;
	int pos = 1;
	char *ptr = buff;

	while ( *ptr && (*ptr >= '0' && *ptr <= '9') ) ptr++;
	while ( ptr-- > buff ){
		val += (pos * (*ptr-'0'));
		pos *= 10;
	}
	return val;
}

// ------------------------------------------------------------------
/** lcd proc input conversion routine
 *
 * @param file   proc file handle
 * @param buffer input buffer
 * @param count  buffer length
 * @param data
 *
 * @return       number of bytes read
*/

static int
vaio_lcd_ctrl(
		struct file *file,
		const char *buffer,
		unsigned long count,
		void *data
		)
{
	int i = 0;
	int val = 0;
	for (i=0; i<BUFF_LEN-1 && i<count; i++)
		get_user( ctrl_msg[i], buffer+i );

	ctrl_msg[i] = '\0';
	 
	val = atoi(ctrl_msg);
	if ( val > LCD_NUM_STEPS ) val = LCD_NUM_STEPS;

	/* adjust lcd brightness */
	ecr_set( 0x96, val *= (255/LCD_NUM_STEPS) );

	if ( verbose ) printk( "vaiostat:lcd_bright = %d\n", val );

	return i;
}

// ------------------------------------------------------------------
/** module init function
 *
 * @return module init return value
*/

static int __init
vaio_init_module(void)
{
	struct proc_dir_entry *vaio_dir;
	struct proc_dir_entry *entry;

	vaio_dir = proc_mkdir( "vaio", &proc_root );

	/* our info status handler */
	entry = create_proc_entry( "status", 0666, vaio_dir );
	entry->read_proc  = write_status_info;
	
	entry = create_proc_entry( "lcd", 0222, vaio_dir );
	entry->write_proc = vaio_lcd_ctrl;

/*	if ( verbose ) printk( "vaiostat:init_module\n" );
*/
	return 0;
}

// ------------------------------------------------------------------
/** module exit function
 *
 * 
*/
static void __exit
vaio_exit_module( void )
{
	remove_proc_entry( "vaio", &proc_root );

/*	if ( verbose ) printk( "vaiostat:cleanup_module\n" );
*/
}

/* module entry points */

module_init( vaio_init_module );
module_exit( vaio_exit_module );

/* module informationals & parameter stuff */

MODULE_AUTHOR("Paul McAvoy <paulmcav@queda.net>");
MODULE_DESCRIPTION("Support selected Sony Vaio features.");
MODULE_LICENSE("GPL");

MODULE_PARM(verbose,"i");
MODULE_PARM_DESC(verbose, "be verbose, default is 0 (no)");

EXPORT_NO_SYMBOLS;

