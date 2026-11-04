// The public C names and ABI of the symbols libc.bend takes over. Each one
// checks the C-side contract (ctype takes an int in -1..255) and calls the
// bridge entry point of the same name, which runs the Bend def.
//
// src/symbols.txt lists exactly the symbols defined here; the Makefile
// checks the two agree.

#include <stddef.h>
#include <stdint.h>

uint32_t libcbend_isalnum(uint32_t c);
uint32_t libcbend_isalpha(uint32_t c);
uint32_t libcbend_isblank(uint32_t c);
uint32_t libcbend_iscntrl(uint32_t c);
uint32_t libcbend_isdigit(uint32_t c);
uint32_t libcbend_isgraph(uint32_t c);
uint32_t libcbend_islower(uint32_t c);
uint32_t libcbend_isprint(uint32_t c);
uint32_t libcbend_ispunct(uint32_t c);
uint32_t libcbend_isspace(uint32_t c);
uint32_t libcbend_isupper(uint32_t c);
uint32_t libcbend_isxdigit(uint32_t c);
uint32_t libcbend_tolower(uint32_t c);
uint32_t libcbend_toupper(uint32_t c);
uint32_t libcbend_abs(uint32_t c);
uint32_t libcbend_strlen(const char* s);

// A character class: 0 for EOF and for values outside unsigned char.
#define LB_CLASS(name) \
  int name(int c) { return (unsigned)c > 255u ? 0 : (int)libcbend_##name((uint32_t)c); }

// A case map: values outside unsigned char come back unchanged, as EOF must.
#define LB_CASE(name) \
  int name(int c) { return (unsigned)c > 255u ? c : (int)libcbend_##name((uint32_t)c); }

LB_CLASS(isalnum)
LB_CLASS(isalpha)
LB_CLASS(isblank)
LB_CLASS(iscntrl)
LB_CLASS(isdigit)
LB_CLASS(isgraph)
LB_CLASS(islower)
LB_CLASS(isprint)
LB_CLASS(ispunct)
LB_CLASS(isspace)
LB_CLASS(isupper)
LB_CLASS(isxdigit)
LB_CASE(tolower)
LB_CASE(toupper)

int abs(int c) {
  return (int)libcbend_abs((uint32_t)c);
}

// The Bend count is a U32: strings of 4 GiB and more are outside this slice.
size_t strlen(const char* s) {
  return (size_t)libcbend_strlen(s);
}
