
#ifndef UTILS_H
#define UTILS_H

#include "stdio.h"
#include "stdlib.h"
#include "limits.h"
#include "stdarg.h"
#include "string.h"
#include "memory.h"

#include "unordered_map"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef HMM_Vec2 v2;
typedef HMM_Vec3 v3;
typedef HMM_Vec4 v4;
typedef HMM_Mat2 mat2;
typedef HMM_Mat3 mat3;
typedef HMM_Mat4 mat4;
typedef HMM_Quat quat;

#define internal static
#define global static
#define local static

#define PATH_MAX 260

#define MAX(a, b) (a > b) ? (a) : (b)
#define MIN(a, b) (a < b) ? (a) : (b)

#define ARRAY_COUNT(x) (sizeof((x))/sizeof((x)[0]))

#define ASSERT(cond, msg) \
do { \
if (!(cond)) { \
fprintf(stderr, "Assertion failed at line %d: %s\n", __LINE__, msg); \
exit(EXIT_FAILURE); \
} \
} while(0)

#define LOG(format, ...) \
do { \
fprintf(stderr, format, ##__VA_ARGS__); \
fprintf(stderr, "\n"); \
} while(0)

#include "stack.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

internal void
read_file(char *filename, u8 **out_data, u64 *out_size = NULL)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        LOG("Could not open shader source file.");
        *out_data = NULL;
        return;
    }
    
    u64 size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    u8 *data = (u8 *)malloc(size+1);
    fread(data, 1, size, file);
    data[size] = '\0';
    
    fclose(file);
    
    *out_data = data;
    
    if(out_size)
        *out_size = size;
}

#endif //UTILS_H
