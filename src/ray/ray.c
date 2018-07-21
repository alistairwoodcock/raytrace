#include "ray.h"
#include "ray_math.h"
#include "entities.h"

#include <time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../shared/stb_image_write.h"

struct Entity* world; //top level world entity containing all others
struct Entity** entities; //array of entities within the scene

struct Ray** rays;
int rays_max = 500;

struct Color ambient;
struct Color background;

const float MAX_WORLD_DIST = 10000.0;

float hit(struct Ray* ray, struct Entity** hit_e){
    float min_dist = MAX_WORLD_DIST;

    int entities_count = a_count(entities);

    for(int i = 0; i < entities_count; i++)
    {
        struct Entity* e = entities[i];

        if(e == NULL) continue;

        float h = entity_hit(e, ray);

        if(h >= 0 && h < min_dist){
            *hit_e = e;
            min_dist = h;
        }
    }

    return min_dist;
}

struct Color trace(struct Ray* ray){
    
    struct Color c = background;

    struct Entity* hit_e = NULL;
    
    float min_dist = hit(ray, &hit_e);

    if(min_dist == MAX_WORLD_DIST || hit_e == NULL){
        return c;
    }

    struct Color surface_color = entity_color(hit_e);

    int next_ray_life = ray->life-1;

    if(next_ray_life == 0) 
        return surface_color;
    

    struct Vector hit_vec = vec_mult_scalar(ray->direction, min_dist);
    struct Vector hit_pos = vec_add(hit_vec, pos_to_vec(ray->origin));
    
    struct Vector hit_normal = vec_sub(hit_pos, pos_to_vec(hit_e->transform.position));
    vec_normalize(&hit_normal);


    struct Ray reflected_ray;

    float dir_normal_dot = vec_dot(ray->direction, hit_normal);
    float facingratio = -vec_dot(ray->direction, hit_normal); 
    float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1); 

    reflected_ray.origin = vec_to_pos(vec_add(hit_pos, hit_normal));
    reflected_ray.direction = vec_sub(
                                    ray->direction, 
                                    vec_mult_scalar(hit_normal, 2*dir_normal_dot)
                              );

    reflected_ray.life = next_ray_life;
    vec_normalize(&reflected_ray.direction);


    struct Ray refracted_ray;
    
    float cosi = clamp(-1, 1, vec_dot(ray->direction, hit_normal));
    float etai = 1;
    float etat = entity_refractive_index(hit_e);
    struct Vector n = hit_normal;
    if(cosi < 0){ 
        cosi = -cosi; 
    } else {
        float temp = etai;
        etai = etat;
        etat = temp;
        n = vec_mult_scalar(hit_normal, -1);
    }
    
    float eta = etai / etat;

    float k = 1 - eta * eta * (1 - cosi * cosi);

    if(k < 0){
        refracted_ray.direction.x = 0;
        refracted_ray.direction.y = 0;
        refracted_ray.direction.z = 0;
    } else {
        refracted_ray.direction = vec_add(
                                    vec_mult_scalar(ray->direction, eta),
                                    vec_mult_scalar(hit_normal, eta * cosi - sqrtf(k))       
                                  );    
    }
    

    refracted_ray.origin = vec_to_pos(vec_add(hit_pos, hit_normal));
    vec_normalize(&refracted_ray.direction);

    refracted_ray.life = next_ray_life;

    struct Color reflected_color = trace(&reflected_ray);

    struct Vector surface_color_v = color_to_vec(surface_color);
    struct Vector reflected_color_v = color_to_vec(reflected_color);


    if(refracted_ray.direction.x != 0 &&
       refracted_ray.direction.y != 0 && 
       refracted_ray.direction.z != 0)
    {
        struct Color refracted_color = trace(&refracted_ray);
        struct Vector refracted_color_v = color_to_vec(refracted_color);

        c = vec_to_color(
                vec_add(
                    vec_mult_scalar(surface_color_v, (1 - entity_diffusion(hit_e))),
                    vec_mult_scalar(
                        vec_add(
                            vec_mult_scalar(reflected_color_v, fresneleffect),
                            vec_mult_scalar(refracted_color_v, (1 - fresneleffect))
                        ),
                        entity_diffusion(hit_e)
                    )
                )
            );
    }
    else
    {
        c = vec_to_color(
                vec_add(
                    vec_mult_scalar(reflected_color_v, (1 - entity_diffusion(hit_e))),
                    vec_mult_scalar(surface_color_v, entity_diffusion(hit_e))
                )
            );
    }

    return c;
}


bool setup(char* scene_descriptor_fil){

    entities = a_new(struct Entity*, 4);

    ambient.r = 0.8;
    ambient.g = 0.2;
    ambient.b = 0.5;

    background.r = 0.9;
    background.g = 0.3;
    background.b = 0.5;

    a_print(entities);
    world = create_entity(entities, EMPTY);
    a_print(entities);


    if(world == NULL) return false;

    struct Entity* prev = world;

    struct Color c1 = {1, 0, 0};
    struct Entity* sphere1 = create_sphere(entities, 4.0, c1);
    sphere1->transform.position.x = 7.5;
    sphere1->transform.position.y = 0;
    sphere1->transform.position.z = 20;
    sphere1->diffusion = 0.5;
    sphere1->refractive_index = 0.001;
    add_child(world, sphere1);


    struct Color c2 = {0, 1, 0};
    struct Entity* sphere2 = create_sphere(entities, 4.0, c2);
    sphere2->transform.position.x = -3.5;
    sphere2->transform.position.y = 0;
    sphere2->transform.position.z = 10;
    sphere2->diffusion = 0.3;
    sphere2->refractive_index = 0.001;
    add_child(world, sphere2);



    struct Color c3 = {0, 0, 1};
    struct Entity* sphere3 = create_sphere(entities, 4.0, c3);
    sphere3->transform.position.x = 0;
    sphere3->transform.position.y = 0;
    sphere3->transform.position.z = 17.5;
    sphere3->diffusion = 0.5;
    sphere3->refractive_index = 0.001;
    add_child(world, sphere3);

    a_print(entities);
    return true;
}

void destroy(){

    destroy_entity(entities, world);
}

char* run(int start_x,
          int end_x,
          int start_y,
          int end_y, 
          int width,
          int height,
          float aspect_ratio, 
          float fov, 
          float scale,
          int rays_per_pix)
{
    int px_width = (end_x - start_x);
    int px_height = (end_y - start_y);

    if(px_width <= 0 || px_height <= 0) return NULL;

    char* buff = malloc(sizeof(char) * px_width * px_height * 3);

    if(buff == NULL){
        logger("error", "Cannot allocate image buffer");
        return NULL;
    }

    for(int y = start_y, y_count = 0; y < end_y; y++, y_count++)
    {
        for(int x = start_x, x_count = 0; x < end_x; x++, x_count++)
        {
            int px_offset = (y_count*px_width + x_count)*3;

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

                struct Ray r = {0,0,8,origin, direction};
                struct Color ray_color = trace(&r);

                c.r += ray_color.r / (float)rays_per_pix;
                c.g += ray_color.g / (float)rays_per_pix;
                c.b += ray_color.b / (float)rays_per_pix;
            }
            

            buff[px_offset + 0] = 255 * c.r; 
            buff[px_offset + 1] = 255 * c.g;
            buff[px_offset + 2] = 255 * c.b;
        }
    }

    return buff;
}

/* -- END Ray Library Functions -- */

int main(void){
    FILE* log = fopen("./logs/ray.log", "a");

    logger_file(log);
    logger_stdout_print(true);

    logger("info", "~~~ starting ray trace process %d ~~~", 10);

    setup(NULL);

    int rays_per_pix = 50;

    int width = 1920/2;
    int height = 1080/2;
    float aspect_ratio = width / (float)height; 
    float fov = 45;
    float scale = tan(to_radians(fov * 0.5)); 

    double time_total = 0;
    double time_count = 0;

    char* buff = NULL;

    // for(int i = 0; i < 3; i++)
    // {
        clock_t begin = clock();

        buff = run(0, width, 0, height, width, height, aspect_ratio, fov, scale, rays_per_pix);

        clock_t end = clock();
        
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        time_total += time_spent;
        time_count += 1;

        free(buff);
    // }

    logger("timing", "run: %fs", time_total/time_count);

    // char *buff1 = run(0, width/2, 0, height/2, width, height, aspect_ratio, fov, scale, rays_per_pix);
    // char *buff2 = run(width/2, width, 0, height/2, width, height, aspect_ratio, fov, scale, rays_per_pix);
    // char *buff3 = run(0, width/2, height/2, height, width, height, aspect_ratio, fov, scale, rays_per_pix);
    // char *buff4 = run(width/2, width, height/2, height, width, height, aspect_ratio, fov, scale, rays_per_pix);
        
    destroy();

    // stbi_write_bmp("./image1.bmp", width/2, height/2, 3, buff1);
    // stbi_write_bmp("./image2.bmp", width/2, height/2, 3, buff2);
    // stbi_write_bmp("./image3.bmp", width/2, height/2, 3, buff3);
    // stbi_write_bmp("./image4.bmp", width/2, height/2, 3, buff4);

    stbi_write_bmp("./image.bmp", width, height, 3, buff);
    

    // char* finalBuff = malloc(width * height * 3);


    // if(result == 0){
    //     logger("error","Failed to write image to file");
    // } else {
    //     logger("info","Image written to: ./image.bmp");
    // }

    return 0;
}