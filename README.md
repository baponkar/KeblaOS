![Static Badge](https://img.shields.io/badge/Kebla-OS-maker)
[![Static Badge](https://img.shields.io/badge/-maker?logo=github&logoColor=white&labelColor=black&color=black)](https://github.com/baponkar/KeblaOS)



# Kebla OS

Version - 0.0.0.1

![KeblaOS_icon.bmp](./images/KeblaOS_icon.png)



### Introducing the KeblaOS Operating System

Are you passionate about contributing to an innovative and ground-breaking project? Join us in developing an Indigenous Operating System (KeblaOS) crafted entirely in C, celebrating the rich technological heritage of India while paving the way for future advancements.

#### Why KeblaOS?

1. **Made in India**: A project close to home, fostering local talent and innovation.
2. **C Language**: Leveraging the power and efficiency of C, a language known for its performance and control.
3. **Community-Driven**: Collaborate with a dedicated community of developers who share a vision for an open, efficient, and robust operating system.
4. **Open Source**: Contribute to an open-source project where your expertise and creativity can make a real impact.

#### Features and Goals

### Introducing the KeblaOS Operating System

Are you passionate about contributing to an innovative and ground-breaking project? Join us in developing an Indigenous Operating System (KeblaOS) crafted entirely in C, celebrating the rich technological heritage of India while paving the way for future advancements.

#### Why KeblaOS?

1. **Made in India**: A project close to home, fostering local talent and innovation.
2. **C Language**: Leveraging the power and efficiency of C, a language known for its performance and control.
3. **Community-Driven**: Collaborate with a dedicated community of developers who share a vision for an open, efficient, and robust operating system.
4. **Open Source**: Contribute to an open-source project where your expertise and creativity can make a real impact.

#### Features and Goals
- **Simplicity and Efficiency**: Building a streamlined, efficient OS with minimal overhead.
- **Customization and Flexibility**: Providing users and developers with extensive customization options.
- **Security and Stability**: Ensuring a secure and stable environment for various applications.
- **Cultural Integration**: Reflecting the unique aspects of Indian culture and technological aspirations.

#### Get Involved

We invite developers, enthusiasts, and visionaries to join us on this exciting journey. Whether you're an experienced programmer or just starting, your contribution can help shape the future of KeblaOS.

### How to Join

- **Contribute to the Codebase**: Help us build and refine the core components.
- **Share Ideas and Feedback**: Participate in discussions and brainstorm sessions.
- **Spread the Word**: Help us reach a broader audience by sharing our mission.

Together, let's create something remarkable. Join the KeblaOS project and be part of a legacy that blends tradition with innovation.

# Required software for building KeblaOS

* nasm : Compile assembly (asm) code
* gcc  : Compile C, C++, Fortran code
* qemu :
* qemu-kvm :
* libvirt-daemon-system :
* libvirt-clients :
* bridge-utils :
* xx2 : to look inside of a binary file
* grub2 : to build iso with grub2 enabled  bootloader 
* make  : To automate building process

```bash
#Update package
sudo apt update -y

#Upgrade package
sudo apt upgrade -y

#Installing nasm
sudo apt install nasm

#Installing gcc
sudo apt install gcc

#Installing Qemu
sudo apt install qemu qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils

#Start Services
sudo systemctl start libvirtd
sudo systemctl enable libvirtd

#Verify installation
qemu-system-x86_64 --version

#Installing xxd
sudo apt install xxd

#Installing GRUB2
sudo apt-get install grub2

#Instaling make
sudo apt install make

```

Generally Build Directory is containing the iso image of KeblaOS but we can generate the iso by  `Makefile` and using `make -B` command.Autometically the above command will generate and run the iso by Qemu.

[⬇️ Download](https://github.com/baponkar/KeblaOS/releases)
[0.0.0.1]()

Reference : 
1. [Writing a Simple Operating System from Scratch](writing_simple_os.pdf)

2. [wiki.osdev.org](https://wiki.osdev.org)

----------------------------------------------------------------------------------
###### First date of Journey : 9th June, 2024
###### Last Update : 9th June, 2024
###### Developer : [baponkar](https://github.com/baponkar)
----------------------------------------------------------------------------------

© 2024 KeblaOS Project. All rights reserved.


