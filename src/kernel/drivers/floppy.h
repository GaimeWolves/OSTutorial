#ifndef FLOPPY_DRIVER
#define FLOPPY_DRIVER

#define DMA_ADDRESS (unsigned char*) 0x21000

unsigned char* flpydsk_read_write_sector(char, int, char);
unsigned char* flpydsk_read_track(char, int, int, int, int, char*, char*);
void flpydsk_lba_to_chs(int, int*, int*, int*);
void init_floppy_driver(void);
void flpydsk_detect_drives();

#endif