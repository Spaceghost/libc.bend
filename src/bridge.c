// libc.bend bridge. The Bend compiler splices this file into the emitted
// runtime (src/libc.bend, effect Libc.bridge), so every runtime symbol is in
// scope and FID(name) resolves to the def's function id. It gives each Bend
// def a plain C entry point, libcbend_<name>, over machine words; the public
// libc names and ABI live in src/exports.c, a separate object.
//
// A call takes the bridge lock, lays the arguments into a task node for the
// def, runs the evaluator to its value, and hands the word back. The corpus
// (the runtime's heap: an 8 GiB address-space reservation on the CPU path)
// is mapped on the first call.
//
// The runtime's own references to the symbols libc.bend takes over are
// renamed to nelibc_<name> by the Makefile, so nothing below can re-enter
// the evaluator through a public libc name. Keep this file free of such
// calls all the same.

static pthread_mutex_t lb_lock = PTHREAD_MUTEX_INITIALIZER;
static u64*            lb_H    = NULL;

static Env lb_enter(void) {
  pthread_mutex_lock(&lb_lock);
  if (lb_H == NULL) {
    lb_H   = corpus_setup(false, 1, 0);
    io_stk = pool_stack();
  }
  Env e = { lb_H, ALC[0] };
  return e;
}

static Term lb_leave(Term t) {
  pthread_mutex_unlock(&lb_lock);
  return t;
}

static Term lb_run(Env e, u32 fid, const Term* args, u32 n) {
  u64 loc = task_node(e, fid, TERM_HOLE, 0, 0);
  for (u32 i = 0; i < n; i += 1) {
    e.mem[loc + i] = args[i];
  }
  return corpus_eval(e.mem, term_tsk(fid, loc));
}

// A C byte string as a Bend String whose Chars are the bytes 0..255. The
// runtime's io_str decodes UTF-8 (and folds bad bytes to U+FFFD), which a
// libc must not do; this is io_str's construction without the decoding.
static Term lb_bytes(Env e, const unsigned char* p, u64 n) {
  Term  s;
  Term* hole = &s;
  for (u64 i = 0; i < n; i += 1) {
    u64  l = heap_alloc(e, 1);
    Term t = term_ctr(CID_SCon, l);
    e.mem[l] = (Term)p[i];
    *hole    = hole == &s ? t : io_seal(e, t, CID_SCon);
    hole     = &e.mem[l + 1];
  }
  *hole = term_pak(CID_SNil, 0);
  return s;
}

#define LB_WORD(name, def)                                        \
  uint32_t libcbend_##name(uint32_t a) {                          \
    Env  e    = lb_enter();                                       \
    Term v[1] = { (Term)a };                                      \
    return (uint32_t)lb_leave(lb_run(e, FID(def), v, 1));         \
  }

LB_WORD(isalnum, c_isalnum)
LB_WORD(isalpha, c_isalpha)
LB_WORD(isblank, c_isblank)
LB_WORD(iscntrl, c_iscntrl)
LB_WORD(isdigit, c_isdigit)
LB_WORD(isgraph, c_isgraph)
LB_WORD(islower, c_islower)
LB_WORD(isprint, c_isprint)
LB_WORD(ispunct, c_ispunct)
LB_WORD(isspace, c_isspace)
LB_WORD(isupper, c_isupper)
LB_WORD(isxdigit, c_isxdigit)
LB_WORD(tolower, c_tolower)
LB_WORD(toupper, c_toupper)
LB_WORD(abs, c_abs)

uint32_t libcbend_strlen(const char* s) {
  u64 n = 0;
  while (s[n] != 0) {
    n += 1;
  }
  Env  e    = lb_enter();
  Term v[1] = { lb_bytes(e, (const unsigned char*)s, n) };
  return (uint32_t)lb_leave(lb_run(e, FID(c_strlen), v, 1));
}

// The effect itself is never run; registering it keeps the splice honest.
Term libc_bridge_run(Env e, Term* f, IoWork* w) {
  return term_pak(CID(Unit), 0);
}

static void __attribute__((constructor)) libc_bridge_use(void) {
  io_eff(CID(Libc.bridge), libc_bridge_run);
}
