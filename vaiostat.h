// File:        vaio.h
// Author:      Paul McAvoy <paulmcav@queda.net>
// Org:
// Desc:        
//              
// 
// $Revision: 1.1 $

/*
 * $Log: not supported by cvs2svn $
*/

#ifndef _VAIO_H_
#define _VAIO_H_


#ifdef __KERNEL__

#define VAIO_DRIVER_MAJORVERSION        1
#define VAIO_DRIVER_MINORVERSION        1


/* Normal models use those */
#define SONYPI_IRQ_PORT                 0x8034
#define SONYPI_IRQ_SHIFT                22
#define SONYPI_BASE                     0x50
#define SONYPI_G10A                     (SONYPI_BASE+0x14)
#define SONYPI_NORMAL_REGION_SIZE       0x08

/* R505 series specifics */
#define SONYPI_SIRQ             0x9b
#define SONYPI_SLOB             0x9c
#define SONYPI_SHIB             0x9d
#define SONYPI_R505_REGION_SIZE 0x20

/* ioports used for brightness and R505 events */
#define SONYPI_DATA_IOPORT      0x62
#define SONYPI_CST_IOPORT       0x66

/* The set of possible ioports */
struct sonypi_ioport_list {
	u16     port1;
	u16     port2;
};

#ifdef NOPE
static struct sonypi_ioport_list sonypi_normal_ioport_list[] = {
	{ 0x10c0, 0x10c4 },     /* looks like the default on C1Vx */
	{ 0x1080, 0x1084 },
	{ 0x1090, 0x1094 },
	{ 0x10a0, 0x10a4 },
	{ 0x10b0, 0x10b4 },
	{ 0x0, 0x0 }
};
#endif

#ifdef NOPE
static struct sonypi_ioport_list sonypi_r505_ioport_list[] = {
	{ 0x1080, 0x1084 },
	{ 0x10a0, 0x10a4 },
	{ 0x10c0, 0x10c4 },
	{ 0x10e0, 0x10e4 },
	{ 0x0, 0x0 }
};
#endif

/* The set of possible interrupts */
struct sonypi_irq_list {
	u16     irq;
	u16     bits;
};

#ifdef NOPE
static struct sonypi_irq_list sonypi_normal_irq_list[] = {
	{ 11, 0x2 },    /* IRQ 11, GO22=0,GO23=1 in AML */
	{ 10, 0x1 },    /* IRQ 10, GO22=1,GO23=0 in AML */
	{  5, 0x0 },    /* IRQ  5, GO22=0,GO23=0 in AML */
	{  0, 0x3 }     /* no IRQ, GO22=1,GO23=1 in AML */
};
#endif

#ifdef NOPE
static struct sonypi_irq_list sonypi_r505_irq_list[] = {
	{ 11, 0x80 },   /* IRQ 11, 0x80 in SIRQ in AML */
	{ 10, 0x40 },   /* IRQ 10, 0x40 in SIRQ in AML */
	{  9, 0x20 },   /* IRQ  9, 0x20 in SIRQ in AML */
	{  6, 0x10 },   /* IRQ  6, 0x10 in SIRQ in AML */
	{  0, 0x00 }    /* no IRQ, 0x00 in SIRQ in AML */
};
#endif

#define SONYPI_CAMERA_BRIGHTNESS                0
#define SONYPI_CAMERA_CONTRAST                  1
#define SONYPI_CAMERA_HUE                       2
#define SONYPI_CAMERA_COLOR                     3
#define SONYPI_CAMERA_SHARPNESS                 4

#define SONYPI_CAMERA_PICTURE                   5
#define SONYPI_CAMERA_EXPOSURE_MASK             0xC
#define SONYPI_CAMERA_WHITE_BALANCE_MASK        0x3
#define SONYPI_CAMERA_PICTURE_MODE_MASK         0x30
#define SONYPI_CAMERA_MUTE_MASK                 0x40

/* the rest don't need a loop until not 0xff */
#define SONYPI_CAMERA_AGC                       6
#define SONYPI_CAMERA_AGC_MASK                  0x30
#define SONYPI_CAMERA_SHUTTER_MASK              0x7

#define SONYPI_CAMERA_SHUTDOWN_REQUEST          7
#define SONYPI_CAMERA_CONTROL                   0x10

#define SONYPI_CAMERA_STATUS                    7
#define SONYPI_CAMERA_STATUS_READY              0x2
#define SONYPI_CAMERA_STATUS_POSITION           0x4

#define SONYPI_DIRECTION_BACKWARDS              0x4

#define SONYPI_CAMERA_REVISION                  8
#define SONYPI_CAMERA_ROMVERSION                9

/* key press event data (ioport2) */
#define SONYPI_NORMAL_JOGGER_EV 0x10
#define SONYPI_R505_JOGGER_EV   0x08
#define SONYPI_CAPTURE_EV       0x60
#define SONYPI_NORMAL_FNKEY_EV  0x20
#define SONYPI_R505_FNKEY_EV    0x08
#define SONYPI_BLUETOOTH_EV     0x30
#define SONYPI_NORMAL_PKEY_EV   0x40

struct sonypi_event {
	u8      data;
	u8      event;
};

/* The set of possible jogger events  */
#ifdef NOTUSED
static struct sonypi_event sonypi_joggerev[] = {
	{ 0x1f, SONYPI_EVENT_JOGDIAL_UP },
	{ 0x01, SONYPI_EVENT_JOGDIAL_DOWN },
	{ 0x5f, SONYPI_EVENT_JOGDIAL_UP_PRESSED },
	{ 0x41, SONYPI_EVENT_JOGDIAL_DOWN_PRESSED },
	{ 0x40, SONYPI_EVENT_JOGDIAL_PRESSED },
	{ 0x00, SONYPI_EVENT_JOGDIAL_RELEASED },
	{ 0x00, 0x00 }
};

/* The set of possible capture button events */
static struct sonypi_event sonypi_captureev[] = {
	{ 0x05, SONYPI_EVENT_CAPTURE_PARTIALPRESSED },
	{ 0x07, SONYPI_EVENT_CAPTURE_PRESSED },
	{ 0x01, SONYPI_EVENT_CAPTURE_PARTIALRELEASED },
	{ 0x00, SONYPI_EVENT_CAPTURE_RELEASED },
	{ 0x00, 0x00 }
};

/* The set of possible fnkeys events */
static struct sonypi_event sonypi_fnkeyev[] = {
	{ 0x10, SONYPI_EVENT_FNKEY_ESC },
	{ 0x11, SONYPI_EVENT_FNKEY_F1 },
	{ 0x12, SONYPI_EVENT_FNKEY_F2 },
	{ 0x13, SONYPI_EVENT_FNKEY_F3 },
	{ 0x14, SONYPI_EVENT_FNKEY_F4 },
	{ 0x15, SONYPI_EVENT_FNKEY_F5 },
	{ 0x16, SONYPI_EVENT_FNKEY_F6 },
	{ 0x17, SONYPI_EVENT_FNKEY_F7 },
	{ 0x18, SONYPI_EVENT_FNKEY_F8 },
	{ 0x19, SONYPI_EVENT_FNKEY_F9 },
	{ 0x1a, SONYPI_EVENT_FNKEY_F10 },
	{ 0x1b, SONYPI_EVENT_FNKEY_F11 },
	{ 0x1c, SONYPI_EVENT_FNKEY_F12 },
	{ 0x21, SONYPI_EVENT_FNKEY_1 },
	{ 0x22, SONYPI_EVENT_FNKEY_2 },
	{ 0x31, SONYPI_EVENT_FNKEY_D },
	{ 0x32, SONYPI_EVENT_FNKEY_E },
	{ 0x33, SONYPI_EVENT_FNKEY_F },
	{ 0x34, SONYPI_EVENT_FNKEY_S },
	{ 0x35, SONYPI_EVENT_FNKEY_B },
	{ 0x00, 0x00 }
};

/* The set of possible program key events */
static struct sonypi_event sonypi_pkeyev[] = {
	{ 0x01, SONYPI_EVENT_PKEY_P1 },
	{ 0x02, SONYPI_EVENT_PKEY_P2 },
	{ 0x04, SONYPI_EVENT_PKEY_P3 },
	{ 0x00, 0x00 }
};

/* The set of possible bluetooth events */
static struct sonypi_event sonypi_blueev[] = {
	{ 0x55, SONYPI_EVENT_BLUETOOTH_PRESSED },
	{ 0x00, 0x00 }
};
#endif

#define SONYPI_BUF_SIZE 128
struct sonypi_queue {
	unsigned long head;
	unsigned long tail;
	unsigned long len;
	spinlock_t s_lock;
	wait_queue_head_t proc_list;
	struct fasync_struct *fasync;
	unsigned char buf[SONYPI_BUF_SIZE];
};

#define SONYPI_DEVICE_MODEL_NORMAL      1
#define SONYPI_DEVICE_MODEL_R505        2

struct sonypi_device {
	struct pci_dev *dev;
	u16 irq;
	u16 bits;
	u16 ioport1;
	u16 ioport2;
	u16 region_size;
	int camera_power;
	struct semaphore lock;
	struct sonypi_queue queue;
	int open_count;
	int model;
};

#define wait_on_command(command) { \
	unsigned int n = 10000; \
	while (--n && (command)) \
	        udelay(100); \
	if (!n) \
	        printk(KERN_WARNING "vaio command failed at " __FILE__ " : " __FUNCTION__ "(line %d)\n", __LINE__); \
}

#endif

#endif