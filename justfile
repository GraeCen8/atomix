
# builds the kernel
build:
  nasm -f elf32 kernel.asm -o kasm.o
  gcc -m32 -c kernel.c -o kc.o 
  ld -m elf_i386 -T link.ld -o kernel kasm.o kc.o

# runs the kernel in qemu
run:
  qemu-system-i386 -kernel kernel

# cleans up unused files from building
clean: 
  rm kasm.o
  rm kc.o

# the kernel pipeline
go:
  just build
  just clean
  just run
