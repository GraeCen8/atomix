;;kernal.asm
bits 32 ;nasm directive - 32bit
section .text
  ;multiboot spec
  align 4
  dd 0x1BADB002 ;magic
  dd 0x00       ;flags
  dd - (0x1BADB002 + 0x00) ; checksum. m+f+c should equal zero.

global start
extern kmain ;this will be defined in C

start:
  cli ;block interrupts
  mov esp, stack_space ;set stack pointer
  call kmain
  hlt ;halt the CPU

section .bss
resb 8192   ; 8kb for stack 
stack_space:
