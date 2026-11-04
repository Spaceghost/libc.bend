# CLAUDE.md

Read `AGENTS.md` first; it is binding. Summary:

- Branch: `main` only (master). Direct pushes, no PRs, no force-push.
- Build host: Incus container `nelibc-writ` on bak, never the workstation.
  Checkout there is `/work/libc.bend` beside `/work/nelibc`; push, pull, then
  `make smoke-matrix ZIG=/opt/zig/zig BEND=/opt/bend-2.0.39/bend/bin/bend NELIBC=/work/nelibc`.
- Theseus rule: take over only symbols nelibc defines; every takeover touches
  `src/libc.bend`, `src/bridge.c`, `src/exports.c`, `src/symbols.txt`, and the tools.
- Report only results observed on the named artifact; unrun is unrun.

Project layout: Bend defs in `src/libc.bend`, the spliced runtime bridge in
`src/bridge.c`, public C names in `src/exports.c`, the takeover list in
`src/symbols.txt`, smoke and diff programs in `tools/`, the dogfood program in
`examples/`, build rules in `Makefile`, mechanism and coverage in `docs/`.
