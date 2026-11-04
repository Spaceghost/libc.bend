// A C program on libc.bend: ctype, abs, and strlen come from Bend, memcpy
// from nelibc beneath it, printf from the host libc beneath that.
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  const char* text = "libc.bend: a C library, rebuilt piece by piece in Bend 2";
  char upper[128];
  size_t n = strlen(text);
  memcpy(upper, text, n + 1);
  size_t letters = 0, digits = 0;
  for (size_t i = 0; i < n; i += 1) {
    upper[i] = (char)toupper((unsigned char)upper[i]);
    letters += isalpha((unsigned char)text[i]) != 0;
    digits += isdigit((unsigned char)text[i]) != 0;
  }
  printf("%s\n", upper);
  printf("length %zu, letters %zu, digits %zu, abs(-42) %d\n", n, letters, digits, abs(-42));
  if (n != 56 || letters != 42 || digits != 1 || abs(-42) != 42 || upper[0] != 'L') {
    puts("dogfood: wrong answers");
    return 1;
  }
  puts("dogfood: OK");
  return 0;
}
