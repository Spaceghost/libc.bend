# Roadmap

| Priority | Item | Why | Status |
| --- | --- | --- | --- |
| P1 | A shootout per overridden symbol against nelibc and glibc, on a named machine | A number nobody has measured is not a cost | not started |
| P1 | `string.h` beyond `strlen`: `strcmp`, `strchr`, `strcpy`, `memcmp` | Byte strings already cross the bridge; comparisons and copies need results that are not words (a pointer, a buffer written in place) | not started |
| P2 | Results and arguments wider than `U32` (`size_t`, pointers) | `strlen` is capped at 4 GiB; a pointer result needs a pair of words or a `Nat` | design |
| P2 | Heap accounting across calls | The smoke's 20,000 repeated calls do not show growth, but nothing measures the corpus | not started |
| P2 | musl profile (`zig cc -target x86_64-linux-musl`) | nelibc has one; the bridge is glibc-only until tried | not run |
| P3 | nelibc's standalone and host profiles beneath the runtime | The runtime needs pthreads, mmap, and stdio from somewhere; nelibc's host profile already runs the unmodified Bend runtime, which is the path to a libc.bend with no host libc at all | design |
| P3 | A GCC build | GCC rejects the emitted C (`preserve_none` calling convention) | blocked on upstream |

Everything above is a direction, not a claim that it is implemented.
