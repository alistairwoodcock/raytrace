#include "ray.h"
#include "ray_math.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../shared/stb_image_write.h"



struct Entity* world; //top level world entity containing all others

struct Entity** entities; //array of entities within the scene
int entities_max = 4;

struct Ray** rays;
int rays_max = 500;

struct Color ambient;
struct Color background;

const float MAX_WORLD_DIST = 10000.0;

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

    //allocate entity to entity array
    //we're going to fill in any empty spot (where it is NULL)
    bool allocated = false;

    for(int i = 0; i < entities_max; i++){
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
        logger("info", "Max entities: %i", entities_max);

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

void destory_entity(struct Entity* entity){
    if(entity == NULL) return;

    for(int i = 0; i < entity->child_count; i++)
    {
        destory_entity(entity->children[i]);
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
    struct Color c;

    struct Sphere* sphere_data = (struct Sphere*)entity->type_data;

    c = sphere_data->color;

    return c;
}



bool cubeoid_hit(struct Entity* entity, struct Ray* ray){

    return -1;
}

struct Color cubeoid_color(struct Entity* entity){
    struct Color c = {0};

    

    return c;
}

/* -- END Entity Type Functions -- */


/* -- Ray Library Functions -- */

void create_reflected_ray(struct Entity* e, struct Ray* ray){

}

void create_refracted_ray(struct Entity* e, struct Ray* ray){
    //TODO(AL): Lets make this a thing once we have entities with material types
}

struct Color trace(struct Ray* ray){
    
    float min_dist = MAX_WORLD_DIST;
    
    struct Color c = background;

    struct Entity* hit_e = NULL;

    for(int i = 0; i < entities_max; i++)
    {
        struct Entity* e = entities[i];

        if(e == NULL) continue;

        switch(e->type)
        {
            case SPHERE: {
                float h = sphere_hit(e, ray);
                
                if(h >= 0 && h < min_dist){
                    hit_e = e;
                    min_dist = h;
                }
            } break;

            case CUBEOID: {

            } break;

            case EMPTY: {

            } break;
        }
    }

    if(min_dist == MAX_WORLD_DIST){
        return c;
    }

    struct Color surface_color;

    switch(hit_e->type)
    {
        case SPHERE: {
            surface_color = sphere_color(hit_e); 
        } break;

        case CUBEOID: {
            surface_color = cubeoid_color(hit_e);
        } break;

        case EMPTY: {

        } break;
    }

                    
    struct Vector hit_vec = vec_mult_scalar(ray->direction, min_dist);
    struct Vector hit_pos = vec_add(hit_vec, pos_to_vec(ray->origin));
    
    struct Vector hit_normal = vec_sub(hit_pos, pos_to_vec(hit_e->transform.position));
    vec_normalize(&hit_normal);

    float dir_normal_dot = vec_dot(ray->direction, hit_normal);
    
    struct Ray reflected_ray;
    reflected_ray.origin = vec_to_pos(vec_add(hit_pos, hit_normal));
    reflected_ray.direction = vec_sub(ray->direction, vec_mult_scalar(hit_normal, 2*dir_normal_dot));
    reflected_ray.life = ray->life-1;
    vec_normalize(&reflected_ray.direction);


    if(reflected_ray.life > 0)
    {
        struct Color reflected_color = trace(&reflected_ray);

        float facingratio = -vec_dot(ray->direction, hit_normal); 
        float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1); 

        struct Vector reflection_c = color_to_vec(reflected_color);
        struct Vector sphere_c = color_to_vec(surface_color);

        c = vec_to_color(vec_mult_vec(vec_mult_scalar(reflection_c, fresneleffect), sphere_c));
    }
    else
    {
        c = surface_color;
    }

    return c;
}


bool setup(char* scene_descriptor_fil){
    // if(scene_descriptor_fil == NULL) return false;

    // FILE *fd = NULL;

    // fd = fopen(scene_descriptor_fil, 'r');

    //TODO(AL): Get file size by skipping ahead
    //          allocate memory to read file in 
    //          parse scene into game state

    ambient.r = 0.8;
    ambient.g = 0.2;
    ambient.b = 0.5;

    background.r = 0.9;
    background.g = 0.3;
    background.b = 0.5;

    world = create_entity(EMPTY);

    if(world == NULL) return false;

    struct Entity* prev = world;

    struct Color c1 = {0, 1, 0};
    struct Entity* sphere1 = create_sphere(4.0, c1);
    sphere1->transform.position.x = 5;
    sphere1->transform.position.y = 0;
    sphere1->transform.position.z = 40;
    add_child(world, sphere1);


    struct Color c2 = {0, 0, 1};
    struct Entity* sphere2 = create_sphere(4.0, c2);
    sphere2->transform.position.x = -5;
    sphere2->transform.position.y = 0;
    sphere2->transform.position.z = 30;
    add_child(world, sphere2);

    // for(int i = 0; i < 20; i++)
    // {
    //  struct Color c = {rand2(), rand2(), rand2()};
    //  struct Entity* sphere = create_sphere(4.0, c);
        
    //  if(sphere != NULL)
    //  {
    //      struct Position pos = {i*4,0,30+4*i};
    //      sphere->transform.position = pos;

    //      bool added = add_child(prev, sphere);
    //  }

    // }


    return true;
}

void destroy(){

    destory_entity(world);

}

char* run(int width, 
                int height, 
                float aspect_ratio, 
                float fov, 
                float scale,
                int rays_per_pix)
{
    char* buff = malloc(sizeof(char) * width * height * 3);

    if(buff == NULL){
        logger("error", "Cannot allocate image buff");
        return NULL;
    }

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            int pix_offset = (y*width + x)*3;
            
            struct Position origin = {0,0,0};

            struct Color c = {0};

            for(int i = 0; i < rays_per_pix; i++)
            {
                struct Vector direction;

                float ray_x = x + (rand2() - rand2())/(float)2;
                float ray_y = y + (rand2() - rand2())/(float)2;

                direction.x = (2 * (ray_x + 0.5) / (float)width - 1) * aspect_ratio * scale; 
                direction.y = (1 - 2 * (ray_y + 0.5) / (float)height) * scale; 
                direction.z = 1;
                vec_normalize(&direction);

                struct Ray r = {0,0,2,origin, direction};
                struct Color ray_color = trace(&r); 

                c.r += ray_color.r / (float)rays_per_pix;
                c.g += ray_color.g / (float)rays_per_pix;
                c.b += ray_color.b / (float)rays_per_pix;
            }
            

            buff[pix_offset + 0] = 255 * c.r; 
            buff[pix_offset + 1] = 255 * c.g;
            buff[pix_offset + 2] = 255 * c.b;
        }

        
    }

    return buff;
}

/* END Ray Library Functions -- */

// const struct ray Ray = {
//     .setup = setup,
//     .destroy = destroy,
//     .render = render
// };

int main(void){
    FILE* log = fopen("./logs/ray.log", "a");

    logger_file(log);
    logger_stdout_print(true);

    logger("info", "~~~ starting ray trace process %d ~~~", 10);

#if 0
    while(true)
    {
        setup(NULL);
        sleep(2);
        destroy();
    }

#endif

#if 1
    setup(NULL);

    int rays_per_pix = 10;

    int width = 1920/4;
    int height = 1080/4;
    float aspect_ratio = width / (float)height; 
    float fov = 45;
    float scale = tan(to_radians(fov * 0.5)); 
    

    char *buff = run(width, height, aspect_ratio, fov, scale, rays_per_pix);
        
    destroy();

    int result = stbi_write_bmp("./image.bmp", width, height, 3, buff);

    free(buff);

    if(result == 0){
        logger("error","Failed to write image to file");
    } else {
        logger("info","Image written to: ./image.bmp");
    }
#endif


    return 0;
}