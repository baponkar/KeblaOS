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
-----------------------------------------------------------------------------------------

# 8085 Processor

![Block diagram of 8085 Processor](./images/8085-microprocessor-architecture-696x562.jpg)


![8085 Pin Configaration](./images/8085-pin-diagram.jpg)


![8085 Instruction Set](./images/8085_instruction_setpage0.png)


[Online 8085 Simulator](https://www.sim8085.com/) .

Example :

1. Addition of two hex numbers (or 8 bit number) and storing the numbers for future use


| Address | Instruction | Hex Code | Comments                                  |
|---------|-------------|----------|-------------------------------------------|
| E000    | MVI A, 49H  | 3E       | Store 49H in A                            |
| E001    |             | 49       |                                           |
| E002    | MOV B, A    | 47       | Copy the contents of A to B               |
| E003    | STA, E050H  | 32       | Store the contents of A in location E050H |
| E004    |             | 50       | location E050H                            |
| E005    |             | E0       |                                           |
| E006    | MVI A, 56H  | 3E       | Store 56H in A                            |
| E007    |             | 56       |                                           |
| E008    | STA, E051H  | 32       | Copy the contents of A                    |
| E009    |             | 51       | in location E051H                         |
| E010    |             | E0       |                                           |
| E011    | ADD B       | 80       | Perform A + B and store the result in A   |
| E012    | RST 1       | CF       | Ends the Program.                         |

## Boolean Algebra

0 + 0 = 0

0 + 1 = 1

1 + 1 = 10

0.0 = 0

1.0 = 0

0.1 = 0

1.1 = 1

(A + B)' = A'.B'

(A.B)' = A' + B'

# Logic Gates and Flip-Flop Input/Output Tables

## OR Gate
| A | B | A OR B |
|---|---|--------|
| 0 | 0 |   0    |
| 0 | 1 |   1    |
| 1 | 0 |   1    |
| 1 | 1 |   1    |

## AND Gate
| A | B | A AND B |
|---|---|---------|
| 0 | 0 |    0    |
| 0 | 1 |    0    |
| 1 | 0 |    0    |
| 1 | 1 |    1    |

## NOT Gate
| A | NOT A |
|---|-------|
| 0 |   1   |
| 1 |   0   |

## Ex-OR (XOR) Gate
| A | B | A ⊕ B |
|---|---|-------|
| 0 | 0 |   0   |
| 0 | 1 |   1   |
| 1 | 0 |   1   |
| 1 | 1 |   0   |

## Ex-NOR (XNOR) Gate
| A | B | A ⊙ B |
|---|---|-------|
| 0 | 0 |   1   |
| 0 | 1 |   0   |
| 1 | 0 |   0   |
| 1 | 1 |   1   |

---

# Flip-Flops

## SR Flip-Flop (Set-Reset)
| S | R | Q (Next State) | Q' (Next State) |
|---|---|----------------|-----------------|
| 0 | 0 |        Q        |       Q'        |
| 0 | 1 |        0        |       1         |
| 1 | 0 |        1        |       0         |
| 1 | 1 |    Undefined    |   Undefined     |

## JK Flip-Flop
| J | K | Q (Next State) | Q' (Next State) |
|---|---|----------------|-----------------|
| 0 | 0 |        Q        |       Q'        |
| 0 | 1 |        0        |       1         |
| 1 | 0 |        1        |       0         |
| 1 | 1 |      Toggle     |     Toggle      |

## D Flip-Flop
| D | Q (Next State) |
|---|----------------|
| 0 |        0       |
| 1 |        1       |

## T Flip-Flop (Toggle)
| T | Q (Next State) |
|---|----------------|
| 0 |        Q       |
| 1 |     Toggle     |







Reference :

1. [geeksforgeeks](https://www.geeksforgeeks.org/architecture-of-8085-microprocessor/)

2. 

-------------------------------------------------------
*© 2025 KeblaOS Project. All rights reserved.*