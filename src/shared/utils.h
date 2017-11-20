#ifndef ALS_UTILS
#define ALS_UTILS

typedef enum { false, true } bool;

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