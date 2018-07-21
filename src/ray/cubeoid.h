#ifndef _CUBEOID
#define _CUBEOID

struct Cubeoid{
    float width;
    float height;
    float transparency; //0 - 1
    struct Color color;
};

struct Color cubeoid_color(struct Entity* entity);
float cubeoid_hit(struct Entity* entity, struct Ray* ray);
struct Entity* create_cubeoid(struct Entity** entities, float width, float height, struct Color color);

struct Entity* create_cubeoid(struct Entity** entities, float width, float height, struct Color color){
    void* cubeoid_mem = malloc(sizeof(struct Cubeoid));
    if(cubeoid_mem == NULL) return NULL;
    
    struct Entity* new_entity = create_entity(entities, CUBEOID);
    if(new_entity == NULL) return NULL;

    struct Cubeoid* c = (struct Cubeoid*)cubeoid_mem;
    c->width = width;
    c->height = height;
    c->transparency = 0; //TODO(AL): set this up when we do transparency handling
    c->color = color;

    new_entity->type_data = cubeoid_mem;

    return new_entity;
}

float cubeoid_hit(struct Entity* entity, struct Ray* ray){
    //TODO(AL): Implement
    return -1;
}

struct Color cubeoid_color(struct Entity* entity){
    struct Cubeoid* cubeoid = (struct Cubeoid*)entity->type_data;
    return cubeoid->color;
}

#endif /* CUBEOID */