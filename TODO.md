
  What to build/fix next (priority order):

  1. Split PIC logic into its own module (pic.c/pic.h) and complete IRQ coverage.
      - Right now PIC remap is embedded in IDT code and only IRQ0/IRQ1 stubs are installed.
      - Files: idt.c, isr.asm, todo.md
  2. Implement remaining IRQ stubs/handlers (IRQ2-IRQ15).
      - You currently only expose timer + keyboard interrupts.
      - Files: isr.asm, idt.c
  3. Harden memory initialization.
      - heap_start is taken from one allocated frame, then a larger range is reserved assuming contiguity; this is fragile long-
        term.
      - Add explicit contiguous-region allocation in PMM or reserve heap range first, then initialize allocator.
      - Files: kernel.c, mem/pmm.c, mem/kmalloc.c
  4. Add allocator safety checks.
      - kfree trusts pointer input and can corrupt heap if given invalid/double-free pointers.
      - Add basic guards (heap bounds, allocation bit/state, optional magic value).
      - File: mem/kmalloc.c
  5. Add basic runtime diagnostics.
      - Print PMM/heap stats during boot and optionally on timer intervals to validate allocator behavior.
      - Files: kernel.c, mem/pmm.h
  6. Improve build ergonomics.
      - README.md is empty; document build/run/clean and toolchain prerequisites.
      - Consider making go not always clean before run (or add separate target).
      - Files: README.md, justfile
