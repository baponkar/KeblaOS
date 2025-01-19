# Debug with GDB


This file will show the use of gdb to debugging binary kernel file.


Add `-s -S` flag along with Qemu Run i.e.

Open a ubuntu terminal and run
```
qemu-system-x86_64 -cdrom build/KeblaOS-0.11-image.iso  -m 4096 -serial file:debug/serial_output.log -d guest_errors,int,cpu_reset -D debug/qemu.log -vga std -machine ubuntu -s -S
```

Now open another Ubuntu Terminal tab and run

```
gdb build/kernel.bin
```
Now in 2nd terminal tab run `target remote:1234`.

`layout asm` for see asm layout

`break kmain` to enter inside of kmain function

`stepi` to increase step inside of kmain function.

Suppos your kmain is look follow

```
void kmain(){
    fun1();
    fun2();
    fun3();
}
```
You want to look inside of `fun3()` after running `fun1()` and `fun2()` then you need to run below commands

```
break fun3();
run
```

Now see below code to look inside of a specific address
```
(gdb) break *0x1000  # Break at address 0x1000
(gdb) break isr_common_stub  # Break at the ISR common stub
```

```
(gdb) continue
```

```
(gdb) info registers
```

```
(gdb) next # step over function
```

```
(gdb) disassemble main
```

Set up rax with 0x1234 value
```
(gdb) set $rax = 0x1234
```

Setting 0x1000 pointer with value 0xdeadbeef
```
(gdb) set *(int*)0x1000 = 0xdeadbeef
```

To see the 0x1000 address
```
(gdb) x/4x 0x1000
```




## References :

- [olivestem video tutorials](https://www.youtube.com/watch?v=q5vagAJ2YH8&list=PL2EF13wm-hWCoj6tUBGUmrkJmH1972dBB&index=40)

- 