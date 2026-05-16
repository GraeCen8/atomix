# Next Milestone: Tiny Kernel Shell

- [ ] Build a line input buffer with backspace/editing support.
- [ ] Add a command parser that splits input by spaces.
- [ ] Implement built-in commands:
  - `help`
  - `mem`
  - `ticks`
  - `clear`
  - `reboot` (or `halt`)
  - `alloc <n>`
- [ ] Replace the periodic-only `kmain` loop with a prompt-driven shell loop.

## Follow-ups

- [ ] Add paging (virtual memory).
- [ ] Add a basic VFS/ramfs.
