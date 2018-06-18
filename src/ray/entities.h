#ifndef ENTITIES
#define ENTITIES

#include "ray.h"
#include "ray_math.h"


//TODO(AL): Change this file to c and make a header
float sphere_hit(struct Entity* entity, struct Ray* ray);
float plane_hit(struct Entity* entity, struct Ray* ray);
float cubeoid_hit(struct Entity* entity, struct Ray* ray);

struct Color sphere_color(struct Entity* entity);
struct Color plane_color(struct Entity* entity);
struct Color cubeoid_color(struct Entity* entity);

struct Entity** entities; //array of entities within the scene
int entities_max = 4;


/* -- Entity Functions -- */
struct Entity* create_entity(enum Types type){

    struct Entity *new_entity = NULL;

    new_entity = malloc(sizeof(struct Entity));

    if(new_entity == NULL) return new_entity;

    new_entity->type = type;
    new_entity->type_data = NULL;
    new_entity->parent = NULL;
    new_entity->child_count = 0;
    new_entity->child_limit = 4; //likely atleast 4 children if any are being set
    new_entity->children = NULL;
    
    new_entity->diffusion = 1;
    new_entity->refractive_index = 0.01; //TODO(AL): Distinction so this doens't have to be non zero
    new_entity->opacity = 0;

    struct Transform trans = {0};
    new_entity->transform = trans;


    if(entities == NULL){
        //create first instance of entities array
        logger("info", "creating first instance of entities array");

        entities = a_new(struct Entity*, entities_max);

        if(entities == NULL){
            free(new_entity);
            return NULL;
        }

        for(int i = 0; i < a_size(entities); i++){
            entities[i] = NULL;
        }
        
    }

    //TODO(AL): refactor this allocation since we're using our array library
    //          does it even have to be pointers to entities??

    //allocate entity to entity array
    //we're going to fill in any empty spot (where it is NULL)
    bool allocated = false;

    for(int i = 0; i < a_size(entities); i++){
        if(entities[i] == NULL){
            entities[i] = new_entity;
            allocated = true;
            break;
        }
    }

    if(!allocated)
    {
        //all entity fields are taken so we need to allocate more memory
        logger("info", "Allocating new memory for entities");
        
        struct Entity **new_entities_mem = NULL;
        int new_entities_max = (2*entities_max);

        new_entities_mem = malloc(sizeof(struct Entity*)*new_entities_max);
        if(new_entities_mem == NULL)
        {
            a_free(entities);
            free(new_entity);
            return NULL;
        }

        //copy across old memory
        for(int i = 0; i < entities_max; i++) {
            new_entities_mem[i] = entities[i];
        }

        //previous max value is the new entity pointer
        //now we have a new max (works because we know all other mem locations are taken)
        new_entities_mem[entities_max] = new_entity;
        entities_max = new_entities_max;

        a_free(entities);

        entities = new_entities_mem;
    
    }

    return new_entity;
}

void destroy_entity(struct Entity* entity){
    if(entity == NULL) return;

    for(int i = 0; i < entity->child_count; i++)
    {
        destroy_entity(entity->children[i]);
        entity->children[i] = NULL;
    }

    if(entity->type_data != NULL) free(entity->type_data);
    if(entity->children != NULL) free(entity->children);
    entity->child_count = 0;
    entity->child_limit = 0;

    //we need to remove entity from entities array
    for(int i = 0; i < entities_max; i++)
    {
        if(entities[i] == entity){ //they point to the same entity memory
            entities[i] = NULL;
            break;
        }
    }

    free(entity);

    entity = NULL;

    return;
}

bool add_child(struct Entity* parent, struct Entity* child){
    if(parent == NULL) return false;
    if(child == NULL) return false;
    if(child->parent != NULL) return false;

    struct Entity **new_child_mem = NULL;

    if(parent->children == NULL)
    {
        if(parent->child_count > 0){
            parent->child_count = 0;
        }

        new_child_mem = malloc(sizeof(struct Entity*)*parent->child_limit);
    }
    else
    {
        if(parent->child_count >= parent->child_limit)
        {
            if(parent->child_limit <= 0) parent->child_limit = 1;

            parent->child_limit *= 2;
            new_child_mem = malloc(sizeof(struct Entity*)*parent->child_limit);
        }
    }

    if(new_child_mem != NULL)
    {
        //we have increased the size of the childrens array

        if(parent->children != NULL)
        {
            //copy old memory out and set new memory
            //free old memory
            struct Entity **old_child_mem = parent->children;

            for(int i = 0; i < parent->child_count; i++) {
                new_child_mem[i] = old_child_mem[i];
            }

            free(old_child_mem);
        }
        
        parent->children = new_child_mem;
    }

    //add new child to parent
    int offset = parent->child_count;

    if(offset >= parent->child_limit){
        return false;
    }

    parent->children[offset] = child;
    parent->child_count++;

    child->parent = parent;

    return true;
}

void print_entity_graph_recursive(struct Entity* e, int indent){
    if(e == NULL) return;

    for(int j = 0; j < indent; j++){
        printf(" ");
        if(j == indent-1) printf("|--");
    }

    switch(e->type)
    {
        case EMPTY: printf("(EMPTY) "); break;
        case SPHERE: printf("(SPHERE) "); break;
        case CUBEOID: printf("(CUBEOID) "); break;
        case PLANE: printf("(PLANE) "); break;
        default: printf("(NO TYPE SET)");
    }

    printf("children: %i", e->child_count);
    printf("\n");

    for(int i = 0; i < e->child_count; i++)
    {
        print_entity_graph_recursive(e->children[i], indent+1);
    }
    
}

void print_entity_graph(struct Entity* e){
    printf("---- ENTITIES ----\n");
    print_entity_graph_recursive(e, 0);
    printf("---- ENTITIES ----\n");
}

struct Position entity_to_world_space(struct Entity *e){
    struct Position ws = e->transform.position;
    
    struct Entity *tmp_e = e;
    while((tmp_e = tmp_e->parent) != NULL)
    {
        struct Position parent_pos = tmp_e->transform.position;
        ws.x += parent_pos.x;
        ws.y += parent_pos.y;
        ws.z += parent_pos.z;
    }

    //TODO(AL): Figure out how to apply rotation changes per parent

    return ws;
}

float entity_hit(struct Entity* entity, struct Ray* ray){
    if(entity == NULL || ray == NULL) return -1;

    
    switch(entity->type)
    {
        case SPHERE: return sphere_hit(entity ,ray);
        case PLANE: 
        case CUBEOID:
        case EMPTY: 
        default:
            return -1;
    }
}

struct Color entity_color(struct Entity* entity){
    struct Color c = {0};

    if(entity == NULL) return c;

    switch(entity->type)
    {
        case SPHERE: return sphere_color(entity); 
        case PLANE: return plane_color(entity);
        case CUBEOID: return cubeoid_color(entity);
        case EMPTY: return c;
    }

    return c;
}

float entity_diffusion(struct Entity* entity){
    if(entity == NULL) return 1;

    return entity->diffusion;

    // switch(entity->type)
    // {
    //     case SPHERE: return sphere_diffusion(entity);
    //     case PLANE: return plane_diffusion(entity);
    //     case CUBEOID: return cubeoid_diffusion(entity);
    //     case EMPTY: return 1;
    // }

    // return 1;
}

float entity_refractive_index(struct Entity* entity){
    if(entity == NULL) return 0;

    return entity->refractive_index;
}

float entity_opacity(struct Entity* entity){
    if(entity == NULL) return 0;
    return entity->opacity;
}

/* -- END Entity Functions -- */

/* -- Entity Type Functions -- */

struct Entity* create_sphere(float radius, struct Color color){
    void* sphere_mem = malloc(sizeof(struct Sphere));
    if(sphere_mem == NULL) return NULL;
    
    struct Entity *new_entity = create_entity(SPHERE);
    if(new_entity == NULL) return NULL;

    struct Sphere *s = (struct Sphere*)sphere_mem;
    s->radius = radius;
    s->color = color;

    new_entity->type_data = sphere_mem;

    return new_entity;
}

// returns the abs distance between r
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

struct Entity* create_cubeoid(float width, float height, struct Color color){
    void* cubeoid_mem = malloc(sizeof(struct Cubeoid));
    if(cubeoid_mem == NULL) return NULL;
    
    struct Entity* new_entity = create_entity(CUBEOID);
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

struct Entity* create_plane(struct Vector normal, struct Color color){
    void* plane_mem = malloc(sizeof(struct Plane));
    if(plane_mem == NULL) return NULL;

    struct Entity* new_entity = create_entity(PLANE);
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

/* -- END Entity Type Functions -- */

#endif