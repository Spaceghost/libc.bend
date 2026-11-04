# Coverage

Which layer serves each symbol in a program linked against
`build/libc-bend-zig.a`. The Bend list is `src/symbols.txt`; the Makefile
checks that `src/exports.c` defines exactly those names and that nelibc
defines every one of them.

## In Bend (16)

| Header | Symbols | Notes |
| --- | --- | --- |
| `ctype.h` | `isalnum` `isalpha` `isblank` `iscntrl` `isdigit` `isgraph` `islower` `isprint` `ispunct` `isspace` `isupper` `isxdigit` | C locale; `int` in -1..255, 0 outside |
| `ctype.h` | `tolower` `toupper` | values outside -1..255 come back unchanged |
| `stdlib.h` | `abs` | `abs(INT_MIN)` wraps, as C leaves it undefined |
| `string.h` | `strlen` | bytes; result is a `U32` |

Each has a demoted twin `nelibc_<name>` in the archive, which only the Bend
runtime's own code reaches.

## From nelibc

Everything nelibc's hybrid (zig, glibc) archive defines and the list above
does not: see nelibc's
[API inventory](https://github.com/Spaceghost/nelibc/blob/main/docs/musl-1.2.5-api-coverage.csv).
`memcpy` is verified present in every linked check.

## From the host

What nelibc delegates to glibc in its hybrid profile. `printf` in the
dogfood program is one.
