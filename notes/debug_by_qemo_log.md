# Debug By QEmu Log

Here an example of QEmu Log:

```bash
   120: v=0e e=0002 i=0 cpl=0 IP=0008:ffffffff80015a40 pc=ffffffff80015a40 SP=0010:0000000000001000 CR2=0000000000000ff8
RAX=0000000000000001 RBX=0000000000000000 RCX=0000000000401011 RDX=0000000000401000
RSI=0000000000000000 RDI=0000000000401020 RBP=ffff80007ff61fc0 RSP=0000000000001000
R8 =0000000000000001 R9 =0000000100c10000 R10=0000000000000000 R11=0000000000000246
R12=0000000000000000 R13=0000000000000000 R14=0000000000000000 R15=0000000000000000
RIP=ffffffff80015a40 RFL=00000002 [-------] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 0000000000000000 0fffffff 00a0f300 DPL=3 DS   [-WA]
CS =0008 0000000000000000 ffffffff 00a09b00 DPL=0 CS64 [-RA]
SS =0010 0000000000000000 ffffffff 00c09300 DPL=0 DS   [-WA]
DS =0023 0000000000000000 0fffffff 00a0f300 DPL=3 DS   [-WA]
FS =0023 0000000000000000 0fffffff 00a0f300 DPL=3 DS   [-WA]
GS =0023 0000000000000000 0fffffff 00a0f300 DPL=3 DS   [-WA]
LDT=0000 0000000000000000 00000000 00008200 DPL=0 LDT
TR =0028 ffffffff8059dac0 00000068 00008900 DPL=0 TSS64-avl
GDT=     ffffffff8059da60 00000037
IDT=     ffffffff8069ec40 00000fff
CR0=80010011 CR2=0000000000000ff8 CR3=000000007ff51000 CR4=00000020
DR0=0000000000000000 DR1=0000000000000000 DR2=0000000000000000 DR3=0000000000000000 
DR6=00000000ffff0ff0 DR7=0000000000000400
CCS=0000000000000000 CCD=0000000000000292 CCO=EFLAGS
EFER=0000000000000d01
check_exception old: 0xe new 0xe
```

Explanation:
```
120: v=0e e=0002 i=0 cpl=0 IP=0008:ffffffff80015a40 pc=ffffffff80015a40 SP=0010:0000000000001000 CR2=0000000000000ff8
```

120 is log number
Here exception v=0xe(decimal 14) Page Fault Exception is occared

cpl=0 means kernel mode
Instraction Pointer where problem occared : 0xffffffff80015a40

e=0002 , error flag = 0002

Stack Pointer address: 0000000000001000 

Page Fault occared at address: CR2=0000000000000ff8

PML4 Directory Address: CR3=000000007ff51000

## Inspect elf or binary file with instruction address:

```
objdump -d -M intel build/kernel.bin | grep -A20 ffffffff8000683e
```

## Output:
```
objdump: Warning: Bogus end-of-siblings marker detected at offset 15a93 in .debug_info section
objdump: Warning: Bogus end-of-siblings marker detected at offset 15a94 in .debug_info section
objdump: Warning: Bogus end-of-siblings marker detected at offset 15a95 in .debug_info section
objdump: Warning: Further warnings about bogus end-of-sibling markers suppressed
ffffffff8000683e:       0f b6 00                movzx  eax,BYTE PTR [rax]
ffffffff80006841:       84 c0                   test   al,al
ffffffff80006843:       75 d5                   jne    ffffffff8000681a <print+0xe>
ffffffff80006845:       90                      nop
ffffffff80006846:       90                      nop
ffffffff80006847:       c9                      leave
ffffffff80006848:       c3                      ret

ffffffff80006849 <get_cursor_pos_x>:
ffffffff80006849:       55                      push   rbp
ffffffff8000684a:       48 89 e5                mov    rbp,rsp
ffffffff8000684d:       8b 05 b5 94 69 00       mov    eax,DWORD PTR [rip+0x6994b5]        # ffffffff8069fd08 <cur_x>
ffffffff80006853:       48 98                   cdqe
ffffffff80006855:       5d                      pop    rbp
ffffffff80006856:       c3                      ret

ffffffff80006857 <get_cursor_pos_y>:
ffffffff80006857:       55                      push   rbp
ffffffff80006858:       48 89 e5                mov    rbp,rsp
ffffffff8000685b:       8b 05 ab 94 69 00       mov    eax,DWORD PTR [rip+0x6994ab]        # ffffffff8069fd0c <cur_y>
ffffffff80006861:       48 98                   cdqe
```

Checking what is causing Fault by GDB
`kernel.bin` should build by `-g` flag.

```
gdb kernel.bin
info line *0xffffffff8000683e
```

## Output:
```
Line 229 of "kernel/src/driver/vga/vga_term.c" starts at address 0xffffffff8000683a <print+46>
   and ends at 0xffffffff80006845 <print+57>.
```

