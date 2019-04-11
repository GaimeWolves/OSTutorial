#include "floppy.h"
#include "../../cpu/isr.h"
#include "../../cpu/timer.h"
#include "../drivers/ports.h"
#include "../drivers/terminal.h"
#include "../util/string.h"

typedef struct
{
	char available;
	char id;
	char perpendicular;
	char is_large_disk;
	char datarate;
	char sectors;
} disk_t;

//IO Port IDs
enum FLPYDSK_IO 
{
	FLPYDSK_DOR = 0x3f2,
	FLPYDSK_MSR = 0x3f4,
	FLPYDSK_DATA = 0x3f5,
	FLPYDSK_CTRL = 0x3f7
};

//Masks for DOR Command Assembly (increased readability)
enum FLPYDSK_DOR_MASK 
{
	FLPYDSK_DOR_MASK_DRIVE0 = 0,		//00000000
	FLPYDSK_DOR_MASK_DRIVE1 = 1,		//00000001
	FLPYDSK_DOR_MASK_DRIVE2 = 2,		//00000010
	FLPYDSK_DOR_MASK_DRIVE3 = 3,		//00000011
	FLPYDSK_DOR_MASK_RESET = 4,			//00000100
	FLPYDSK_DOR_MASK_DMA = 8,			//00001000
	FLPYDSK_DOR_MASK_DRIVE0_MOTOR = 16,	//00010000
	FLPYDSK_DOR_MASK_DRIVE1_MOTOR = 32,	//00100000
	FLPYDSK_DOR_MASK_DRIVE2_MOTOR = 64,	//01000000
	FLPYDSK_DOR_MASK_DRIVE3_MOTOR = 128	//10000000
};

//Status Register Masks
enum FLPYDSK_MSR_MASK 
{
	FLPYDSK_MSR_MASK_DRIVE1_POS_MODE = 1,	//00000001
	FLPYDSK_MSR_MASK_DRIVE2_POS_MODE = 2,	//00000010
	FLPYDSK_MSR_MASK_DRIVE3_POS_MODE = 4,	//00000100
	FLPYDSK_MSR_MASK_DRIVE4_POS_MODE = 8,	//00001000
	FLPYDSK_MSR_MASK_BUSY = 16,				//00010000
	FLPYDSK_MSR_MASK_DMA = 32,				//00100000
	FLPYDSK_MSR_MASK_DATAIO = 64, 			//01000000
	FLPYDSK_MSR_MASK_DATAREG = 128			//10000000
};

//Floppy Disk Commands (Lower Bytes)
enum FLPYDSK_CMD 
{
	FDC_CMD_READ_TRACK = 2,
	FDC_CMD_SPECIFY = 3,
	FDC_CMD_CHECK_STAT = 4,
	FDC_CMD_WRITE_SECT = 5,
	FDC_CMD_READ_SECT = 6,
	FDC_CMD_CALIBRATE = 7,
	FDC_CMD_CHECK_INT = 8,
	FDC_CMD_WRITE_DEL_S = 9,
	FDC_CMD_READ_ID_S = 0xa,
	FDC_CMD_READ_DEL_S = 0xc,
	FDC_CMD_FORMAT_TRACK = 0xd,
	FDC_CMD_SEEK = 0xf,
	FDC_CMD_PERPENDICULAR = 0x12
};

//Floppy Disk Command Attributes
enum FLPYDSK_CMD_EXT 
{
	FDC_CMD_EXT_SKIP = 0x20,		//00100000
	FDC_CMD_EXT_DENSITY = 0x40,		//01000000
	FDC_CMD_EXT_MULTITRACK = 0x80	//10000000
};

//Space between sectors
enum FLPYDSK_GAP3_LENGTH 
{
	FLPYDSK_GAP3_LENGTH_STD = 42,
	FLPYDSK_GAP3_LENGTH_5_14 = 32,
	FLPYDSK_GAP3_LENGTH_3_5 = 27
};

//Bytes per sector
enum FLPYDSK_SECTOR_DTL 
{
	FLPYDSK_SECTOR_DTL_128 = 0,
	FLPYDSK_SECTOR_DTL_256 = 1,
	FLPYDSK_SECTOR_DTL_512 = 2,
	FLPYDSK_SECTOR_DTL_1024 = 3
};

enum FLPYDSK_SECTORS_PER_TRACK
{
	FLPYDSK_SECTORS_PER_TRACK_9 = 9,
	FLPYDSK_SECTORS_PER_TRACK_15 = 15,
	FLPYDSK_SECTORS_PER_TRACK_18 = 18,
	FLPYDSK_SECTORS_PER_TRACK_36 = 36
};

static unsigned volatile int IRQ_SET = 0;
static disk_t drive0, drive1;
static disk_t* current_drive;

// initialize DMA to use physical address 0x21000 - 0x30000
static void flpydsk_initialize_dma(int direction) 
{
	port_byte_out(0x0a, 0x06);	//mask dma channel 2
	port_byte_out(0xd8, 0xff);	//reset master flip-flop
	port_byte_out(0x04, 0);     //address=0x1000 
	port_byte_out(0x04, 0x10);
	port_byte_out(0xd8, 0xff);  //reset master flip-flop
	port_byte_out(0x05, 0xff);  //count to 0x23ff (number of bytes in a 3.5" floppy disk track)
	port_byte_out(0x05, 0x8F);
	port_byte_out(0x81, 0x02);     //external page register = 0

	switch (direction)
	{
	case 0:
		port_byte_out(0x0b, 0x56); //single transfer, address increment, autoinit, read, channel 2
		break;
	case 1:
		port_byte_out(0x0b, 0x5a); //single transfer, address increment, autoinit, write, channel 2
		break;
	default:
		port_byte_out(0x0b, 0x56); //single transfer, address increment, autoinit, read, channel 2
		break;
	}

	port_byte_out(0x0a, 0x02);  //unmask dma channel 2
}

static void floppy_irq(registers_t regs)
{
	IRQ_SET = 1;
}

static inline void flpydsk_wait_irq()
{
	while (IRQ_SET == 0);
	IRQ_SET = 0;
}

static inline void flpydsk_write_dor(unsigned char val) 
{
	port_byte_out(FLPYDSK_DOR, val);
}

static inline unsigned char flpydsk_read_status() 
{
	return port_byte_in(FLPYDSK_MSR);
}

static inline void flpydsk_write_ccr(unsigned char val) 
{
	port_byte_out(FLPYDSK_CTRL, val);
}

static void flpydsk_send_command(unsigned char cmd) 
{
	for (int i = 0; i < 500; i++)
		if (flpydsk_read_status() & FLPYDSK_MSR_MASK_DATAREG)
			return port_byte_out(FLPYDSK_DATA, cmd);
}

static unsigned char flpydsk_read_data() 
{
	for (int i = 0; i < 500; i++)
		if (flpydsk_read_status() & FLPYDSK_MSR_MASK_DATAREG)
			return port_byte_in(FLPYDSK_DATA);
	return 0;
}

static void flpydsk_disable_controller()
{
	flpydsk_write_dor(0);
}

static void flpydsk_enable_controller()
{
	flpydsk_write_dor(FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
}

static void flpydsk_check_int(unsigned char* st0, unsigned char* cyl) 
{
	flpydsk_send_command(FDC_CMD_CHECK_INT);

	*st0 = flpydsk_read_data();
	*cyl = flpydsk_read_data();
}

static void flpydsk_control_motor(unsigned char b) 
{
	unsigned char motor = current_drive->id == 0 ? FLPYDSK_DOR_MASK_DRIVE0_MOTOR : FLPYDSK_DOR_MASK_DRIVE1_MOTOR;

	//! turn on or off the motor of that drive
	if (b)
		flpydsk_write_dor(current_drive->id | motor | FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
	else
		flpydsk_write_dor(FLPYDSK_DOR_MASK_RESET);

	//! in all cases; wait a little bit for the motor to spin up/turn off
	sleep(100);
}

static int print_err(unsigned char* st0, unsigned char* st1, unsigned char* st2, unsigned char* rcy, unsigned char* rhe, unsigned char* rse, unsigned char* bps)
{
	int error = 0;
	if (*st0 & 0xC0) 
	{
		char* status[] = { 0, "error", "invalid command", "drive not ready" };
		terminal_writestring("floppy_do_sector: status = ");
		terminal_writeline(status[*st0 >> 6]);
		error = 1;
	}
	if (*st1 & 0x80) 
	{
		terminal_writeline("floppy_do_sector: end of cylinder");
		error = 1;
	}
	if (*st0 & 0x08) 
	{
		terminal_writeline("floppy_do_sector: drive not ready");
		error = 1;
	}
	if (*st1 & 0x20) 
	{
		terminal_writeline("floppy_do_sector: CRC error");
		error = 1;
	}
	if (*st1 & 0x10) 
	{
		terminal_writeline("floppy_do_sector: controller timeout");
		error = 1;
	}
	if (*st1 & 0x04) 
	{
		terminal_writeline("floppy_do_sector: no data found");
		error = 1;
	}
	if ((*st1 | *st2) & 0x01) 
	{
		terminal_writeline("floppy_do_sector: no address mark found");
		error = 1;
	}
	if (*st2 & 0x40) 
	{
		terminal_writeline("floppy_do_sector: deleted address mark");
		error = 1;
	}
	if (*st2 & 0x20) 
	{
		terminal_writeline("floppy_do_sector: CRC error in data");
		error = 1;
	}
	if (*st2 & 0x10) 
	{
		terminal_writeline("floppy_do_sector: wrong cylinder");
		error = 1;
	}
	if (*st2 & 0x04)
	{
		terminal_writeline("floppy_do_sector: uPD765 sector not found");
		error = 1;
	}
	if (*st2 & 0x02) 
	{
		terminal_writeline("floppy_do_sector: bad cylinder");
		error = 1;
	}
	if (*bps != 0x2) 
	{
		terminal_writestring("floppy_do_sector: wanted 1024B/sector, got ");
		char* str = "0000";
		int_to_str((1 << (*bps + 7)), str);
		terminal_writeline(str);
		error = 1;
	}
	if (*st1 & 0x02) 
	{
		terminal_writeline("floppy_do_sector: not writable");
		error = 2;
	}
	return error;
}

static int flpydsk_read_write_sector_imp(unsigned char write, unsigned char head, unsigned char track, unsigned char sector) 
{
	unsigned char st0, cyl;

	//! set the DMA for read transfer
	flpydsk_initialize_dma(write);

	//! read in a sector
	flpydsk_send_command(write == 1 ? FDC_CMD_WRITE_SECT : FDC_CMD_READ_SECT | FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_SKIP | FDC_CMD_EXT_DENSITY);
	flpydsk_send_command(head << 2 | current_drive->id);
	flpydsk_send_command(track);
	flpydsk_send_command(head);
	flpydsk_send_command(sector);
	flpydsk_send_command(current_drive->datarate);
	flpydsk_send_command(((sector + 1) >= current_drive->sectors) ? current_drive->sectors : sector + 1);
	flpydsk_send_command(current_drive->is_large_disk == 1 ? FLPYDSK_GAP3_LENGTH_5_14 : FLPYDSK_GAP3_LENGTH_3_5);
	flpydsk_send_command(0xff);

	//! wait for irq
	flpydsk_wait_irq();

	//! read status info
	unsigned char st1, st2, rcy, rhe, rse, bps;
	st0 = flpydsk_read_data();
	st1 = flpydsk_read_data();
	st2 = flpydsk_read_data();
	rcy = flpydsk_read_data();
	rhe = flpydsk_read_data();
	rse = flpydsk_read_data();
	bps = flpydsk_read_data();

	int error = print_err(&st0, &st1, &st2, &rcy, &rhe, &rse, &bps);

	flpydsk_check_int(&st0, &cyl);

	if (error > 1) 
	{
		terminal_writeline("floppy_do_sector: not retrying..");
		return -2;
	}
	return 0;
}

static void flpydsk_read_track_imp(unsigned char head, unsigned char track)
{
	unsigned char st0, cyl;

	//! set the DMA for read transfer
	flpydsk_initialize_dma(0);

	//! read in a sector
	flpydsk_send_command(FDC_CMD_READ_TRACK | FDC_CMD_EXT_DENSITY);
	flpydsk_send_command(head << 2 | current_drive->id);
	flpydsk_send_command(track);
	flpydsk_send_command(head);
	flpydsk_send_command(1);
	flpydsk_send_command(current_drive->datarate);
	flpydsk_send_command(current_drive->sectors);
	flpydsk_send_command(current_drive->is_large_disk == 1 ? FLPYDSK_GAP3_LENGTH_5_14 : FLPYDSK_GAP3_LENGTH_3_5);
	flpydsk_send_command(0xff);

	//! read status info
	unsigned char st1, st2, rcy, rhe, rse, bps;
	st0 = flpydsk_read_data();
	st1 = flpydsk_read_data();
	st2 = flpydsk_read_data();
	rcy = flpydsk_read_data();
	rhe = flpydsk_read_data();
	rse = flpydsk_read_data();
	bps = flpydsk_read_data();

	int error = print_err(&st0, &st1, &st2, &rcy, &rhe, &rse, &bps);

	flpydsk_check_int(&st0, &cyl);

	if (error > 1) 
	{
		terminal_writeline("floppy_do_sector: not retrying..");
	}
}

static void flpydsk_drive_data(unsigned char steprate, unsigned char loadt, unsigned char unloadt, unsigned char dma)
{
	unsigned char data = 0;

	flpydsk_send_command(FDC_CMD_SPECIFY);

	data = ((steprate & 0xf) << 4) | (unloadt & 0xf);
	flpydsk_send_command(data);

	data = (loadt) << 1 | (dma == 1) ? 1 : 0;
	flpydsk_send_command(data);
}

static char flpydsk_calibrate(unsigned char drive) 
{
	unsigned char st0, cyl;

	if (drive >= 2)
		return -2;

	//! turn on the motor
	flpydsk_control_motor(1);

	for (int i = 0; i < 10; i++) 
	{
		//! send command
		flpydsk_send_command(FDC_CMD_CALIBRATE);
		flpydsk_send_command(drive);
		flpydsk_wait_irq();
		flpydsk_check_int(&st0, &cyl);

		//! did we find cylinder 0? if so, we are done
		if (!cyl) 
		{
			flpydsk_control_motor(0);
			return 0;
		}
	}

	flpydsk_control_motor(0);
	return -1;
}

static char flpydsk_seek(unsigned char cyl, unsigned char head) 
{
	unsigned char st0, cyl0;

	for (int i = 0; i < 10; i++) 
	{
		//! send the command
		flpydsk_send_command(FDC_CMD_SEEK);
		flpydsk_send_command((head) << 2 | current_drive->id);
		flpydsk_send_command(cyl);

		//! wait for the results phase IRQ
		flpydsk_wait_irq();
		flpydsk_check_int(&st0, &cyl0);

		//! found the cylinder?
		if (cyl0 == cyl)
			return 0;
	}

	return -1;
}

static void flpydsk_perpendicular(int d0, int d1)
{
	flpydsk_send_command(FDC_CMD_PERPENDICULAR);
	flpydsk_send_command((d1 << 3) | (d0 << 2));
}

static void flpydsk_reset() 
{
	unsigned char st0, cyl;

	//! reset the controller
	flpydsk_disable_controller();
	flpydsk_enable_controller();
	flpydsk_wait_irq();

	//! send CHECK_INT/SENSE INTERRUPT command to all drives
	for (int i = 0; i < 4; i++)
		flpydsk_check_int(&st0, &cyl);

	//! transfer speed 500kb/s
	flpydsk_write_ccr(current_drive->datarate);
	flpydsk_perpendicular(drive0.perpendicular, drive1.perpendicular);

	//! pass mechanical drive info. steprate=3ms, unload time=240ms, load time=16ms
	flpydsk_drive_data(3, 16, 240, 1);

	//! calibrate the disk
	flpydsk_calibrate(current_drive->id);
}

/////////////////////
// PUBLIC FUNCTION //
/////////////////////

void flpydsk_lba_to_chs(int lba, int *head, int *track, int *sector) 
{
	*head = (lba % (current_drive->sectors * 2)) / (current_drive->sectors);
	*track = lba / (current_drive->sectors * 2);
	*sector = lba % current_drive->sectors + 1;
}

void flpydsk_detect_drives()
{
	port_byte_out(0x70, 0x10);
	unsigned char drives = port_byte_in(0x71);

	drive0.available = 1;
	switch (drives >> 4)
	{
	case 1:
		drive0.datarate = FLPYDSK_SECTOR_DTL_256;
		drive0.sectors = FLPYDSK_SECTORS_PER_TRACK_9;
		drive0.perpendicular = 0;
		drive0.is_large_disk = 1;
		break;
	case 2:
		drive0.datarate = FLPYDSK_SECTOR_DTL_512;
		drive0.sectors = FLPYDSK_SECTORS_PER_TRACK_15;
		drive0.perpendicular = 0;
		drive0.is_large_disk = 1;
		break;
	case 3:
		drive0.datarate = FLPYDSK_SECTOR_DTL_256;
		drive0.sectors = FLPYDSK_SECTORS_PER_TRACK_9;
		drive0.perpendicular = 0;
		drive0.is_large_disk = 0;
		break;
	case 4:
		drive0.datarate = FLPYDSK_SECTOR_DTL_512;
		drive0.sectors = FLPYDSK_SECTORS_PER_TRACK_18;
		drive0.perpendicular = 0;
		drive0.is_large_disk = 0;
		break;
	case 5:
		drive0.datarate = FLPYDSK_SECTOR_DTL_1024;
		drive0.sectors = FLPYDSK_SECTORS_PER_TRACK_36;
		drive0.perpendicular = 1;
		drive0.is_large_disk = 0;
		break;
	default:
		drive0.available = 0;
		break;
	}

	drive1.available = 1;
	switch (drives & 0xf)
	{
	case 1:
		drive1.datarate = FLPYDSK_SECTOR_DTL_256;
		drive1.sectors = FLPYDSK_SECTORS_PER_TRACK_9;
		drive1.perpendicular = 0;
		drive1.is_large_disk = 1;
		break;
	case 2:
		drive1.datarate = FLPYDSK_SECTOR_DTL_512;
		drive1.sectors = FLPYDSK_SECTORS_PER_TRACK_15;
		drive1.perpendicular = 0;
		drive1.is_large_disk = 1;
		break;
	case 3:
		drive1.datarate = FLPYDSK_SECTOR_DTL_256;
		drive1.sectors = FLPYDSK_SECTORS_PER_TRACK_9;
		drive1.perpendicular = 0;
		drive1.is_large_disk = 0;
		break;
	case 4:
		drive1.datarate = FLPYDSK_SECTOR_DTL_512;
		drive1.sectors = FLPYDSK_SECTORS_PER_TRACK_18;
		drive1.perpendicular = 0;
		drive1.is_large_disk = 0;
		break;
	case 5:
		drive1.datarate = FLPYDSK_SECTOR_DTL_1024;
		drive1.sectors = FLPYDSK_SECTORS_PER_TRACK_36;
		drive1.perpendicular = 1;
		drive1.is_large_disk = 0;
		break;
	default:
		drive1.available = 0;
		break;
	}
}

unsigned char* flpydsk_read_track(char useLBA, int sectorLBA, int track, int head, int drive, char* sectors_to_end, char* offset)
{
	flpydsk_detect_drives();

	if (drive != 0 && drive != 1) return 0;
	if ((drive == 0 && drive0.available == 0) || (drive == 1 && drive1.available == 0))
	{
		terminal_writeline("No drive detected");
		return 0;
	}

	if (useLBA)
	{
		if (drive == 0)
		{
			*sectors_to_end = sectorLBA % drive0.sectors;
			*offset = drive0.sectors - *sectors_to_end;
		}
		else
		{
			*sectors_to_end = sectorLBA % drive1.sectors;
			*offset = drive1.sectors - *sectors_to_end;
		}
	}

	terminal_writeline("Reset controller");
	current_drive = drive == 0 ? &drive0 : &drive1;
	flpydsk_reset();

	if (useLBA == 1)
	{
		//! convert LBA sector to CHS
		track = 0;
		int sec;
		flpydsk_lba_to_chs(sectorLBA, &head, &track, &sec);

		terminal_writeline("Converted to CHS");
	}

	//! turn motor on and seek to track
	flpydsk_control_motor(1);
	terminal_writeline("Turned on motor");

	if (flpydsk_seek(track, 0) != 0)
		return 0;
	if (flpydsk_seek(track, 1) != 0)
		return 0;
	terminal_writeline("Seeked track");

	//! read sector and turn motor off
	flpydsk_read_track_imp(head, track);
	terminal_writeline("Read track");
	flpydsk_control_motor(0);
	terminal_writeline("Turned off Motor");

	//! warning: this is a bit hackish
	return (unsigned char*)DMA_ADDRESS;
}

unsigned char* flpydsk_read_write_sector(char write, int sectorLBA, char drive) 
{
	flpydsk_detect_drives();

	if (drive != 0 && drive != 1) return 0;
	if ((drive == 0 && drive0.available == 0) || (drive == 1 && drive1.available == 0))
	{
		terminal_writeline("No drive detected");
		return 0;
	}

	terminal_writeline("Reset controller");
	current_drive = drive == 0 ? &drive0 : &drive1;
	flpydsk_reset();

	//! convert LBA sector to CHS
	int head = 0, track = 0, sector = 1;
	flpydsk_lba_to_chs(sectorLBA, &head, &track, &sector);

	terminal_writeline("Converted to CHS");

	//! turn motor on and seek to track
	flpydsk_control_motor(1);
	terminal_writeline("Turned on motor");

	if (flpydsk_seek(track, 0) != 0)
		return 0;
	if (flpydsk_seek(track, 1) != 0)
		return 0;
	terminal_writeline("Seeked track");

	//! read sector and turn motor off
	flpydsk_read_write_sector_imp(write, head, track, sector);
	terminal_writeline("Read sector");
	flpydsk_control_motor(0);
	terminal_writeline("Turned off Motor");

	//! warning: this is a bit hackish
	return (unsigned char*)DMA_ADDRESS;
}

void init_floppy_driver(void)
{
	drive0.id = 0;
	drive1.id = 1;
	current_drive = &drive0;
	register_interrupt_handler(IRQ6, floppy_irq);
	flpydsk_detect_drives();
	flpydsk_initialize_dma(0);
	flpydsk_reset();
}