```(bash)
	#Run asm file on qemu


	#Creating Binary 
	nasm -f hello.asm -o hello.bin


	#Creating a bootable disk image
	#Creting 2880 sectors of 512 bytes each i.e. 1474560 bytes i.e. 1440KB i.e. 1.40625 MB
	dd if=/dev/zero of=disk.img bs=512 count=2880

	#Copy binary into disk image
	#This command writes the binary to the begining of the disk 
	#image without truncating it.
	dd if=hello.bin of=disk.img conv=notrunc 

	#Run the image on qemu
	qemu-system-i386 -fda disk.img
```
