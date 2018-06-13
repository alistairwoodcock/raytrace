#ifndef ALS_UTILS
#define ALS_UTILS
#include <stdint.h>

/* COLOUR CODES */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


#define null 0

typedef enum { false, true } bool;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

void swap(float *a, float *b){
    float tmp = *a;
    *a = *b;
    *b = tmp;
}

float to_degrees(float radians) {
    return radians * (180.0 / 3.1415962);
}

float to_radians(float degrees) {
    return degrees / (180.0 / 3.1415962);
}



#endif