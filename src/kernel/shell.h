#ifndef SHELL
#define SHELL

#define BUFFER_SIZE 256

void shell_type_char(char c);
void shell_delete_key(unsigned char index);
void shell_execute();
void init_shell();
void shell_function_key(unsigned char index);
void shell_directional_key(unsigned char index);

#endif
