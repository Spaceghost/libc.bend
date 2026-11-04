# libc.bend agent agreement

## Branch

Work on `main` (Jack calls it master). Short-lived topic branches are fine;
land them with ordinary direct pushes. No PRs unless asked, never force-push.

## Where to build

Never compile or run tests on the workstation. Every build, smoke, diff, and
dogfood runs inside the Incus container `nelibc-writ` on bak (Debian 12; Zig
0.17.0 at `/opt/zig/zig`, Bend 2.0.39 at `/opt/bend-2.0.39/bend/bin/bend`,
Nelua `3c29419` at `/opt/nelua/nelua` for nelibc). The checkout is
`/work/libc.bend`, beside nelibc's `/work/nelibc`. The repository is private,
so until the container has credentials the tree is shipped with
`git archive` over `ssh bak incus exec nelibc-writ -- su build -c "bash -s"`
rather than pulled. Run as the unprivileged `build` user.

```sh
ssh bak incus exec nelibc-writ -- su build -c 'cd /work/libc.bend && git pull -q && \
  TMPDIR=/work/tmp ZIG_GLOBAL_CACHE_DIR=/work/zig-cache \
  make smoke-matrix ZIG=/opt/zig/zig BEND=/opt/bend-2.0.39/bend/bin/bend NELIBC=/work/nelibc'
```

nelibc's hybrid archive must exist first: `make -C /work/nelibc lib-zig
"NELUA=/opt/nelua/nelua --cache-dir /work/nelua-cache" ZIG=/opt/zig/zig`.
Record the nelibc commit the archive came from with every result.

CI (`.github/workflows/smoke.yml`) runs on the self-hosted bak runner, builds
nelibc at the pinned `NELIBC_REF`, and runs `smoke-matrix`.

## The theseus rule

libc.bend replaces nelibc's pieces, not the host's. A symbol moves into Bend
only when nelibc defines it; the Makefile refuses otherwise. Taking one over
means all of: the def in `src/libc.bend`, the bridge entry in `src/bridge.c`,
the C name in `src/exports.c`, the line in `src/symbols.txt`, and its checks
in `tools/smoke.c` and `tools/ctype_dump.c`. The runtime's references to the
symbol are demoted to `nelibc_<name>` by the build; never call a public libc
name from `src/bridge.c`.

## The gate

Before pushing a behavior change, run and record `make smoke-matrix`
(`smoke-zig`, `diff-zig`, `dogfood-zig`). A check that was not run is "not
run", not "passes".

## Evidence standard

Do not report a build, smoke, or diff result that was not observed on the
exact artifact and profile claimed. Static checks prove only what they run.
When something is unimplemented, say so in the docs rather than writing
around it. Keep secrets out of history.
