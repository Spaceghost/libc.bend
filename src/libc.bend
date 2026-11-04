import Base

# libc.bend: C library functions written in Bend 2.
#
# Every def here takes and returns machine words (U32) or byte strings (a
# String whose Chars are the bytes 0..255). src/bridge.c, spliced into the
# emitted runtime by the Libc.bridge effect below, carries the arguments in
# and the results out; src/exports.c gives each one its C name and ABI.
# C locale only.

# ctype
# -----

def in_range(+c: U32, lo: U32, hi: U32) -> Bool:
  U32.is_ge(c, lo) && U32.is_le(c, hi)

def is_digit(+c: U32) -> Bool:
  in_range(c, 48, 57)

def is_upper(+c: U32) -> Bool:
  in_range(c, 65, 90)

def is_lower(+c: U32) -> Bool:
  in_range(c, 97, 122)

def is_alpha(+c: U32) -> Bool:
  is_upper(c) || is_lower(c)

def is_alnum(+c: U32) -> Bool:
  is_alpha(c) || is_digit(c)

def is_graph(+c: U32) -> Bool:
  in_range(c, 33, 126)

def c_isdigit(+c: U32) -> U32:
  Bool.to_u32(is_digit(c))

def c_isupper(+c: U32) -> U32:
  Bool.to_u32(is_upper(c))

def c_islower(+c: U32) -> U32:
  Bool.to_u32(is_lower(c))

def c_isalpha(+c: U32) -> U32:
  Bool.to_u32(is_alpha(c))

def c_isalnum(+c: U32) -> U32:
  Bool.to_u32(is_alnum(c))

def c_isxdigit(+c: U32) -> U32:
  Bool.to_u32(is_digit(c) || in_range(c, 65, 70) || in_range(c, 97, 102))

def c_isspace(+c: U32) -> U32:
  Bool.to_u32(U32.is_eq(c, 32) || in_range(c, 9, 13))

def c_isblank(+c: U32) -> U32:
  Bool.to_u32(U32.is_eq(c, 32) || U32.is_eq(c, 9))

def c_iscntrl(+c: U32) -> U32:
  Bool.to_u32(U32.is_lt(c, 32) || U32.is_eq(c, 127))

def c_isprint(+c: U32) -> U32:
  Bool.to_u32(in_range(c, 32, 126))

def c_isgraph(+c: U32) -> U32:
  Bool.to_u32(is_graph(c))

def c_ispunct(+c: U32) -> U32:
  Bool.to_u32(is_graph(c) && Bool.not(is_alnum(c)))

def c_toupper(+c: U32) -> U32:
  U32.sub(c, U32.mul(32, Bool.to_u32(is_lower(c))))

def c_tolower(+c: U32) -> U32:
  U32.add(c, U32.mul(32, Bool.to_u32(is_upper(c))))

# stdlib
# ------

# abs over the two's-complement word: with s the sign bit and m its mask,
# (c xor m) + s. abs(INT_MIN) wraps to INT_MIN, as C leaves it undefined.
def c_abs(+c: U32) -> U32:
  +s = U32.shrn(c, 31n)
  U32.add(U32.xor(c, U32.sub(0, s)), s)

# string
# ------

def strlen_go(s: String, acc: U32) -> U32:
  match s:
    case SNil{}:
      acc
    case SCon{h, t}:
      strlen_go(t, U32.inc(acc))

# The bridge hands over the bytes before the NUL; the count is Bend's.
def c_strlen(s: String) -> U32:
  strlen_go(s, 0)

# bridge
# ------

# The effect exists to splice src/bridge.c into the emitted C; it is never
# run. main refers to it so the program has an IO entry; the runtime's own
# main is renamed at build time and a consumer program supplies its own.
def Libc.bridge() -> IO(Unit):
  import "./bridge.c"

def main() -> IO(Unit):
  Libc.bridge()
