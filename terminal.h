#ifndef TERMINAL_H
#define TERMINAL_H

void terminal_initialize();
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_write(const char *data);

#endif
