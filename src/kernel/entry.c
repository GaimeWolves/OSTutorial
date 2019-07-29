#include <multiboot.h>

extern void __attribute__((__cdecl__)) main(multiboot_info_t* boot_info);

void __attribute__((__cdecl__)) entry()
{
	multiboot_info_t* boot_info = 0;
#ifdef I86
	__asm__("mov %0, ebx" : "=r" (boot_info));
#endif

	main(boot_info);
}