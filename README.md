<div align="center">

# libc.bend

### A C library, rebuilt piece by piece in [Bend 2](https://github.com/bendlang/bend)

Replace selected libc functions with Bend implementations. Keep the rest on
[nelibc](https://github.com/Spaceghost/nelibc), and beneath nelibc on glibc,
then grow the replacement upward.

[Quickstart](#try-it) ·
[The bridge](docs/bridge.md) ·
[Coverage](docs/coverage.md) ·
[Roadmap](docs/roadmap.md) ·
[Contributing](#contributing)

**Experimental** · **Linux x86-64** · **Hybrid build working** ·
**16 symbols in Bend**

</div>

## What this is

nelibc rebuilds a C library in Nelua, one function at a time, over a host
libc. libc.bend does the same thing one layer up: it rebuilds nelibc in Bend,
one function at a time, over nelibc. A program linked against
`libc-bend-zig.a` gets

| Symbol | Served by |
| --- | --- |
| the 16 in [`src/symbols.txt`](src/symbols.txt) | Bend defs in [`src/libc.bend`](src/libc.bend), run by the Bend 2 evaluator |
| what nelibc implements | nelibc's hybrid archive, absorbed into this one |
| everything else | the host libc (glibc) |

Each symbol Bend takes over is one nelibc owns. nelibc's copy is not
discarded: it is renamed `nelibc_<name>` inside the archive, where the Bend
runtime's own calls still land. That is the only way the evaluator can call
`strlen` without calling itself.

## Try it

Install [Bend 2](https://github.com/bendlang/bend) (2.0.39 is the pinned
compiler) and [Zig](https://ziglang.org) (0.17.0), build nelibc's hybrid
archive beside this checkout, then:

```sh
git clone https://github.com/Spaceghost/nelibc.git
git clone https://github.com/Spaceghost/libc.bend.git
make -C nelibc lib-zig
cd libc.bend
make dogfood-zig
```

This emits the Bend runtime as C, builds the archive, links a C program
against it, runs the program, and checks with `nm` that the Bend symbols and
nelibc's demoted copies both made it into the executable.

To link your own C program:

```sh
make lib-zig
zig cc -I../nelibc/include -fno-builtin app.c build/libc-bend-zig.a -o app
```

`-fno-builtin` directs calls to the library symbols under evaluation;
nelibc's headers declare the ctype functions as functions rather than glibc's
table macros.

## What works today

| Area | In Bend | Beneath |
| --- | --- | --- |
| ctype (12 classes, `toupper`, `tolower`) | C locale, over `int` in -1..255 | nelibc |
| `abs` | two's-complement word | nelibc |
| `strlen` | byte strings, count as a `U32` | nelibc |
| memory, math, stdio, files, processes, threads | not yet | nelibc, then glibc |

Each call crosses into the evaluator under a lock: the arguments are laid
into a task node, the evaluator runs the def to its value, the word comes
back. [The bridge](docs/bridge.md) explains the mechanism and its costs. There
is no shootout yet; a Bend `strlen` is not a fast `strlen`, and the number
has not been measured.

## Build profiles

One profile: `zig` (zig cc on glibc, x86-64). The Bend-emitted C needs clang,
which zig cc provides; GCC does not compile it. musl and the standalone,
freestanding, and host profiles of nelibc are [roadmap](docs/roadmap.md)
items, not builds.

## Checks

| Target | Covers |
| --- | --- |
| `make smoke-zig` | every overridden symbol against the C standard's definition, 20,000 repeated calls, a 100,000-byte string |
| `make diff-zig` | the same calls through glibc, output diffed line for line |
| `make dogfood-zig` | a C program linked against the archive; Bend symbols, `nelibc_strlen`, and nelibc's `memcpy` verified present |
| `make smoke-matrix` | all three |

Every target is run in the Incus container `nelibc-writ` on bak and in CI on
the bak runner; see [`AGENTS.md`](AGENTS.md). Results are reported only for
the artifact they were observed on. Last observed: 2026-11-04, `smoke-matrix`
in `nelibc-writ` (Zig 0.17.0, Bend 2.0.39, glibc 2.36) over nelibc `5dd38c1`:
`smoke: OK`, 273 diff lines identical, dogfood OK.

## Project guide

| Guide | What it covers |
| --- | --- |
| [The bridge](docs/bridge.md) | How a C call reaches a Bend def and comes back |
| [Coverage](docs/coverage.md) | Which symbols are Bend, nelibc, or host |
| [Roadmap](docs/roadmap.md) | What comes next and what is missing |

## Contributing

Pick a symbol nelibc defines, write its Bend def in `src/libc.bend`, give it
a bridge entry in `src/bridge.c` and its C name in `src/exports.c`, add it to
`src/symbols.txt`, and extend `tools/smoke.c` and `tools/ctype_dump.c`. The
Makefile refuses a symbol nelibc does not own and a runtime that still
references one.

## License

MIT.
