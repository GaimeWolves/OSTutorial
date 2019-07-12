extern void __attribute__((__cdecl__)) main();

void __attribute__((__cdecl__)) entry()
{
	__asm__(
		"cli\n\t"
		"mov eax, 0x10\n\t"
		"mov ds, ax\n\t"
		"mov es, ax\n\t"
		"mov fs, ax\n\t"
		"mov gs, ax\n\t"
		"mov ss, ax\n\t"
		"mov esp, 0x90000\n\t"
		"mov ebp, esp\n\t"
		"push ebp\n\t"
	);

	main();

	__asm__
	(
		"cli\n\t"
		"hlt\n\t"
	);
}