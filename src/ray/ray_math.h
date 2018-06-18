#ifndef RAY_MATH
#define RAY_MATH

#include <math.h>
#include "ray.h"

float rand2(){
    return (float)rand() / (float)RAND_MAX;
}

float clamp(float low, float high, float val){
    if(val < low) return low;
    if(val > high) return high;
    return val;
}

struct Vector pos_to_vec(struct Position p){
    struct Vector v = {p.x,p.y,p.z};
    return v;
}

struct Vector color_to_vec(struct Color c){
    struct Vector v = {c.r,c.g,c.b};
    return v;
}

struct Position vec_to_pos(struct Vector v){
    struct Position p = {v.x,v.y,v.z};
    return p;
}

struct Color vec_to_color(struct Vector v){
    struct Color c = {v.x,v.y,v.z};
    return c;   
}


struct Vector vec_sub(struct Vector v, struct Vector u){
    struct Vector new_vec = {v.x-u.x, v.y-u.y, v.z-u.z};
    return new_vec;
}

struct Vector vec_mult_scalar(struct Vector v, float s){
    struct Vector new_vec = {v.x*s, v.y*s, v.z*s};
    return new_vec;
}

struct Vector vec_mult_vec(struct Vector v1, struct Vector v2){
    struct Vector new_vec = {v1.x * v2.x, v1.y*v2.y, v1.z*v2.z};
    return new_vec;
}

struct Vector vec_add(struct Vector v1, struct Vector v2){
    struct Vector new_vec = {v1.x+v2.x, v1.y+v2.y, v1.z+v2.z};
    return new_vec;
}


void vec_normalize(struct Vector *v){
    float a = sqrt((v->x*v->x) + (v->y*v->y) + (v->z*v->z));
    v->x = v->x/a;
    v->y = v->y/a;
    v->z = v->z/a;
}

float vec_dot(struct Vector v, struct Vector u){
    return (v.x*u.x)+(v.y*u.y)+(v.z*u.z);
}

float mix(float a, float b, float mix){
    return b * mix + a * (1 - mix); 
} 

bool solve_quadratic(float a, float b, float c, float *x0, float *x1){
    float discr = b * b - 4 * a * c; 
    if (discr < 0) return false; 
    else if (discr == 0) *x0 = *x1 = - 0.5 * b / a; 
    else { 
        float q = (b > 0) ? 
            -0.5 * (b + sqrt(discr)) : 
            -0.5 * (b - sqrt(discr)); 
        *x0 = q / a; 
        *x1 = c / q; 
    } 
    if (*x0 > *x1) swap(x0, x1); 
 
    return true; 
} 

#endif