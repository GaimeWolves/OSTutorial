#include "drivers/floppy.h"
#include "util/math.h"

static enum ATTRIBUTES
{
	READ_ONLY = 0x01,
	HIDDEN = 0x02,
	SYSTEM = 0x04,
	VOLUME_ID = 0x08,
	DIRECTORY = 0x10,
	ARCHIVE = 0x20
};

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

typedef struct
{
	struct entry_t* parent;
	unsigned short cluster;
	struct entry_t* child_entries;
	unsigned char attributes;
	unsigned char is_empty : 1;
	unsigned char is_dir : 1;
	char file_name[11];
	unsigned long file_size;
} entry_t;

static fat_BS_t fat_bs;
static fat_extBS_t fat_ext_bs;

static int total_sectors;
static int fat_size;
static int root_dir_sectors;
static int first_data_sector;
static int first_fat_sector;
static int data_sectors;
static int total_clusters;
static int first_root_dir_sector;

static entry_t* root_dir;
char* fat1;
char* fat2;

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

	total_sectors = (fat_bs.total_sectors_16 == 0) ? fat_bs.total_sectors_32 : fat_bs.total_sectors_16;
	fat_size = fat_bs.table_size_16;
	root_dir_sectors = ((fat_bs.root_entry_count * 32) + (fat_bs.bytes_per_sector - 1)) / fat_bs.bytes_per_sector;
	first_data_sector = fat_bs.reserved_sector_count + (fat_bs.table_count * fat_size) + root_dir_sectors;
	first_fat_sector = fat_bs.reserved_sector_count;
	data_sectors = total_sectors - first_data_sector;
	total_clusters = data_sectors / fat_bs.sectors_per_cluster;
	first_root_dir_sector = first_data_sector - root_dir_sectors;

	entry_t root_entries[fat_bs.root_entry_count];
	root_dir = &root_entries;

	char fat_1_arr[fat_size * fat_bs.bytes_per_sector];
	fat1 = &fat_1_arr;
	char fat_2_arr[fat_size * fat_bs.bytes_per_sector];
	fat2 = &fat_2_arr;
}

static void read_fs()
{
	unsigned char* addr = flpydsk_read_track(1, first_root_dir_sector, 0, 0, 0);
	for (int i = 0; i < fat_bs.root_entry_count; i++)
	{
		if (*addr == 0x00 || *addr == 0xE5)
		{
			addr += 32;
			root_dir[i].is_empty = 1;
		}
		else
		{
			root_dir[i].is_empty = 0;
			root_dir[i].parent = 0;
			for (int j = 0; j < 11; j++)
			{
				root_dir[i].file_name[11] = *(addr++);
			}
			root_dir[i].attributes = *(addr++);
			addr += 13;
			root_dir[i].cluster = swap_endian_short(*((unsigned short*)addr));
			addr += sizeof(short);
			root_dir[i].file_size = swap_endian_int(*((unsigned int*)addr));
			addr += sizeof(int);

			if (root_dir[i].attributes & DIRECTORY > 0)
				root_dir[i].is_dir = 1;
			else
				root_dir[i].is_dir = 0;
		}
	}
}

static entry_t* follow_path(char* path)
{
	
}

void write_file(char* path, char* data, unsigned long file_size)
{

}

void init_fs()
{
	parse_bs();
	read_fs();
}