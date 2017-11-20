#include "ray.h"
#include "ray_math.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../shared/stb_image_write.h"

struct Entity* world; //top level world entity containing all others

struct Entity** entities;
int entities_max = 64;

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
		printf("creating first instance of entities array\n");

		entities = malloc(sizeof(struct Entity*) * entities_max);
		if(entities == NULL){
			free(new_entity);
			return NULL;
		}

		if(entities != NULL)
		{
			for(int i = 0; i < entities_max; i++){
				entities[i] = NULL;
			}
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
		printf("Allocating new memory for entities\n");
		printf("Max entities: %i\n", entities_max);

		struct Entity **new_entities_mem = NULL;
		int new_entities_max = (2*entities_max);

		new_entities_mem = malloc(sizeof(struct Entity*)*new_entities_max);
		if(new_entities_mem == NULL)
		{
			free(entities);
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

		free(entities);

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
	struct Vector orig = {ray->origin.x,ray->origin.y,ray->origin.z};
	struct Position world_pos = entity_to_world_space(entity);
	struct Vector center = {world_pos.x,world_pos.y,world_pos.z};
	struct Vector dir = {ray->direction.x,ray->direction.y,ray->direction.z};

	float t0, t1; // solutions for t if the ray intersects 
    
    struct Vector L = vec_sub(orig, center); 
    float a = dot_product(dir, dir); 
    float b = 2 * dot_product(dir, L);
    float c = dot_product(L, L) - (radius*radius); 
    if (!solve_quadratic(a, b, c, &t0, &t1)) return -1; 


    if (t0 > t1) swap(&t0, &t1); 

    if (t0 < 0) { 
        t0 = t1; // if t0 is negative, let's use t1 instead 
        if (t0 < 0) return -1; // both t0 and t1 are negative 
    } 

    return t0; 
}

struct Color sphere_color(struct Entity* entity, struct Ray* ray){
	struct Color c;

	struct Sphere* sphere_data = (struct Sphere*)entity->type_data;

	c = sphere_data->color;

	return c;
}



bool cubeoid_hit(struct Entity* entity, struct Ray* ray){

	return -1;
}

struct Color cubeoid_color(struct Entity* entity, struct Ray* ray){
	struct Color c;

	

	return c;
}

/* -- END Entity Type Functions -- */


/* -- Ray Library Functions -- */

void create_reflected_ray(struct Entity* e, struct Ray* ray){



}

void create_refracted_ray(struct Entity* e, struct Ray* ray){
	//TODO(AL): Lets make this a thing once we have entities with material types
}

struct Color render_recurse(struct Entity* e, struct Ray* ray, float* max_dist){
	struct Color c = {-1,0,0};
	
	float hit = -1;

	switch(e->type)
	{
		case SPHERE:
			hit = sphere_hit(e, ray);
			
			if(hit >= 0 && hit < *max_dist){
				c = sphere_color(e, ray);	
				*max_dist = hit;
			}
			
		break;

		case CUBEOID: 
			// hit = cubeoid_hit(e, ray);
			// if(hit >= 0) c = cubeoid_color(e, ray);

		break;

		case EMPTY: 
		default:
			hit = -1;
	}

	for(int i = 0; i < e->child_count; i++)
	{
		struct Color tmp_c = render_recurse(e->children[i], ray, max_dist);
		if(tmp_c.r >= 0) c = tmp_c;
	}
	
	return c;
}

struct Color render_loop(struct Ray* ray, float* max_dist){
	struct Color c = {-1,0,0};

	float hit = -1;

	for(int i = 0; i < entities_max; i++)
	{
		struct Entity* e = entities[i];

		if(e == NULL) continue;

		float hit = -1;

		switch(e->type)
		{
			case SPHERE:
				hit = sphere_hit(e, ray);
				
				if(hit >= 0 && hit < *max_dist){
					c = sphere_color(e, ray);	
					
					create_reflected_ray(e, ray);
					create_refracted_ray(e, ray);

					*max_dist = hit;
				}
				
			break;

			case CUBEOID: 
			break;

			case EMPTY: 
			break;
		}
	}


	return c;
}

struct Color render(struct Ray* ray){
	
	float max_dist = MAX_WORLD_DIST;
	
	// struct Color ray_c = render_recurse(world, ray, &max_dist);
	struct Color ray_c = render_loop(ray, &max_dist);

	float dist_percent = max_dist/MAX_WORLD_DIST;

	struct Color c = {
		(ray_c.r*0.8) + (ambient.r*0.2), 
		(ray_c.g*0.8) + (ambient.r*0.2), 
		(ray_c.b*0.8) + (ambient.b*0.2)
	};

	if(max_dist == MAX_WORLD_DIST)
	{
		c = background;
	}
	else
	{
		// struct Vector light_normal = {0,0,1};
		// struct Vector ray_vec = ray->direction;

		// printf("dot: %f\n", dot_product(light_normal, ray_vec));
	}

	return c;
}


bool setup(char* scene_descriptor_fil){
	// if(scene_descriptor_fil == NULL) return false;

	// FILE *fd = NULL;

	// fd = fopen(scene_descriptor_fil, 'r');

	//TODO(AL): Get file size by skipping ahead
	//			allocate memory to read file in 
	//			parse scene into game state

	world = create_entity(EMPTY);

	if(world == NULL) return false;

	struct Entity* prev = world;

	for(int i = 0; i < 20; i++)
	{
		struct Color c = {rand2(), rand2(), rand2()};
		struct Entity* sphere = create_sphere(4.0, c);
		
		if(sphere != NULL)
		{
			struct Position pos = {i*4,0,30+4*i};
			sphere->transform.position = pos;

			bool added = add_child(prev, sphere);
		}

	}

	struct Color amb = {0.8,0.2,0.5};
	ambient = amb;

	struct Color bg = {0.9,0.3,0.5};
	background = bg;

	return true;
}

void destroy(){

	destory_entity(world);

}

/* END Ray Library Functions -- */

// const struct ray Ray = {
//     .setup = setup,
//     .destroy = destroy,
//     .render = render
// };

int main(void){

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

	int rays_per_pix = 5;

	int width = 1920;
	int height = 1080;
	float aspect_ratio = width / (float)height; 
	float fov = 45;
	float scale = tan(to_radians(fov * 0.5)); 
	

	char *buff = malloc(sizeof(char) * width * height * 3);
	if(buff == NULL){
		printf("Cannot allocate image buff\n");
		return 0;
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

	            // create_ray(origin, direction, 5);

				struct Ray r = {0,0,0,origin, direction};
				struct Color ray_color = render(&r);	

				c.r += ray_color.r / (float)rays_per_pix;
				c.g += ray_color.g / (float)rays_per_pix;
				c.b += ray_color.b / (float)rays_per_pix;
			}
			

			buff[pix_offset + 0] = 255 * c.r; 
			buff[pix_offset + 1] = 255 * c.g;
			buff[pix_offset + 2] = 255 * c.b;
		}

		
	}
		
	destroy();

	int result = stbi_write_bmp("./image.bmp", width, height, 3, buff);

	if(result == 0){
		printf("Failed to write image to file\n");
	} else {
		printf("Image written to: ./image.bmp\n");
	}
#endif


	return 0;
}