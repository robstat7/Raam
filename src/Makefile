build_kernel:
	x86_64-elf-gcc boot/fb.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/fb.o

	x86_64-elf-gcc boot/boot.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/boot.o

	x86_64-elf-gcc boot/xsdp.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/xsdp.o

	x86_64-elf-gcc boot/mem.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/mem.o
	
	x86_64-elf-gcc boot/main.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/boot/main.o
	
	x86_64-elf-gcc init/main.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/init/main.o
	
	x86_64-elf-gcc kernel/tty_io.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/tty_io.o
	
	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/res/font_8x16.c -o build/kernel/res/font_8x16.o
	
	x86_64-elf-gcc kernel/printk.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/printk.o
	
	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c lib/string.c -o build/lib/string.o
	
	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c lib/checksum.c -o build/lib/checksum.o
	
	x86_64-elf-gcc kernel/acpi.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/acpi.o
	
	x86_64-elf-gcc kernel/pcie.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I include/ -DEFI_FUNCTION_WRAPPER -o build/kernel/pcie.o
	
	x86_64-elf-gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I include/ -c kernel/nvme.c -o build/kernel/nvme.o

# do not link this file
gcc tools/mkfs_ext2.c -o build/mkfs_ext2 -I include/
	
	x86_64-elf-ld build/boot/fb.o build/boot/boot.o build/boot/xsdp.o build/boot/mem.o build/boot/main.o build/init/main.o build/kernel/tty_io.o build/kernel/res/font_8x16.o build/lib/string.o build/lib/checksum.o build/kernel/printk.o build/kernel/acpi.o build/kernel/pcie.o build/kernel/nvme.o /home/dileep/gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o -shared -Bsymbolic -L /home/dileep/gnu-efi/x86_64/lib/ -L /home/dileep/gnu-efi/x86_64/gnuefi/ -T /home/dileep/gnu-efi/gnuefi/elf_x86_64_efi.lds -o build/system.so -lefi -lgnuefi
	
	x86_64-elf-objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 build/system.so build/BOOTx64.EFI

	sudo cp build/BOOTx64.EFI /mnt/C96B-D48F/EFI/BOOT/BOOTx64.EFI
