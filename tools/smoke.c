// Self-checking smoke for every symbol libc.bend takes over. The expected
// values are computed here from the C standard's definitions (C locale), not
// taken from any libc, since the host's copies are not linked.
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

static void expect(const char* what, int c, long got, long want) {
  if (got != want) {
    failures += 1;
    printf("FAIL %s(%d): got %ld, want %ld\n", what, c, got, want);
  }
}

int main(void) {
  for (int c = -1; c <= 255; c += 1) {
    int digit = c >= '0' && c <= '9';
    int upper = c >= 'A' && c <= 'Z';
    int lower = c >= 'a' && c <= 'z';
    int alpha = upper || lower;
    int alnum = alpha || digit;
    int graph = c >= 33 && c <= 126;
    expect("isdigit", c, !!isdigit(c), digit);
    expect("isupper", c, !!isupper(c), upper);
    expect("islower", c, !!islower(c), lower);
    expect("isalpha", c, !!isalpha(c), alpha);
    expect("isalnum", c, !!isalnum(c), alnum);
    expect("isxdigit", c, !!isxdigit(c), digit || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
    expect("isspace", c, !!isspace(c), c == ' ' || (c >= 9 && c <= 13));
    expect("isblank", c, !!isblank(c), c == ' ' || c == 9);
    expect("iscntrl", c, !!iscntrl(c), (c >= 0 && c < 32) || c == 127);
    expect("isprint", c, !!isprint(c), c >= 32 && c <= 126);
    expect("isgraph", c, !!isgraph(c), graph);
    expect("ispunct", c, !!ispunct(c), graph && !alnum);
    expect("toupper", c, toupper(c), lower ? c - 32 : c);
    expect("tolower", c, tolower(c), upper ? c + 32 : c);
  }
  expect("toupper", 256, toupper(256), 256);
  expect("isalpha", 300, isalpha(300), 0);

  expect("abs", 0, abs(0), 0);
  expect("abs", 7, abs(7), 7);
  expect("abs", -7, abs(-7), 7);
  expect("abs", INT_MAX, abs(INT_MAX), INT_MAX);
  expect("abs", -INT_MAX, abs(-INT_MAX), INT_MAX);

  expect("strlen", 0, (long)strlen(""), 0);
  expect("strlen", 1, (long)strlen("a"), 1);
  expect("strlen", 5, (long)strlen("hello"), 5);
  expect("strlen", 4, (long)strlen("\xff\x80\xc3\xa9"), 4);  /* bytes, not code points */
  expect("strlen", 3, (long)strlen("abc\0def"), 3);
  {
    static char big[100001];
    memset(big, 'x', sizeof big - 1);
    expect("strlen", 100000, (long)strlen(big), 100000);
  }
  for (int i = 0; i < 20000; i += 1) {
    if (strlen("twenty thousand calls") != 21 || toupper('q') != 'Q') {
      failures += 1;
      printf("FAIL repeated call %d\n", i);
      break;
    }
  }

  if (failures != 0) {
    printf("smoke: %d failures\n", failures);
    return 1;
  }
  puts("smoke: OK");
  return 0;
}
