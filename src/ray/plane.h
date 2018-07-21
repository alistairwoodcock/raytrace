#ifndef _PLANE
#define _PLANE

struct Plane{
    float transparency; //0 - 1
    struct Color color;
    struct Vector normal;
};

struct Color plane_color(struct Entity* entity);
float plane_hit(struct Entity* entity, struct Ray* ray);
struct Entity* create_plane(struct Entity** entities, struct Vector normal, struct Color color);

struct Entity* create_plane(struct Entity** entities, struct Vector normal, struct Color color){
    void* plane_mem = malloc(sizeof(struct Plane));
    if(plane_mem == NULL) return NULL;

    struct Entity* new_entity = create_entity(entities, PLANE);
    if(new_entity == NULL) return NULL;

    struct Plane* p = (struct Plane*)plane_mem;
    p->transparency = 0; //TODO(AL): Figure this out when we do transparency
    p->color = color;
    p->normal = normal;

    new_entity->type_data = plane_mem;

    return new_entity;
}

float plane_hit(struct Entity* entity, struct Ray* ray){
    //TODO(AL): Implement
    return -1;
}

struct Color plane_color(struct Entity* entity){
    struct Plane* plane_data = (struct Plane*)entity->type_data;
    return plane_data->color;
}

#endif /* PLANE */