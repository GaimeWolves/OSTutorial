#$@ target
#$< first dependency
#$^all dependencies

CC 	= gcc -ffreestanding -O0 -Wall -Wextra -m32 -fno-pie -nostdlib -g -Isrc/kernel/include -masm=intel -D _DEBUG -D I86
LD 	= ld -m elf_i386
GDB = gdb

KERNEL_C_SRC = $(shell find src/kernel/kernel -name '*.c')
STDLIB_C_SRC = $(shell find src/kernel/stdlib -name '*.c')
HAL_I86_C_SRC = $(shell find src/kernel/hal/i86 -name '*.c')

SRCDIRS = $(shell find . -name '*.c' -exec dirname {} \; | uniq)

KERNEL_OBJ = $(patsubst %.c,build/%.o,$(KERNEL_C_SRC))
STDLIB_OBJ = $(patsubst %.c,build/%.o,$(STDLIB_C_SRC))
HAL_I86_OBJ = $(patsubst %.c,build/%.o,$(HAL_I86_C_SRC))

all: buildrepo bin/boot.bin bin/KRNLDR.SYS bin/KRNL.SYS

run: buildrepo bin/boot.bin
	qemu-system-i386 -boot a -fda bin/os.bin

debug: buildrepo bin/boot.bin
	qemu-system-i386 -s -fda bin/boot.bin -no-reboot &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file bin/kernel.elf"

build/%.o: src/boot/%.asm
	nasm $< -f elf -o $@ -I src/boot/

build/%.bin: src/boot/%.asm
	nasm $< -f bin -o $@ -I src/boot/

bin/boot.bin: build/boot.bin
	cat $^ > $@

bin/KRNLDR.SYS: build/krnldr.bin
	cat $^ > $@

build/kernel.bin: build/src/kernel/entry.o ${KERNEL_OBJ} bin/stdlib.lib bin/hal_i86.lib
	${LD} -e entry -o $@ -Ttext 0x100000 build/src/kernel/entry.o ${KERNEL_OBJ} --oformat binary -Lbin -l:stdlib.lib -l:hal_i86.lib
	${LD} -e entry -o bin/kernel.elf -Ttext 0x100000 build/src/kernel/entry.o ${KERNEL_OBJ} -Lbin -l:stdlib.lib -l:hal_i86.lib

bin/stdlib.lib: ${STDLIB_OBJ}
	ar rcs $@ $^

bin/hal_i86.lib: ${HAL_I86_OBJ}
	ar rcs $@ $^

bin/KRNL.SYS: build/kernel.bin
	cat $^ > $@

build/stage3.bin: src/kernel/stage3.asm
	nasm $< -f bin -o $@ -I src/boot/

build/src/kernel/kernel/%.o: src/kernel/kernel/%.c
	${CC} -c $< -o $@

build/src/kernel/entry.o: src/kernel/entry.c
	${CC} -c $< -o $@

build/src/kernel/stdlib/%.o: src/kernel/stdlib/%.c
	${CC} -c $< -o $@

build/src/kernel/hal/%.o: src/kernel/hal/%.c
	${CC} -c $< -o $@ -Lbin -l:stdlib.lib

buildrepo:
	@$(call make-repo)
	
define make-repo
mkdir bin; \
mkdir build; \
mkdir build/kernel; \
for dir in $(SRCDIRS); \
do \
mkdir -p build/$$dir; \
done
endef

clean:
	rm -rf build/*
