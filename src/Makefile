build_kernel:
	x86_64-elf-gcc boot/fb.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/fb.o

	x86_64-elf-gcc boot/boot.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/boot.o

	x86_64-elf-gcc boot/free_stack.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/free_stack.o

	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/gdt.c -o build/kernel/gdt.o

	nasm -f elf64 kernel/load_gdt.asm -o build/kernel/load_gdt.o

	x86_64-elf-gcc -masm=intel -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/idt.c -o build/kernel/idt.o

	nasm kernel/interrupt.asm -f elf64 -o build/kernel/interrupt.o

	x86_64-elf-gcc -masm=intel -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/isr.c -o build/kernel/isr.o

	x86_64-elf-gcc boot/xsdp.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/xsdp.o

	x86_64-elf-gcc boot/mem.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/mem.o
	
	x86_64-elf-gcc boot/main.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/main.o
	
	x86_64-elf-gcc kernel/startup.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/startup.o
	
	x86_64-elf-gcc kernel/mm/memory.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/mm/memory.o

	x86_64-elf-gcc kernel/main.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/main.o
	
	x86_64-elf-gcc kernel/tty_io.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/tty_io.o
	
	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/res/font_8x16.c -o build/kernel/res/font_8x16.o
	
	x86_64-elf-gcc kernel/printk.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/printk.o
	
	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c lib/string.c -o build/lib/string.o
	
	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c lib/checksum.c -o build/lib/checksum.o
	
	x86_64-elf-gcc kernel/acpi.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/acpi.o
	
	x86_64-elf-gcc kernel/pcie.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/pcie.o
	
	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/nvme.c -o build/kernel/nvme.o
	
	x86_64-elf-gcc -masm=intel -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/port_io.c -o build/kernel/port_io.o

	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/timer.c -o build/kernel/timer.o

	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/pic.c -o build/kernel/pic.o

	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/fs/open.c -o build/kernel/fs/open.o

	x86_64-elf-ld build/boot/fb.o build/boot/boot.o build/boot/free_stack.o build/kernel/gdt.o build/kernel/load_gdt.o build/kernel/idt.o build/kernel/interrupt.o build/kernel/isr.o build/boot/xsdp.o build/boot/mem.o build/boot/main.o build/kernel/startup.o build/kernel/mm/memory.o build/kernel/main.o build/kernel/tty_io.o build/kernel/res/font_8x16.o build/lib/string.o build/lib/checksum.o build/kernel/printk.o build/kernel/acpi.o build/kernel/pcie.o build/kernel/nvme.o build/kernel/port_io.o build/kernel/timer.o build/kernel/pic.o build/kernel/fs/open.o /home/dileep/gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o -shared -Bsymbolic -L /home/dileep/gnu-efi/x86_64/lib/ -L /home/dileep/gnu-efi/x86_64/gnuefi/ -T /home/dileep/gnu-efi/gnuefi/elf_x86_64_efi.lds -o build/system.so -lefi -lgnuefi
	
	x86_64-elf-objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 build/system.so build/BOOTx64.EFI

	sudo cp build/BOOTx64.EFI /mnt/C96B-D48F/EFI/BOOT/BOOTx64.EFI
