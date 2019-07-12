[bits 16]
[org 0x0]					;Setze den allgemeinen Offset auf 0x7C00 (Anfang des Bootsektors).

start:
	jmp boot

TIMES 03h-$+start DB 0

bpbOEM:					DB "OwOS 0.1"
bpbBytesPerSector:  	DW 512
bpbSectorsPerCluster: 	DB 1
bpbReservedSectors: 	DW 1
bpbNumberOfFATs: 	    DB 2
bpbRootEntries: 	    DW 224
bpbTotalSectors: 	    DW 2880
bpbMedia: 	            DB 0xF0
bpbSectorsPerFAT: 	    DW 9
bpbSectorsPerTrack: 	DW 18
bpbHeadsPerCylinder: 	DW 2
bpbHiddenSectors: 	    DD 0
bpbTotalSectorsBig:     DD 0
bsDriveNumber: 	        DB 0
bsUnused: 	            DB 0
bsExtBootSignature: 	DB 0x29
bsSerialNumber:	        DD 0xa0a1a2a3
bsVolumeLabel: 	        DB "MOS FLOPPY "
bsFileSystem: 	        DB "FAT12   "

print:
		lodsb			; load next byte from string from SI to AL
		or	al, al		; Does AL=0?
		jz	printdone	; If yes then return
		mov	ah,	0eh		; Else print char
		int	10h
		jmp	print		; Repeat until null terminator found
printdone:
		ret				; we are done, so return


; CX > Number of sectors to read
; AX > Starting sector
; ES:BX > Buffer to read to
read_sectors:
	.main_read:
		mov     di, 0x0005                          ; five retries for error
	.sectorloop:
		push    ax
		push    bx
		push    cx
		call    LBAtoCHS                            ; convert starting sector to CHS
		mov     ah, 0x02                            ; BIOS read sector
		mov     al, 0x01                            ; read one sector
		mov     ch, BYTE [absoluteTrack]            ; track
		mov     cl, BYTE [absoluteSector]           ; sector
		mov     dh, BYTE [absoluteHead]             ; head
		mov     dl, BYTE [bsDriveNumber]            ; drive
		int     0x13                                ; invoke BIOS
		jnc     .readsuccess                        ; test for read error
		xor     ax, ax                              ; BIOS reset disk
		int     0x13                                ; invoke BIOS
		dec     di                                  ; decrement error counter
		pop     cx
		pop     bx
		pop     ax
		jnz     .sectorloop                         ; attempt to read again
		int     0x18
	.readsuccess:
		mov     si, msgProgress
		call    print
		pop     cx
		pop     bx
		pop     ax
		add     bx, WORD [bpbBytesPerSector]		; queue next buffer
		inc     ax                                  ; queue next sector
		loop    .main_read                          ; read next sector
		ret

CHStoLBA:
		sub     ax, 0x0002                          ; zero base cluster number
		xor     cx, cx
		mov     cl, BYTE [bpbSectorsPerCluster]     ; convert byte to word
		mul     cx
		add     ax, WORD [datasector]               ; base data sector
		ret

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

boot:
		;Set segment registers
		cli						;disable interrupts
		mov     ax, 0x07C0		;setup registers to point to our segment
		mov     ds, ax
		mov     es, ax
		mov     fs, ax
		mov     gs, ax
		
		;Set stack
		mov     ax, 0x0000
		mov     ss, ax
		mov     sp, 0xFFFF
		mov		bp, 0xFFFF
		sti						 ;enable interrupts

		;Loading message
		mov     si, msgLoading
		call    print

read_root:
		xor     cx, cx
		xor     dx, dx
		mov     ax, 0x0020							;32 byte directory entry
		mul     WORD [bpbRootEntries]				;total size of directory
		div     WORD [bpbBytesPerSector]			;sectors used by directory
		xchg    ax, cx
          
		mov     al, BYTE [bpbNumberOfFATs]            ; number of FATs
		mul     WORD [bpbSectorsPerFAT]               ; sectors used by FATs
		add     ax, WORD [bpbReservedSectors]         ; adjust for bootsector
		mov     WORD [datasector], ax                 ; base of root directory
		add     WORD [datasector], cx
          
		mov     bx, 0x0200                            ; copy root dir above bootcode
		call    read_sectors
		
read_dir:
		mov     cx, WORD [bpbRootEntries]				; load loop counter
		mov     di, 0x0200								; locate first root entry
	.dir_loop:
		push    cx
		mov     cx, 0x000B								; eleven character name
		mov     si, Filename							; image name to find
		push    di
		rep  cmpsb										; test for entry match
		pop     di
		je      load_fat
		pop     cx
		add     di, 0x0020								; queue next directory entry
		loop    .dir_loop
		jmp     failure


load_fat:
		;Save starting sector of file
		mov     si, msgNewline
		call    print
		mov     dx, WORD [di + 0x001A]
		mov     WORD [cluster], dx                  ; file's first cluster
          
		;CX > Size of FATs
		xor     ax, ax
		mov     al, BYTE [bpbNumberOfFATs]          ; number of FATs
		mul     WORD [bpbSectorsPerFAT]             ; sectors used by FATs
		mov     cx, ax

		;AX > Location of FATs
		mov     ax, WORD [bpbReservedSectors]       ; adjust for bootsector
          
		;Read FAT
		mov     bx, 0x0200                          ; copy FAT above bootcode
		call    read_sectors

		;Initialize file location
		mov     si, msgNewline
		call    print
		mov     ax, 0x0050
		mov     es, ax
		mov     bx, 0x0000                          ; destination for image
		push    bx

load_file:    
		mov     ax, WORD [cluster]                  ; cluster to read
		pop     bx                                  ; buffer to read into
		call    CHStoLBA	                        ; convert cluster to LBA
		xor     cx, cx
		mov     cl, BYTE [bpbSectorsPerCluster]     ; sectors to read
		call    read_sectors
		push    bx
          
		;Compute next cluster   
		mov     ax, WORD [cluster]                  ; identify current cluster
		mov     cx, ax                              ; copy current cluster
		mov     dx, ax                              ; copy current cluster
		shr     dx, 0x0001                          ; divide by two
		add     cx, dx                              ; sum for (3/2)
		mov     bx, 0x0200                          ; location of FAT in memory
		add     bx, cx                              ; index into FAT
		mov     dx, WORD [bx]                       ; read two bytes from FAT
		test    ax, 0x0001
		jnz     .odd_cluster
          
	.even_cluster: 
		and     dx, 0000111111111111b               ; take low twelve bits
		jmp     .done
         
	.odd_cluster:    
		shr     dx, 0x0004                          ; take high twelve bits
          
	.done: 
		mov     WORD [cluster], dx                  ; store new cluster
		cmp     dx, 0x0FF0                          ; test for end of file
		jb      load_file
          
done:   
		mov     si, msgNewline
		call    print
		mov		si, msgDone
		call	print	
		push    WORD 0x0050
		push    WORD 0x0000
		retf					;Essentially far call to 0x0050:0x0000
          
failure:  
		mov     si, msgFailure
		call    print
		mov     ah, 0x00
		int     0x16                                ; await keypress
		int     0x19                                ; warm boot computer
     
absoluteSector:	db 0x00
absoluteHead:	db 0x00
absoluteTrack:	db 0x00

datasector:		dw 0x0000
cluster:		dw 0x0000
Filename:		db "KRNLDR  SYS"
msgLoading:		db 0x0D, 0x0A, "Loading Boot Image ", 0x0D, 0x0A, 0x00
msgNewline:		db 0x0D, 0x0A, 0x00
msgProgress:	db ".", 0x00
msgDone:		db "Done", 0x0D, 0x0A, 0x00
msgFailure:		db 0x0D, 0x0A, "ERROR : Press Any Key to Reboot", 0x0D, 0x0A, 0x00

times 510 - ($-$$) db 0						; We have to be 512 bytes. Clear the rest of the bytes with 0 
dw 0xAA55