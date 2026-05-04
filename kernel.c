
void clear(void);
void write(char *str);

void kmain(void) {
  clear();
  write("hello world");
}

/* the colors for the forground or backround
0 - Black, 1 - Blue, 2 - Green, 3 - Cyan,
4 - Red, 5 - Magenta, 6 - Brown, 7 - Light Grey,
8 - Dark Grey, 9 - Light Blue, 10/a - Light Green,
11/b - Light Cyan, 12/c - Light Red, 13/d - Light Magenta,
14/e - Light Brown, 15/f – White.
*/

void clear(void) {
  char *vidptr = (char *)0xb8000; // this is where video mem starts
  unsigned int i = 0;
  unsigned int j = 0;

  // clears the screen
  while (j < 80 * 25 * 2) {
    vidptr[j] = ' ';      // white space char
    vidptr[j + 1] = 0x07; // light grey
    j += 2;
  }

  return;
}

void write(char *str) {
  char *vidptr = (char *)0xb8000; // this is where video mem starts
  unsigned int i = 0;
  unsigned int j = 0;

  // writes to video memory
  while (str[j] != '\0') {
    vidptr[i] = str[j];   // the chars ascii
    vidptr[i + 1] = 0x07; // gives the char black bg and light grey fg
    ++j;
    i += 2;
  }

  return;
}
