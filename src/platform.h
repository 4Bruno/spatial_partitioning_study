#ifndef COMMON_H

#include <stdint.h>
#include <stdio.h>

#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))
#define Assert(exp) if (!(exp)) { *(volatile int *)0 = 0; } 

#define internal static

#define Kilobytes(x) x * 1024
#define Megabytes(x) Kilobytes(x) * 1024

#define Log(format, ...) printf(format, __VA_ARGS__)
#define Logn(format, ...) printf(format "\n", __VA_ARGS__)

typedef uint8_t u8;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t i64;

typedef int32_t b32;

typedef float r32;


#define COMMON_H
#endif
