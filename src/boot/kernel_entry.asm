;Callt die main() Funktion des Kernels.
[bits 32]
[extern main]

global _start

_start:
	call main
	jmp $		;Hänge für immer falls der Kernel returned.
