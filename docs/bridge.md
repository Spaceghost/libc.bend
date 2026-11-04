# The bridge

How a C call to `toupper` reaches a Bend def and comes back.

## What the Bend compiler emits

`bend src/libc.bend -o build/libc-bend.c` writes one C file: the Bend 2
runtime (a flat state machine over a single heap, the corpus), every def the
program reaches as a segment of that machine, and a `main` that parses runtime
options and runs the program's `IO` main. A def that is an effect, declared as

```python
def Libc.bridge() -> IO(Unit):
  import "./bridge.c"
```

has its C file spliced into the emitted source after the runtime, with
`FID(name)` and `CID(Name)` replaced by the ids the compiler assigned. That is
the documented extension point for custom effects, and it is the only way to
reach the runtime's internals by name. libc.bend uses it in the opposite
direction from an effect: `src/bridge.c` never runs as an effect, it defines
C functions that call into the evaluator.

## A call

`libcbend_toupper(c)` in `src/bridge.c`:

1. takes the bridge lock, and on the first call maps the corpus
   (`corpus_setup(false, 1, 0)`: CPU path, one lane) and the work stack;
2. allocates a task node for `FID(c_toupper)` with its continuation set to
   the root, and writes the argument word into the node;
3. calls `corpus_eval` on the task term, which runs the state machine until
   the root holds the value;
4. returns the root word and releases the lock.

`toupper` in `src/exports.c` is the public symbol: it applies the C contract
(values outside `-1..255` come back unchanged, as `EOF` must) and calls
`libcbend_toupper`. Keeping the public names out of the spliced file matters:
the runtime calls `strlen` itself, and if the spliced file defined `strlen`
the evaluator would re-enter itself.

## Strings

A Bend `String` is a cons list of `Char`, and the runtime's `io_str` builds
one by decoding UTF-8, folding bad bytes to U+FFFD. A libc counts bytes, so
the bridge has its own `lb_bytes`: the same node construction with each byte
as the Char's code. The Bend `strlen` sees bytes 0..255 and counts them; the
bridge has to find the NUL to know how many bytes to hand over, and the count
the caller receives is the one Bend computed. The result is a `U32`, so
strings of 4 GiB and more are outside this slice.

## The archive

`build/libc-bend-zig.a` holds three things:

- `runtime-zig.o`: the emitted C compiled with `-Dmain=libcbend_runtime_main`,
  so a consumer program's `main` is the only one, and then run through
  `objcopy --redefine-sym` for every symbol in `src/symbols.txt`, so the
  runtime's references to `strlen` become references to `nelibc_strlen`.
  The build fails if any overridden name is still referenced.
- `exports-zig.o`: the public names. The build fails if the set of defined
  symbols differs from `src/symbols.txt`.
- nelibc's hybrid archive with the same `--redefine-sym` applied to every
  member, so nelibc's `strlen` is defined as `nelibc_strlen` and nelibc's
  own internal calls to it follow. The build fails if nelibc does not define
  a symbol on the list: libc.bend replaces nelibc's pieces, not the host's.

The linker then resolves `strlen` to Bend, `nelibc_strlen` to nelibc, and
`memcpy` (not taken over) to nelibc's `memcpy` as before.

## Costs and limits

- One lock around every call; two threads calling `isalpha` serialize.
- The corpus is an 8 GiB address-space reservation (`1 << 33` bytes, mapped
  lazily by the kernel) made on the first call.
- A call allocates a task node in the corpus; `match` frees the nodes it
  consumes, and 20,000 repeated calls in the smoke do not grow the heap in
  a way the smoke can observe, but the bridge does not measure it.
- A def that spawns parallel work would take the runtime's thread pool
  path inside `corpus_eval`; none of the current defs do.
- The runtime's C compiles with clang (zig cc); GCC rejects it.
- Bend 2.0.39 has no stable ABI for these internals (`corpus_setup`,
  `task_node`, `corpus_eval`, `heap_alloc`, `term_ctr`). The bridge tracks
  the pinned compiler and must be revisited on every update.
