%ifndef MULTIBOOT_INFO
%define MULTIBOOT_INFO

struc multiboot_info
	.flags 			resd 1
	.memory_lo 		resd 1
	.memory_hi 		resd 1
	.boot_device 	resd 1
	.cmd_ln 		resd 1
	.mods_cnt 		resd 1
	.mods_addr 		resd 1
	.syms0 			resd 1
	.syms1 			resd 1
	.syms2 			resd 1
	.mmap_len 		resd 1
	.mmap_addr 		resd 1
	.drives_len 	resd 1
	.drives_addr 	resd 1
	.conf_table 	resd 1
	.boot_name 		resd 1
	.apm_table 		resd 1
	.vbe_ctrl_info 	resd 1
	.vbe_mode_info 	resw 1
	.vbe_io_seg 	resw 1
	.vbe_io_off 	resw 1
	.vbe_io_len 	resw 1
endstruc


%endif