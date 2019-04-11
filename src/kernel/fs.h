#ifndef FS_H
#define FS_H

void init_fs();
int write_file(char* path, char* filename, char* data, unsigned long file_size);
int is_valid_path(char* path);

#endif