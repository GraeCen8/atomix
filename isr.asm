bits 32

extern isr_handler
extern irq_handler

global idt_flush

%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    cli
    push dword 0
    push dword %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    cli
    push dword %1
    jmp isr_common_stub
%endmacro

%macro IRQ_STUB 2
  global irq%1
  irq%1:
    cli
    push dword 0
    push dword %2
    jmp irq_common_stub
%endmacro

; ISR0: Divide Error (#DE) - CPU detected divide-by-zero or quotient overflow.
ISR_NOERRCODE 0
; ISR1: Debug Exception (#DB) - CPU debug trap/fault.
ISR_NOERRCODE 1
; ISR2: Non-Maskable Interrupt (NMI) - external hardware NMI pin event.
ISR_NOERRCODE 2
; ISR3: Breakpoint (#BP) - software INT3 breakpoint trap.
ISR_NOERRCODE 3
; ISR4: Overflow (#OF) - software INTO instruction trap.
ISR_NOERRCODE 4
; ISR5: BOUND Range Exceeded (#BR) - BOUND instruction range check fault.
ISR_NOERRCODE 5
; ISR6: Invalid Opcode (#UD) - CPU cannot decode/execute instruction.
ISR_NOERRCODE 6
; ISR7: Device Not Available (#NM) - FPU/SIMD used while unavailable.
ISR_NOERRCODE 7
; ISR8: Double Fault (#DF) - exception while servicing another exception.
ISR_ERRCODE 8
; ISR9: Coprocessor Segment Overrun - legacy x87 overrun condition.
ISR_NOERRCODE 9
; ISR10: Invalid TSS (#TS) - task-state segment fault.
ISR_ERRCODE 10
; ISR11: Segment Not Present (#NP) - referenced segment is not present.
ISR_ERRCODE 11
; ISR12: Stack-Segment Fault (#SS) - stack segment load/access fault.
ISR_ERRCODE 12
; ISR13: General Protection Fault (#GP) - protection violation.
ISR_ERRCODE 13
; ISR14: Page Fault (#PF) - paging translation/protection violation.
ISR_ERRCODE 14
; ISR15: Reserved - Intel reserved vector.
ISR_NOERRCODE 15
; ISR16: x87 Floating-Point Exception (#MF) - x87 FPU error.
ISR_NOERRCODE 16
; ISR17: Alignment Check (#AC) - unaligned memory access at CPL3.
ISR_ERRCODE 17
; ISR18: Machine Check (#MC) - hardware-reported CPU error.
ISR_NOERRCODE 18
; ISR19: SIMD Floating-Point Exception (#XM/#XF) - SSE floating-point error.
ISR_NOERRCODE 19
; ISR20: Virtualization Exception (#VE) - EPT/virtualization-related fault.
ISR_NOERRCODE 20
; ISR21: Control Protection Exception (#CP) - CET/control-flow protection fault.
ISR_ERRCODE 21
; ISR22: Reserved - Intel reserved vector.
ISR_NOERRCODE 22
; ISR23: Reserved - Intel reserved vector.
ISR_NOERRCODE 23
; ISR24: Reserved - Intel reserved vector.
ISR_NOERRCODE 24
; ISR25: Reserved - Intel reserved vector.
ISR_NOERRCODE 25
; ISR26: Reserved - Intel reserved vector.
ISR_NOERRCODE 26
; ISR27: Reserved - Intel reserved vector.
ISR_NOERRCODE 27
; ISR28: Hypervisor Injection Exception (#HV) - hypervisor injection event.
ISR_NOERRCODE 28
; ISR29: VMM Communication Exception (#VC) - guest/hypervisor communication fault.
ISR_NOERRCODE 29
; ISR30: Security Exception (#SX) - security-sensitive fault.
ISR_ERRCODE 30
; ISR31: Reserved - Intel reserved vector.
ISR_NOERRCODE 31


; IRQ0 -> INT32: Programmable Interval Timer tick interrupt.
IRQ_STUB 0, 32
; IRQ1 -> INT33: PS/2 keyboard controller interrupt (key events).
IRQ_STUB 1, 33
; IRQ2 -> INT34: PIC cascade line between master and slave PIC.
IRQ_STUB 2, 34
; IRQ3 -> INT35: COM2/COM4 serial ports (legacy).
IRQ_STUB 3, 35
; IRQ4 -> INT36: COM1/COM3 serial ports (legacy).
IRQ_STUB 4, 36
; IRQ5 -> INT37: LPT2 or sound card (legacy/board-specific).
IRQ_STUB 5, 37
; IRQ6 -> INT38: Floppy disk controller (legacy).
IRQ_STUB 6, 38
; IRQ7 -> INT39: LPT1 or spurious master PIC IRQ.
IRQ_STUB 7, 39
; IRQ8 -> INT40: Real-time clock (RTC).
IRQ_STUB 8, 40
; IRQ9 -> INT41: ACPI / redirected IRQ2 (legacy).
IRQ_STUB 9, 41
; IRQ10 -> INT42: General-purpose peripheral interrupt.
IRQ_STUB 10, 42
; IRQ11 -> INT43: General-purpose peripheral interrupt.
IRQ_STUB 11, 43
; IRQ12 -> INT44: PS/2 mouse interrupt.
IRQ_STUB 12, 44
; IRQ13 -> INT45: x87 FPU interrupt (legacy).
IRQ_STUB 13, 45
; IRQ14 -> INT46: Primary ATA channel interrupt.
IRQ_STUB 14, 46
; IRQ15 -> INT47: Secondary ATA channel interrupt.
IRQ_STUB 15, 47

isr_common_stub:
  pusha
  push ds
  push es
  push fs
  push gs
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  push esp
  call isr_handler
  add esp, 4
  pop gs
  pop fs
  pop es
  pop ds
  popa
  add esp, 8
  sti
  iret

irq_common_stub:
  pusha
  push ds
  push es
  push fs
  push gs
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  push esp
  call irq_handler
  add esp, 4
  pop gs
  pop fs
  pop es
  pop ds
  popa
  add esp, 8
  sti
  iret

idt_flush:
  mov eax, [esp + 4]
  lidt [eax]
  ret
