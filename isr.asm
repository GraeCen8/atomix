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

%macro IRQ_STUB 2
  global irq%1
  irq%1:
    cli
    push dword 0
    push dword %2
    jmp irq_common_stub
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1

IRQ_STUB 0, 32
IRQ_STUB 1, 33

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
