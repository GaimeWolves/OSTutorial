%ifndef FLOPPY16
%define FLOPPY16

[bits 16]

bpbOEM:					DB "OwOS 0.1"
bpbBytesPerSector:  	DW 512
bpbSectorsPerCluster: 	DB 1
bpbReservedSectors: 	DW 1
bpbNumberOfFATs: 		DB 2
bpbRootEntries: 		DW 224
bpbTotalSectors: 		DW 2880
bpbMedia: 				DB 0xf0  ;; 0xF1
bpbSectorsPerFAT: 		DW 9
bpbSectorsPerTrack: 	DW 18
bpbHeadsPerCylinder: 	DW 2
bpbHiddenSectors: 		DD 0
bpbTotalSectorsBig:     DD 0
bsDriveNumber: 	        DB 0
bsUnused: 				DB 0
bsExtBootSignature: 	DB 0x29
bsSerialNumber:	        DD 0xa0a1a2a3
bsVolumeLabel: 	        DB "MOS FLOPPY "
bsFileSystem: 	        DB "FAT12   "

datasector  dw 0x0000
cluster     dw 0x0000

absoluteSector db 0x00
absoluteHead   db 0x00
absoluteTrack  db 0x00

;AX->Cluster
ClusterLBA:
          sub     ax, 0x0002                          ; zero base cluster number
          xor     cx, cx
          mov     cl, BYTE [bpbSectorsPerCluster]     ; convert byte to word
          mul     cx
          add     ax, WORD [datasector]               ; base data sector
          ret

;AX->LBA Address to convert
LBAtoCHS:
          xor     dx, dx                              ; prepare dx:ax for operation
          div     WORD [bpbSectorsPerTrack]           ; calculate
          inc     dl                                  ; adjust for sector 0
          mov     BYTE [absoluteSector], dl
          xor     dx, dx                              ; prepare dx:ax for operation
          div     WORD [bpbHeadsPerCylinder]          ; calculate
          mov     BYTE [absoluteHead], dl
          mov     BYTE [absoluteTrack], al
          ret

;CX->Number of sectors to read
;AX->Starting sector
;ES:EBX->Buffer to read to
read_sectors:
	.read_sectors_start:
		mov     di, 0x0005                          ; five retries for error
	.read_sectors_loop:
		push    ax
		push    bx
		push    cx
		call    LBAtoCHS                            ; convert starting sector to CHS
		mov     ah, 0x02                            ; BIOS read sector
		mov     al, 0x01                            ; read one sector
		mov     ch, byte [absoluteTrack]            ; track
		mov     cl, byte [absoluteSector]           ; sector
		mov     dh, byte [absoluteHead]             ; head
		mov     dl, byte [bsDriveNumber]            ; drive
		int     0x13                                ; invoke BIOS
		jnc     .read_sectors_done                  ; test for read error
		xor     ax, ax                              ; BIOS reset disk
		int     0x13                                ; invoke BIOS
		dec     di                                  ; decrement error counter
		pop     cx
		pop     bx
		pop     ax
		jnz     .read_sectors_loop                  ; attempt to read again
		int     0x18
	.read_sectors_done:
		pop     cx
		pop     bx
		pop     ax
		add     bx, word [bpbBytesPerSector]        ; queue next buffer
		inc     ax                                  ; queue next sector
		loop    .read_sectors_start                 ; read next sector
		ret

%endif ;FLOPPY16
