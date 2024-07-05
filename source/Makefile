build_and_boot_Core:
	gcc -masm=intel -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c boot.c -o build/boot.o -Wall
	
	gcc -masm=intel -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c main.c -o build/main.o -Wall
	
	# gcc -masm=intel -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c mm.c -o build/mm.o -Wall

	gcc -masm=intel -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c timer.c -o build/timer.o -Wall
	
	gcc -masm=intel -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c xhci_usb.c -o build/xhci_usb.o -Wall
	
	# gcc -masm=intel -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c nvme.c -o build/nvme.o -Wall
	
	gcc -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c tty_io.c -o build/tty_io.o -Wall
	
	gcc -c printk.c -o build/printk.o -Wall
	
	gcc -c string.c -o build/string.o -Wall
	
	gcc -c fonts.c -o build/fonts.o -Wall
	
	gcc -c test_timer.c -o build/test_timer.o -Wall
	
	gcc -c apic.c -o build/apic.o -Wall
	
	gcc -masm=intel -c msr_io.c -o build/msr_io.o -Wall
	
	gcc -c descriptor.c -o build/descriptor.o -Wall
	
	fasm descriptor_load.asm build/descriptor_load.o
	
	fasm paging.asm build/paging.o
	
	gcc -c isr.c -o build/isr.o -Wall
	
	nasm interrupt.asm -f elf64 -o build/interrupt.o	
	
	ld -shared -Bsymbolic -L /home/dileep/gnu-efi/x86_64/lib/ -L /home/dileep/gnu-efi/x86_64/gnuefi/ -T /home/dileep/gnu-efi/gnuefi/elf_x86_64_efi.lds \
		/home/dileep/gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o build/boot.o build/main.o build/xhci_usb.o build/timer.o build/tty_io.o build/printk.o build/string.o build/fonts.o build/test_timer.o build/apic.o build/msr_io.o build/descriptor.o build/descriptor_load.o build/isr.o build/paging.o build/interrupt.o -o build/system.so -lefi -lgnuefi
	
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 build/system.so build/BOOTx64.EFI
	
	sudo cp build/BOOTx64.EFI /mnt/C96B-D48F/EFI/BOOT/BOOTx64.EFI
	
	# reboot
