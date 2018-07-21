#ifndef ENTITIES
#define ENTITIES

#include "ray.h"
#include "ray_math.h"

enum Types{
    EMPTY,
    PLANE,
    SPHERE,
    CUBEOID
};

struct Entity{
    enum Types type; //type to determine entity
    void* type_data; //actual entity data representation
    struct Entity* parent; //single parent

    int child_count;
    int child_limit;
    struct Entity** children; //pointer to children array (ref by pointers)
    
    struct Transform transform; //transform relative to parent

    float diffusion; //TODO(AL): probably move this into the specific entity type
    float refractive_index; //TODO(AL): probably move this into the specific entity type
    float opacity; //TODO(AL): probably move this into the specific entity type
};

struct Entity* create_entity(struct Entity** entities, enum Types type);
void destroy_entity(struct Entity** entities, struct Entity* entity);

bool add_child(struct Entity* parent, struct Entity* child);
void print_entity_graph_recursive(struct Entity* e, int indent);
void print_entity_graph(struct Entity* e);
struct Position entity_to_world_space(struct Entity *e);
float entity_hit(struct Entity* entity, struct Ray* ray);
struct Color entity_color(struct Entity* entity);
float entity_diffusion(struct Entity* entity);
float entity_refractive_index(struct Entity* entity);
float entity_opacity(struct Entity* entity);

#include "cubeoid.h"
#include "plane.h"
#include "sphere.h"

int default_entities_max = 4;

// creates new entity in entities array
// entities is null, allocates memory for it
struct Entity* create_entity(struct Entity** entities, enum Types type){

    struct Entity *new_entity = NULL;

    // TODO(AL): Why are we even allocating
    //           in this way?? when it can
    //           all be alloc in the entities
    //           array??

    // TODO(AL): Better allocator for this
    //           so we don't just allocate 
    //           to new rando parts of memory
    new_entity = malloc(sizeof(struct Entity));

    if(new_entity == NULL) return new_entity;

    new_entity->type = type;
    new_entity->type_data = NULL;
    new_entity->parent = NULL;
    new_entity->child_count = 0;
    new_entity->child_limit = 4; // likely atleast 4 children if any are being set
    new_entity->children = NULL;
    
    new_entity->diffusion = 1;
    // TODO(AL): Distinction so this doens't have to be non zero
    new_entity->refractive_index = 0.01; 
    new_entity->opacity = 0;

    struct Transform trans = {0};
    new_entity->transform = trans;


    if(entities == NULL){
        // create first instance of entities array
        logger("info", "creating first instance of entities array");

        entities = a_new(struct Entity*, default_entities_max);

        if(entities == NULL){
            free(new_entity);
            return NULL;
        }

        for(int i = 0; i < a_size(entities); i++){
            entities[i] = NULL;
        }
        
    }

    // allocate entity to entity array
    // we're going to fill in any empty spot (where it is NULL)
    bool allocated = false;

    for(int i = 0; i < a_size(entities); i++){
        if(entities[i] == NULL){
            entities[i] = new_entity;
            allocated = true;
            break;
        }
    }

    if(allocated) return new_entity; // we done
    
    // all entity fields are taken so we need to allocate more memory
    logger("info", "Allocating new memory for entities");

    // put in next available space and will maybe grow array
    bool pushed = a_push(entities, new_entity);

    return new_entity;
}

void destroy_entity(struct Entity** entities, struct Entity* entity){
    if(entities == NULL) return;
    if(entity == NULL) return;

    for(int i = 0; i < entity->child_count; i++){
        destroy_entity(entities, entity->children[i]);
        entity->children[i] = NULL;
    }

    if(entity->type_data != NULL) free(entity->type_data);
    if(entity->children != NULL) free(entity->children);
    entity->child_count = 0;
    entity->child_limit = 0;

    //we need to remove entity from entities array
    for(int i = 0; i < a_size(entities); i++)
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

#endif