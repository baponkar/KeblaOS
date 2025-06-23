# Exception Message Code

| s/l | Hex Code | Description                          |
|-----|----------|--------------------------------------|
| 0   | 0x00     | Division By Zero                     |
| 1   | 0x01     | Debug                                |
| 2   | 0x02     | Non Maskable Interrupt               |
| 3   | 0x03     | Breakpoint                           |
| 4   | 0x04     | Into Detected Overflow               |
| 5   | 0x05     | Out of Bounds                        |
| 6   | 0x06     | Invalid Opcode                       |
| 7   | 0x07     | Device Not Available (No Math Coproc)|
| 8   | 0x08     | Double Fault                         |
| 9   | 0x09     | Coprocessor Segment Overrun          |
| 10  | 0x0A     | Invalid TSS                          |
| 11  | 0x0B     | Segment Not Present                  |
| 12  | 0x0C     | Stack-Segment Fault                  |
| 13  | 0x0D     | General Protection Fault             |
| 14  | 0x0E     | Page Fault                           |
| 15  | 0x0F     | Reserved                             |
| 16  | 0x10     | x87 Floating-Point Exception         |
| 17  | 0x11     | Alignment Check                      |
| 18  | 0x12     | Machine Check                        |
| 19  | 0x13     | SIMD Floating-Point Exception        |
| 20  | 0x14     | Virtualization Exception             |
| 21  | 0x15     | Control Protection Exception         |
| 22  | 0x16     | Reserved                             |
| 23  | 0x17     | Reserved                             |
| 24  | 0x18     | Reserved                             |
| 25  | 0x19     | Reserved                             |
| 26  | 0x1A     | Reserved                             |
| 27  | 0x1B     | Reserved                             |
| 28  | 0x1C     | Hypervisor Injection Exception       |
| 29  | 0x1D     | VMM Communication Exception          |
| 30  | 0x1E     | Security Exception                   |
| 31  | 0x1F     | Reserved                             |

# Exception Error Code Table

This table shows which x86 exceptions push an error code on the stack automatically.

| s/l | Hex Code | Description                          | Error Code? |
|-----|----------|--------------------------------------|-------------|
| 0   | 0x00     | Division By Zero                     | No          |
| 1   | 0x01     | Debug                                | No          |
| 2   | 0x02     | Non Maskable Interrupt               | No          |
| 3   | 0x03     | Breakpoint                           | No          |
| 4   | 0x04     | Into Detected Overflow               | No          |
| 5   | 0x05     | Out of Bounds                        | No          |
| 6   | 0x06     | Invalid Opcode                       | No          |
| 7   | 0x07     | Device Not Available                 | No          |
| 8   | 0x08     | Double Fault                         | Yes         |
| 9   | 0x09     | Coprocessor Segment Overrun          | No          |
| 10  | 0x0A     | Invalid TSS                          | Yes         |
| 11  | 0x0B     | Segment Not Present                  | Yes         |
| 12  | 0x0C     | Stack-Segment Fault                  | Yes         |
| 13  | 0x0D     | General Protection Fault             | Yes         |
| 14  | 0x0E     | Page Fault                           | Yes         |
| 15  | 0x0F     | Reserved                             | No          |
| 16  | 0x10     | x87 Floating-Point Exception         | No          |
| 17  | 0x11     | Alignment Check                      | Yes         |
| 18  | 0x12     | Machine Check                        | No          |
| 19  | 0x13     | SIMD Floating-Point Exception        | No          |
| 20  | 0x14     | Virtualization Exception             | No          |
| 21  | 0x15     | Control Protection Exception         | Yes         |
| 22  | 0x16     | Reserved                             | No          |
| 23  | 0x17     | Reserved                             | No          |
| 24  | 0x18     | Reserved                             | No          |
| 25  | 0x19     | Reserved                             | No          |
| 26  | 0x1A     | Reserved                             | No          |
| 27  | 0x1B     | Reserved                             | No          |
| 28  | 0x1C     | Hypervisor Injection Exception       | Yes         |
| 29  | 0x1D     | VMM Communication Exception          | Yes         |
| 30  | 0x1E     | Security Exception                   | Yes         |
| 31  | 0x1F     | Reserved                             | No          |


# Some QEmu Code

| Code    |  Description                                               |
|---------|------------------------------------------------------------|
| 12:     | Internal QEMU line/trace number                            |
| `v=30`        | **Vector** number of the exception (e.g., `0x30 = 48`, likely user-defined interrupt) |
| `e=0000`      | **Error code** pushed by CPU for the exception |
| `i=0`         | `0 = hardware interrupt`, `1 = software interrupt` |
| `cpl=0`       | Current Privilege Level (0 = kernel) |
| `IP=seg:offset` | Instruction pointer where the fault occurred |
| `pc=`         | Linear address of the instruction |
| `SP=seg:addr` | Stack pointer at time of exception |



These are the general-purpose registers. Important ones:

| Register | Use |
|----------|-----|
| `RAX`    | Return value / accumulator |
| `RBX`    | Base register (callee-saved) |
| `RCX`    | Argument / counter |
| `RDX`    | Argument / data |
| `RSP`    | Stack Pointer |
| `RBP`    | Base Pointer (frame) |
| `RIP`    | Instruction Pointer (where crash occurred) |
| `RFL`    | Flags register (e.g., `--S--PC`) |

---

## 🏁 RFLAGS Register


### Flag bits:

| Bit | Flag | Description |
|-----|------|-------------|
| 0   | CF   | Carry Flag |
| 2   | PF   | Parity Flag |
| 6   | ZF   | Zero Flag |
| 7   | SF   | Sign Flag |
| 8   | TF   | Trap Flag (single-step) |
| 9   | IF   | Interrupt Enable |
| 10  | DF   | Direction Flag |
| 11  | OF   | Overflow Flag |

Here, `--S--PC` means:
- `S`: Sign flag set
- `P`: Parity flag set
- `C`: Carry flag set

---

## 🧵 Segment Descriptors


| Field         | Meaning |
|---------------|---------|
| `CS`, `DS`, `SS`, `ES`, `FS`, `GS` | Segment registers |
| `0008`, etc. | Selector index (GDT offset) |
| `Base`       | Base linear address |
| `Limit`      | Segment limit |
| `Access`     | Flags and type |
| `DPL`        | Descriptor Privilege Level |
| `[Flags]`    | Human-readable access rights |
  - `[-R-]`: Readable
  - `[-WA]`: Writable/Accessible
  - `CS64`: 64-bit code segment

---

## 🌍 Special Registers


| Field        | Meaning |
|--------------|---------|
| `TR=0028`    | Task Register (selector of TSS) |
| `Base`       | Base address of TSS |
| `Limit`      | Size of TSS |
| `Flags`      | Access byte |
| `TSS64-avl`  | 64-bit TSS, available |

---

## 💡 Example: Diagnosing a Fault

Say the log contains:



