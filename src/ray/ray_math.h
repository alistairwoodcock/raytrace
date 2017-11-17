#include <math.h>
#include "ray.h"

struct Vector vec_sub(struct Vector v, struct Vector u){
    struct Vector new_vec = {v.x-u.x, v.y-u.y, v.z-u.z};
    return new_vec;
}

void vec_normalize(struct Vector *v){
    float a = sqrt((v->x*v->x) + (v->y*v->y) + (v->z*v->z));
    v->x = v->x/a;
    v->y = v->y/a;
    v->z = v->z/a;
}

float dot_product(struct Vector v, struct Vector u){
    return (v.x*u.x)+(v.y*u.y)+(v.z*u.z);
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