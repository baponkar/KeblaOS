[![KeblaOS Badge](https://img.shields.io/badge/Kebla-OS-maker?labelColor=red&color=blue)](https://gitlab.com/baponkar/kebla-os)
[![GitHub Badge](https://img.shields.io/badge/Fork-Me-maker?logo=GitHub&logoColor=Blue&labelColor=white&color=blue)
](https://github.com/baponkar/KeblaOS)
[![GitLab Badge](https://img.shields.io/badge/Fork-Me-maker?logo=GitLab&logoColor=Blue&labelColor=white&color=blue)
](https://gitlab.com/baponkar/KeblaOS)
[![Linux Badge](https://img.shields.io/badge/-Linux-maker?logo=linux&logoColor=black&logoSize=auto&labelColor=white&color=blue)
](https://kernel.com)
![C Badge](https://img.shields.io/badge/C-Language-maker?logo=c&logoColor=black&labelColor=white&color=blue)
![x86_32bit Badge](https://img.shields.io/badge/x86-32bit-maker?logo=intel&labelColor=white&color=blue)
![ASM Badge](https://img.shields.io/badge/ASM-Language-maker?logo=assembly&labelColor=white&color=blue)

<h1 style="display: flex;flex-direction: column;color:red;font-size: 3em;align-items: center;justify-content: center;"> Kebla OS </h1>

<p style="display: flex;flex-direction: column;color:skyblue;font-size: 1.5em;align-items: center;justify-content: center;"> 0.7</p>


Build Date : 12/10/24

--------------------------------------------------------

# Features :

* Booting by GRUB2.

* VGA TEXT 80X25 Screen Driver.

* Cleaning Code with my thoughts

* Rewrite VGA Driver with adding more functions like print, putchar, update_cursor etc.

* GDT enabled

* IDT enabled 

* Interrupts Request implemented

* The PIT: A System Clock implemented.

* Keyboard Driver Implemented.

* Fix VGA issue.

* A User Shell implemented

* Implemented Paging

This version Kebla OS will print "Hello, World!" as well as multiple character, decimal and hexadecimal number in VGA Text Screen. This Version is also implemented timer.Keyboard Input is implemented in this version.

# Output Result :
![Output](./KeblaOS_0.7.gif)


# Info :

I have installed cross compiler in `$(HOME)/opt/cross/bin/i686-elf-ld` directory.
To build iso image and run by qemu then use `make`.


# Reference :

1. [Environment Setup](https://web.archive.org/web/20160326062945/http://jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html)

2. [Genesis](https://web.archive.org/web/20160326060959/http://jamesmolloy.co.uk/tutorial_html/2.-Genesis.html)

3. [The Screen](https://web.archive.org/web/20160326064341/http://jamesmolloy.co.uk/tutorial_html/3.-The%20Screen.html)

4. [VGA Hardware](https://wiki.osdev.org/VGA_Hardware)

5. [IRQ](http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial)

6. [The PIT: A System Clock](http://www.osdever.net/tutorials/view/brans-kernel-development-tutorial)

7. [PS2 Keyboard](https://wiki.osdev.org/PS/2_Keyboard)

--------------------------------------------------------

#### Developed by : [baponkar](https://github.com/baponkar)

*© 2024 [KeblaOS](https://github.com/baponkar/KeblaOS) Project. All rights reserved.*


