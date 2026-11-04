# libc.bend: build rules. Every target is checked in the Incus container
# nelibc-writ on bak (see AGENTS.md); the workstation never runs them.
#
#   make lib-zig        the archive: Bend runtime + exports + nelibc beneath
#   make smoke-zig      self-checking C smoke of every overridden symbol
#   make diff-zig       the same calls through the host libc, diffed
#   make dogfood-zig    a C program linked against the archive, symbols verified
#   make smoke-matrix   all of the above

BEND     ?= bend
ZIG      ?= zig
ZIGFLAGS ?= -mcpu=x86_64
AR       ?= ar
NM       ?= nm
OBJCOPY  ?= objcopy
OUT      ?= build

# The nelibc checkout and its hybrid (zig, glibc) archive. libc.bend sits on
# top of it: every symbol libc.bend takes over is one nelibc defines, and
# nelibc's copy stays reachable as nelibc_<name>.
NELIBC     ?= ../nelibc
NELIBC_LIB ?= $(NELIBC)/build/libnelibc-zig.a

SYMBOLS := $(shell grep -v '^\#' src/symbols.txt)
DEMOTE  := $(foreach s,$(SYMBOLS),--redefine-sym $(s)=nelibc_$(s))

.PHONY: all lib-zig smoke-zig diff-zig dogfood-zig smoke-matrix clean

all: lib-zig

$(OUT):
	mkdir -p $(OUT)

# The Bend compiler emits one C file: the evaluator, the compiled defs, and
# src/bridge.c spliced in by the Libc.bridge effect.
$(OUT)/libc-bend.c: src/libc.bend src/bridge.c | $(OUT)
	BEND_NO_TELEMETRY=1 $(BEND) src/libc.bend -o $@

# The runtime object. Its main is renamed so a consumer supplies its own, and
# every overridden symbol it references is demoted to nelibc's copy, so the
# evaluator can never re-enter itself through a public libc name.
$(OUT)/runtime-zig.o: $(OUT)/libc-bend.c src/symbols.txt
	$(ZIG) cc $(ZIGFLAGS) -w -O2 -Dmain=libcbend_runtime_main -c $(OUT)/libc-bend.c -o $@.raw
	$(OBJCOPY) $(DEMOTE) $@.raw $@
	@bad="$$($(NM) -u $@ | awk '{print $$2}' | grep -Fx -f src/symbols.txt)"; test -z "$$bad" || { echo "runtime-zig.o still references overridden symbols: $$bad"; exit 1; }

# nelibc with the overridden symbols demoted to nelibc_<name>, definitions
# and internal references alike. A symbol nelibc does not define cannot be
# taken over: libc.bend replaces nelibc's pieces, not the host's.
$(OUT)/libnelibc-theseus-zig.a: $(NELIBC_LIB) src/symbols.txt | $(OUT)
	@defined="$$($(NM) --defined-only $(NELIBC_LIB) | awk '{print $$3}')"; for s in $(SYMBOLS); do printf '%s\n' "$$defined" | grep -Fxq "$$s" || { echo "nelibc ($(NELIBC_LIB)) does not define $$s; libc.bend takes over only symbols nelibc owns"; exit 1; }; done
	$(OBJCOPY) $(DEMOTE) $(NELIBC_LIB) $@

$(OUT)/exports-zig.o: src/exports.c src/symbols.txt | $(OUT)
	$(ZIG) cc $(ZIGFLAGS) -fno-builtin -O2 -c src/exports.c -o $@
	@test "$$($(NM) --defined-only $@ | awk '$$2 ~ /^[TW]$$/ {print $$3}' | sort)" = "$$(grep -v '^\#' src/symbols.txt | sort)" || { echo "src/exports.c and src/symbols.txt disagree:"; $(NM) --defined-only $@; exit 1; }

$(OUT)/libc-bend-zig.a: $(OUT)/libnelibc-theseus-zig.a $(OUT)/runtime-zig.o $(OUT)/exports-zig.o
	cp $(OUT)/libnelibc-theseus-zig.a $@
	$(AR) rs $@ $(OUT)/runtime-zig.o $(OUT)/exports-zig.o

lib-zig: $(OUT)/libc-bend-zig.a

# Consumers compile against nelibc's headers (plain declarations, no ctype
# macros) with -fno-builtin so each call reaches the archive.
CONSUMER_FLAGS = -I$(NELIBC)/include -fno-builtin

# Every overridden symbol must be defined in the binary, and nelibc's demoted
# strlen must be there too: the runtime's own calls land on nelibc, not Bend.
define check-symbols
	@defined="$$($(NM) --defined-only $(1) | awk '{print $$3}')"; for s in $(SYMBOLS) nelibc_strlen memcpy; do printf '%s\n' "$$defined" | grep -Fxq "$$s" || { echo "$(1) did not link $$s" >&2; exit 1; }; done
endef

$(OUT)/smoke-zig: tools/smoke.c $(OUT)/libc-bend-zig.a
	$(ZIG) cc $(ZIGFLAGS) $(CONSUMER_FLAGS) -Wl,-u,memcpy tools/smoke.c $(OUT)/libc-bend-zig.a -o $@

smoke-zig: $(OUT)/smoke-zig
	$(OUT)/smoke-zig
	$(call check-symbols,$(OUT)/smoke-zig)
	@echo 'smoke-zig: every overridden symbol answered as the C standard says, through Bend'

$(OUT)/dump-host: tools/ctype_dump.c | $(OUT)
	$(ZIG) cc $(ZIGFLAGS) -fno-builtin tools/ctype_dump.c -o $@

$(OUT)/dump-zig: tools/ctype_dump.c $(OUT)/libc-bend-zig.a
	$(ZIG) cc $(ZIGFLAGS) $(CONSUMER_FLAGS) tools/ctype_dump.c $(OUT)/libc-bend-zig.a -o $@

diff-zig: $(OUT)/dump-host $(OUT)/dump-zig
	$(OUT)/dump-host > $(OUT)/dump-host.txt
	$(OUT)/dump-zig > $(OUT)/dump-zig.txt
	diff -u $(OUT)/dump-host.txt $(OUT)/dump-zig.txt
	$(call check-symbols,$(OUT)/dump-zig)
	@echo "diff-zig: $$(wc -l < $(OUT)/dump-zig.txt) lines identical between the host libc and libc.bend"

$(OUT)/dogfood-zig: examples/incremental.c $(OUT)/libc-bend-zig.a
	$(ZIG) cc $(ZIGFLAGS) $(CONSUMER_FLAGS) -Wl,-u,memcpy examples/incremental.c $(OUT)/libc-bend-zig.a -o $@

dogfood-zig: $(OUT)/dogfood-zig
	$(OUT)/dogfood-zig
	$(call check-symbols,$(OUT)/dogfood-zig)
	@echo 'dogfood-zig: C program linked against libc.bend, Bend symbols and nelibc beneath them verified'

smoke-matrix: smoke-zig diff-zig dogfood-zig

clean:
	rm -rf $(OUT)
