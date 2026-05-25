;;kernal.asm
bits 32 ;nasm directive - 32bit
section .multiboot
  ;multiboot spec
  align 4
  dd 0x1BADB002 ;magic
  dd 0x03       ;flags: align modules + request mem_* info
  dd - (0x1BADB002 + 0x03) ; checksum. m+f+c should equal zero.

section .text
global start
extern kmain ;this will be defined in C

start:
  cli ;block interrupts
  mov esp, stack_space ;set stack pointer
  push ebx ;multiboot info pointer
  push eax ;multiboot magic
  call kmain
  add esp, 8
  hlt ;halt the CPU

section .bss
resb 8192   ; 8kb for stack 
stack_space:
