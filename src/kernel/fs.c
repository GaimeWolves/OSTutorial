#include "drivers/floppy.h"
#include "util/math.h"

typedef struct
{
	unsigned char		bios_drive_num;
	unsigned char		reserved1;
	unsigned char		boot_signature;
	unsigned int		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];

}__attribute__((packed)) fat_extBS_t;

typedef struct
{
	unsigned char 		oem_name[8];
	unsigned short 	    bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;

}__attribute__((packed)) fat_BS_t;

static fat_BS_t fat_bs;
static fat_extBS_t fat_ext_bs;

static void parse_bs()
{
	char* addr = flpydsk_read_write_sector(0, 0, 0);
	addr += 3;

	for (char i = 0; i < 8; i++)
		fat_bs.oem_name[7 - i] = (unsigned char)*(addr++);

	fat_bs.bytes_per_sector = swap_endian_short(*((unsigned short*)addr));
	addr += sizeof(short);
	fat_bs.sectors_per_cluster = *(addr++);
	fat_bs.reserved_sector_count = swap_endian_short(*((unsigned short*)addr));
	addr += sizeof(short);
	fat_bs.table_count = *(addr++);
	fat_bs.root_entry_count = swap_endian_short(*((unsigned short*)addr));
	addr += sizeof(short);
	fat_bs.total_sectors_16 = swap_endian_short(*((unsigned short*)addr));
	addr += sizeof(short);
	fat_bs.media_type = *(addr++);
	fat_bs.table_size_16 = swap_endian_short(*((unsigned short*)addr));
	addr += sizeof(short);
	fat_bs.sectors_per_track = swap_endian_short(*((unsigned short*)addr));
	addr += sizeof(short);
	fat_bs.head_side_count = swap_endian_short(*((unsigned short*)addr));
	addr += sizeof(short);
	fat_bs.hidden_sector_count = swap_endian_int(*((unsigned int*)addr));
	addr += sizeof(int);
	fat_bs.total_sectors_32 = swap_endian_int(*((unsigned int*)addr));
	addr += sizeof(int);

	fat_ext_bs.bios_drive_num = *(addr++);
	fat_ext_bs.reserved1 = *(addr++);
	fat_ext_bs.boot_signature = *(addr++);
	fat_ext_bs.volume_id = swap_endian_int(*((unsigned int*)addr));
	addr += sizeof(int);

	for (char i = 0; i < 11; i++)
		fat_ext_bs.volume_label[10 - i] = *(addr++);

	for (char i = 0; i < 8; i++)
		fat_ext_bs.fat_type_label[7 - i] = *(addr++);
}

void init_fs()
{
	parse_bs();
}