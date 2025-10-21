#ifndef TYPE_H
#define TYPE_H

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

typedef char        ascii;
typedef uint8_t     byte;

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef u32         addr_t;

#define     ON        true
#define     OFF       false

#define KB (1024)
#define MB (1024*KB)

#define BIT(N) (1UL << (N))

#endif // ! TYPE_H
