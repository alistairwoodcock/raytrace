#ifndef _SPHERE
#define _SPHERE

struct Sphere{
    float radius;
    float transparency; //0 - 1
    struct Color color;
};

struct Entity* create_sphere(struct Entity** entities, float radius, struct Color color);
struct Color sphere_color(struct Entity* entity);
float sphere_hit(struct Entity* entity, struct Ray* ray);

struct Entity* create_sphere(struct Entity** entities, float radius, struct Color color){
    void* sphere_mem = malloc(sizeof(struct Sphere));
    if(sphere_mem == NULL) return NULL;
    
    struct Entity *new_entity = create_entity(entities, SPHERE);
    if(new_entity == NULL) return NULL;

    struct Sphere *s = (struct Sphere*)sphere_mem;
    s->radius = radius;
    s->color = color;

    new_entity->type_data = sphere_mem;

    return new_entity;
}

float sphere_hit(struct Entity* entity, struct Ray* ray){
    if(entity->type != SPHERE) return -1;
    if(entity->type_data == NULL) return -1;

    struct Sphere *sphere_data = (struct Sphere*)entity->type_data;
    float radius = sphere_data->radius;

    // analytic solution
    struct Vector orig = pos_to_vec(ray->origin);
    struct Position world_pos = entity_to_world_space(entity);
    struct Vector center = pos_to_vec(world_pos);
    struct Vector dir = ray->direction;

    float t0, t1; // solutions for t if the ray intersects 
    
    struct Vector L = vec_sub(orig, center); 
    float a = vec_dot(dir, dir); 
    float b = 2 * vec_dot(dir, L);
    float c = vec_dot(L, L) - (radius*radius); 
    if (!solve_quadratic(a, b, c, &t0, &t1)) return -1; 


    if (t0 > t1) swap(&t0, &t1); 

    if (t0 < 0) { 
        t0 = t1; // if t0 is negative, let's use t1 instead 
        if (t0 < 0) return -1; // both t0 and t1 are negative 
    } 

    return t0; 
}

struct Color sphere_color(struct Entity* entity){
    struct Sphere* sphere_data = (struct Sphere*)entity->type_data;
    return sphere_data->color;
}

#endif /* SPHERE */