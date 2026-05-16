# Atomix

Minimal 32-bit x86 kernel with:
- GDT and IDT setup
- ISR/IRQ dispatch
- PIC remapping
- PIT timer
- PS/2 keyboard input
- Physical frame allocator (PMM)
- Simple heap allocator (`kmalloc`/`kfree`)

## Prerequisites

- `gcc` with 32-bit support
- `ld` (binutils)
- `nasm`
- `qemu-system-i386`
- `just`

On Debian/Ubuntu-like systems, these are typically available via:
`build-essential`, `gcc-multilib`, `binutils`, `nasm`, `qemu-system-x86`, `just`.

## Commands

- `just build`: assemble, compile, and link `kernel`
- `just run`: boot the kernel in QEMU
- `just go`: build then run (no forced clean)
- `just go-clean`: clean, rebuild, and run
- `just clean`: remove generated object files

## Notes

- Build outputs object files in the repo root with source-path-flattened names.
- The linked kernel artifact is `./kernel`.
