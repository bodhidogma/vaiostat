// File:        vaiostat.c
// Author:      Paul McAvoy <paulmcav@queda.net>
// Org:
// Desc:        
// 
// $Revision: 1.1 $
/*
 * $Log: not supported by cvs2svn $
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <linux/delay.h>
#include <asm/io.h>

#include <linux/time.h>

#include "vaiostat.h"

#define BUFF_LEN 256
static char ctrl_msg[ BUFF_LEN ];

#define MAX_FNKEY   16

static int verbose = 0;

static struct v_data {
	int  fnkey_cnt;
	int  fnkeyn[ MAX_FNKEY ];
	char fnkeys[ MAX_FNKEY ][ BUFF_LEN ];
} _vaio;

static struct sonypi_device sonypi_device;

// ------------------------------------------------------------------
/**
 *
 * \param 
*/

/* Inits the queue */
static inline void sonypi_initq(void) {
        sonypi_device.queue.head = sonypi_device.queue.tail = 0;
	sonypi_device.queue.len = 0;
	sonypi_device.queue.s_lock = (spinlock_t)SPIN_LOCK_UNLOCKED;
	init_waitqueue_head(&sonypi_device.queue.proc_list);
}

/* Pulls an event from the queue */
static inline unsigned char sonypi_pullq(void) {
        unsigned char result;
	unsigned long flags;

	spin_lock_irqsave(&sonypi_device.queue.s_lock, flags);
	if (!sonypi_device.queue.len) {
		spin_unlock_irqrestore(&sonypi_device.queue.s_lock, flags);
		return 0;
	}
	result = sonypi_device.queue.buf[sonypi_device.queue.head];
        sonypi_device.queue.head++;
	sonypi_device.queue.head &= (SONYPI_BUF_SIZE - 1);
	sonypi_device.queue.len--;
	spin_unlock_irqrestore(&sonypi_device.queue.s_lock, flags);
        return result;
}

/* Pushes an event into the queue */
static inline void sonypi_pushq(unsigned char event) {
	unsigned long flags;

	spin_lock_irqsave(&sonypi_device.queue.s_lock, flags);
	if (sonypi_device.queue.len == SONYPI_BUF_SIZE) {
		/* remove the first element */
        	sonypi_device.queue.head++;
		sonypi_device.queue.head &= (SONYPI_BUF_SIZE - 1);
		sonypi_device.queue.len--;
	}
	sonypi_device.queue.buf[sonypi_device.queue.tail] = event;
	sonypi_device.queue.tail++;
	sonypi_device.queue.tail &= (SONYPI_BUF_SIZE - 1);
	sonypi_device.queue.len++;

	kill_fasync(&sonypi_device.queue.fasync, SIGIO, POLL_IN);
	wake_up_interruptible(&sonypi_device.queue.proc_list);
	spin_unlock_irqrestore(&sonypi_device.queue.s_lock, flags);
}

/* Tests if the queue is empty */
static inline int sonypi_emptyq(void) {
        int result;
	unsigned long flags;

	spin_lock_irqsave(&sonypi_device.queue.s_lock, flags);
        result = (sonypi_device.queue.len == 0);
	spin_unlock_irqrestore(&sonypi_device.queue.s_lock, flags);
        return result;
}


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


#ifdef NOTYET
static u16 ecr_get(u16 addr)
{
	wait_on_command(inw_p(SONYPI_CST_IOPORT) & 3);
	outw_p(0x80, SONYPI_CST_IOPORT);
	wait_on_command(inw_p(SONYPI_CST_IOPORT) & 2);
	outw_p(addr, SONYPI_DATA_IOPORT);
	wait_on_command(inw_p(SONYPI_CST_IOPORT) & 2);
	return inw_p(SONYPI_DATA_IOPORT);
}
#endif

static u8 ecr_get8(u8 addr)
{
	wait_on_command( inb_p(SONYPI_CST_IOPORT) & 3 );
	outb(0x80, SONYPI_CST_IOPORT);
	wait_on_command( inb_p(SONYPI_CST_IOPORT) & 2 );
	outb(addr, SONYPI_DATA_IOPORT);
	wait_on_command( inb_p(SONYPI_CST_IOPORT) & 2 );
	return inb_p(SONYPI_DATA_IOPORT);
}

static u16 ecr_get16(u8 addr)
{
	return ecr_get8(addr) | (ecr_get8(addr+1)<<8);
}

#define LCD_LEVEL   0x96
#define PWR_SRCS    0x81

#define B0_PCTRM	0xa0
#define B0_LFTTK	0xa2
#define B0_MAXRT    0xa4	/* max run time (s) = # / 7 */
#define B0_MAXTK	0xb0
#define B0_FULTK    0xb2

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
	int v1,v2,v3;
	int ac, mt, sec_left, h_left, m_left;
	int cnt;

	if ( off > 0 )
		return 0;

	/* get LCD brightness level */
	v1 = ecr_get8( LCD_LEVEL ) / 31;
	len += sprintf( page+len,
			"lcd_lvl\t : %d\n"
			,v1
			);

	/* get power source / remaining information */
	v3 = ecr_get16( PWR_SRCS );
	v1 = ecr_get16( B0_LFTTK );
	v2 = ecr_get16( B0_FULTK );
	mt = ecr_get16( B0_MAXRT ) / 7;

	ac = (v3 & 0x4);
	sec_left = (int)(mt / (float)v2 * v1);

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
			"pw_lvl\t : %d/%d %d%% %d %d:%d\n"
			, (ac ? "AC " : "" )
			, (v3 & 0x1 ? "BAT1 " : "" )
			, (v3 & 0x2 ? "BAT2 " : "" )
			, v1, v2
			, 100*v1/v2
			, sec_left, h_left, m_left
			);
	
	if ( verbose ){
		len += sprintf( page+len,
				"fn\t : verbose\n"
				);
	}
	for ( cnt = 0; cnt < _vaio.fnkey_cnt; cnt++ ){
		len += sprintf( page+len,
				"fn%d\t : %x = %s\n"
				, cnt
				, _vaio.fnkeyn[ cnt ], _vaio.fnkeys[ cnt ]
				);
	}
			
	/* print out the status message buffer */
	if ( len <= off+count ) *eof = 1;
	*start = page + off;
	len -= off;
	if ( len > count ) len = count;
	if ( len < 0 ) len = 0;

	return len;
}

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
	if ( val > 8 ) val = 8;

	/* adjust lcd brightness */
	ecr_set( 0x96, val *= 31 );

	if ( verbose ) printk( "vaiostat:lcd_bright = %d\n", val );

	return i;
}

static int
vaio_fnkey_ctrl(
		struct file *file,
		const char *buffer,
		unsigned long count,
		void *data
		)
{
	int i = 0;
//	int val = 0;
	for (i=0; i<BUFF_LEN-1 && i<count; i++)
		get_user( ctrl_msg[i], buffer+i );

	ctrl_msg[i] = '\0';
	 
	printk( "vaio fnkey val: %s", ctrl_msg );

	return i;
}

static int __init vaio_init_module(void)
{
	struct proc_dir_entry *vaio_dir;
	struct proc_dir_entry *entry;

	vaio_dir = proc_mkdir( "vaio", &proc_root );

	/* our info status handler */
	entry = create_proc_entry( "status", 0666, vaio_dir );
	entry->read_proc  = write_status_info;
	
	entry = create_proc_entry( "lcd", 0222, vaio_dir );
	entry->write_proc = vaio_lcd_ctrl;

	entry = create_proc_entry( "fnkey", 0222, vaio_dir );
	entry->write_proc = vaio_fnkey_ctrl;

	memset( &_vaio, 0, sizeof(_vaio) );
	
//	if ( verbose ) printk( "vaiostat:init_module\n" );

	return 0;
}

static void __exit
vaio_exit_module( void )
{
	remove_proc_entry( "vaio", &proc_root );

//	if ( verbose ) printk( "vaiostat:cleanup_module\n" );
}

/* module entry points */

module_init( vaio_init_module );
module_exit( vaio_exit_module );

MODULE_AUTHOR("Paul McAvoy <paulmcav@queda.net>");
MODULE_DESCRIPTION("Support selected Sony Vaio features.");
MODULE_LICENSE("GPL");

MODULE_PARM(verbose,"i");
MODULE_PARM_DESC(verbose, "be verbose, default is 0 (no)");

