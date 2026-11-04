// Prints every overridden symbol's answers over its domain so the same
// program built against the host libc and against libc.bend can be diffed.
// Class results are normalized to 0/1: glibc answers with table bits.
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  for (int c = -1; c <= 255; c += 1) {
    printf("%d %d%d%d%d%d%d%d%d%d%d%d%d %d %d\n", c, !!isalnum(c), !!isalpha(c), !!isblank(c),
      !!iscntrl(c), !!isdigit(c), !!isgraph(c), !!islower(c), !!isprint(c), !!ispunct(c),
      !!isspace(c), !!isupper(c), !!isxdigit(c), tolower(c), toupper(c));
  }
  int ints[] = { 0, 1, -1, 7, -7, 1000000, -1000000, INT_MAX, -INT_MAX };
  for (unsigned i = 0; i < sizeof ints / sizeof ints[0]; i += 1) {
    printf("abs %d %d\n", ints[i], abs(ints[i]));
  }
  const char* strs[] = { "", "a", "hello", "hello, world", "\xff\x80\xc3\xa9", "tab\there", "abc\0hidden" };
  for (unsigned i = 0; i < sizeof strs / sizeof strs[0]; i += 1) {
    printf("strlen %u %zu\n", i, strlen(strs[i]));
  }
  return 0;
}
