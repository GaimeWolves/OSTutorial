#include "fs.h"
#include "drivers/floppy.h"
#include "util/math.h"
#include "util/string.h"
#include "util/memory.h"

enum ATTRIBUTES
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

} fat_extBS_t;

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

} fat_BS_t;

typedef struct entry
{
	struct entry* parent;
	unsigned short cluster;
	struct entry* child_entries;
	unsigned char attributes;
	unsigned char is_empty : 1;
	unsigned char is_dir : 1;
	char file_name[11];
	unsigned long file_size;
} entry_t;

static int MAX_ENTRIES;

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
static unsigned char* fat1;
static unsigned char* fat2;

static void parse_bs()
{
	unsigned char* addr = flpydsk_read_write_sector(0, 0, 0);
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

	MAX_ENTRIES = fat_bs.root_entry_count;

	entry_t root_entries[MAX_ENTRIES];
	root_dir = root_entries;

	unsigned char fat_1_arr[fat_size * fat_bs.bytes_per_sector];
	fat1 = fat_1_arr;
	unsigned char fat_2_arr[fat_size * fat_bs.bytes_per_sector];
	fat2 = fat_2_arr;
}

static void read_fs()
{
	char* offset = 0;
	char* ste = 0;
	unsigned char* addr = flpydsk_read_track(1, first_root_dir_sector, 0, 0, 0, ste, offset);
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

			if ((root_dir[i].attributes & DIRECTORY) > 0)
				root_dir[i].is_dir = 1;
			else
				root_dir[i].is_dir = 0;
		}
	}
}

static entry_t* follow_path(char* path)
{
	unsigned char* dir_count = 0;
	char** dirs = split(path, '\\', 256, dir_count);

	if (strcmp(dirs[0], "A") != 0)
		return 0;

	entry_t* current = root_dir;
	for (int i = 1; i < *dir_count; i++)
	{
		int found = 0;
		for (int j = 0; j < MAX_ENTRIES; j++)
		{
			if (current->child_entries[j].is_dir && !current->child_entries[j].is_empty)
			{
				if (strcmp(current->child_entries[j].file_name, dirs[i]) == 0)
				{
					current = &current->child_entries[j];
					found = 1;
					break;
				}
			}
		}
		if (!found)
			return 0;
	}
	return current;
}

static void read_fat()
{
	char sectors_read = 0;
	char* current_sectors = 0;
	char* offset = 0;

	do
	{
		unsigned char* address = flpydsk_read_track(1, first_fat_sector + sectors_read, 0, 0, 0, current_sectors, offset);
		if (sectors_read < fat_size)
		{
			if (*current_sectors > fat_size - sectors_read)
			{
				memcpy(fat1 + sectors_read * fat_bs.bytes_per_sector, address + (*offset * fat_bs.bytes_per_sector), (fat_size - sectors_read) * fat_bs.bytes_per_sector);
				*offset += (fat_size - sectors_read) * fat_bs.bytes_per_sector;
				sectors_read += (fat_size - sectors_read);
				*current_sectors -= (fat_size - sectors_read);
			}
			else
			{
				memcpy(fat1 + sectors_read * fat_bs.bytes_per_sector, address + (*offset * fat_bs.bytes_per_sector), *current_sectors * fat_bs.bytes_per_sector);
				sectors_read += *current_sectors;
				continue;
			}
		}
		else if (sectors_read < fat_size * 2)
		{
			if (*current_sectors > fat_size - (sectors_read - fat_size))
			{
				memcpy(fat2 + (sectors_read - fat_size) * fat_bs.bytes_per_sector, address + (*offset * fat_bs.bytes_per_sector), (fat_size - (sectors_read - fat_size)) * fat_bs.bytes_per_sector);
				*offset += (fat_size - (sectors_read - fat_size)) * fat_bs.bytes_per_sector;
				sectors_read += (fat_size - (sectors_read - fat_size));
				*current_sectors -= (fat_size - (sectors_read - fat_size));
			}
			else
			{
				memcpy(fat2 + (sectors_read - fat_size) * fat_bs.bytes_per_sector, address + (*offset * fat_bs.bytes_per_sector), *current_sectors * fat_bs.bytes_per_sector);
				sectors_read += *current_sectors;
			}
		}
	} while (sectors_read < fat_size * 2);
}


static unsigned short read_cluster(unsigned short cluster)
{
	unsigned int fat_offset = cluster + (cluster / 2);

	unsigned short table_value = *(unsigned short*)&fat1[fat_offset];

	if (cluster & 0x0001)
		table_value = table_value >> 4;		//[xx x0] 00
	else
		table_value = table_value & 0x0FFF;	//00 [0x xx]

	return table_value;
}


static void write_cluster(unsigned short cluster, unsigned short value /* -123 */ )
{
	value &= 0x0FFF;

	unsigned int fat_offset = cluster + (cluster / 2);

	if (cluster & 0x0001)
	{
		unsigned char l_byte = value >> 4; // 12
		unsigned char r_byte = (value << 4) & 0x00FF; // 3-

		fat1[fat_offset] = l_byte;
		fat1[fat_offset + 1] = (fat1[fat_offset + 1] & 0x0F) | r_byte;

		fat2[fat_offset] = l_byte;
		fat2[fat_offset + 1] = (fat2[fat_offset + 1] & 0x0F) | r_byte;
	}
	else
	{
		unsigned char l_byte = value >> 8; // -1
		unsigned char r_byte = value & 0x00FF; // 23

		fat1[fat_offset] = (fat1[fat_offset] & 0xF0) | l_byte;
		fat1[fat_offset + 1] = r_byte;

		fat2[fat_offset] = (fat2[fat_offset] & 0xF0) | l_byte;
		fat2[fat_offset + 1] = r_byte;
	}
}

int write_file(char* path, char* filename, char* data, unsigned long file_size)
{
	if (strlen(filename) > 12)
		return 0;

	unsigned char stramount = 0;
	char** nameparts = split(filename, '.', 11, &stramount);

	if (stramount != 2)
		return 0;

	char name[11];

	for (int i = 0; i < stramount; i++)
	{
		if (!i)
		{
			for (int j = 0; j < 8 && nameparts[i][j] != 0; j++)
				name[j] = nameparts[i][j];
		}
		else
		{
			for (int j = 0; j < 3 && nameparts[i][j] != 0; j++)
				name[j + 8] = nameparts[i][j];
		}
	}
	
	entry_t* dir = follow_path(path);
	if (!dir)
		return 0;

	entry_t* file = 0;

	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (dir->child_entries[i].is_empty || (!dir->child_entries[i].is_dir && strcmp(dir->child_entries[i].file_name, filename) == 0))
		{
			file = &(dir->child_entries[i]);
			break;
		}
	}

	if (!file)
		return 0;

	unsigned short cluster_count = (file_size + fat_bs.bytes_per_sector) / fat_bs.bytes_per_sector;
	unsigned short amount = 0;

	unsigned short clusters[cluster_count];

	if (!file->is_empty)
	{
		amount++;

		char eof_reached = 0;
		unsigned short current = file->cluster;

		unsigned short table_value;
		do
		{
			table_value = read_cluster(current);
			
			if (table_value == 0xFF7)
			{
				eof_reached = 1;
				break;
			}

			clusters[amount++] = current;

			if (table_value >= 0xFF8)
				eof_reached = 1;
			else
				current = table_value;
		} while (amount < cluster_count && !eof_reached);

		if (amount >= cluster_count && !eof_reached) //Free up remaining clusters and set last cluster to 0xFF8
		{
			write_cluster(clusters[amount - 1], 0xFF8);
			do
			{
				table_value = read_cluster(current);

				write_cluster(current, 0);

				if (table_value < 0xFF8)
					current = table_value;

			} while (table_value < 0xFF8);
		}
		else if (amount < cluster_count && eof_reached) //Write to free clusters
		{
			current = 1;
			do
			{
				unsigned short table_value = read_cluster(current);

				if (table_value == 0)
				{
					clusters[amount] = current;

					if (amount > 0)
						write_cluster(clusters[amount - 1], current);

					amount++;
				}

			} while (amount < cluster_count && current++ < total_clusters);

			write_cluster(clusters[amount - 1], 0xFF8);
		}
	}
	else
	{
		unsigned short current = 1;
		do
		{
			unsigned short table_value = read_cluster(current);

			if (table_value == 0)
			{
				clusters[amount] = current;

				if (amount > 0)
					write_cluster(clusters[amount - 1], current);
			
				amount++;
			}

		} while (amount < cluster_count && current++ < total_clusters);

		write_cluster(clusters[amount - 1], 0xFF8);
	}

	file->cluster = clusters[0];
	
	for (int i = 0; i < 11; i++)
		file->file_name[i] = name[i];

	file->attributes = 0;
	file->is_empty = 0;
	file->is_dir = 0;
	file->file_size = file_size;
	file->parent = dir;
	file->child_entries = 0;

	for (unsigned short i = 0; i < cluster_count; i++)
	{
		int sector = first_data_sector + clusters[i];

		memcpy_s((char*)DMA_ADDRESS, &data[i * fat_bs.bytes_per_sector], fat_bs.bytes_per_sector);
		flpydsk_read_write_sector(1, sector, 0);
	}

	return 1;
}

int is_valid_path(char* path)
{
	entry_t* dir = follow_path(path);
	return dir ? 1 : 0;
}

void init_fs()
{
	parse_bs();
	read_fs();
	read_fat();
}