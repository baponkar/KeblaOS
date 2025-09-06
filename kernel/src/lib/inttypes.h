// inttypes.h (minimal for LVGL + KeblaOS)
#ifndef _INTTYPES_H
#define _INTTYPES_H

#include <stdint.h>

// Printf macros for 32-bit and 64-bit
#define PRId8  "d"
#define PRIi8  "i"
#define PRIo8  "o"
#define PRIu8  "u"
#define PRIx8  "x"
#define PRIX8  "X"

#define PRId16 "d"
#define PRIi16 "i"
#define PRIo16 "o"
#define PRIu16 "u"
#define PRIx16 "x"
#define PRIX16 "X"

#define PRId32 "d"
#define PRIi32 "i"
#define PRIo32 "o"
#define PRIu32 "u"
#define PRIx32 "x"
#define PRIX32 "X"

#define PRId64 "lld"
#define PRIi64 "lli"
#define PRIo64 "llo"
#define PRIu64 "llu"
#define PRIx64 "llx"
#define PRIX64 "llX"

#endif // _INTTYPES_H
