; gdt.asm
bits 32

global gdt_flush/
extern gdt_ptr ; this is defined in C

gdt_flush:
  lgdt [gdt_ptr]  ; load gdt pointer
  mov ax, 0x10    ; 0x10 is the offset of the data segment in gdt
  mov ds, ax      ; set data segment
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  jmp 0x08:.flush 

.flush:
  ret
