#include "ray.h"
#include "ray_math.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../shared/stb_image_write.h"

struct Entity* world; //top level world entity containing all others

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

			for(int i = 0; i < parent->child_count; i++)
			{
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

struct Entity* create_sphere(float radius){
	void* sphere_mem = malloc(sizeof(struct Sphere));
	if(sphere_mem == NULL) return NULL;
	
	struct Entity *new_entity = create_entity(SPHERE);
	if(new_entity == NULL) return NULL;

	struct Sphere *s = (struct Sphere*)sphere_mem;
	s->radius = radius;

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
	struct Color c = {0,0,1};

	return c;
}



bool cubeoid_hit(struct Entity* entity, struct Ray* ray){

	return -1;
}

struct Color cubeoid_color(struct Entity* entity, struct Ray* ray){
	struct Color c = {0,0,0};

	return c;
}

/* -- END Entity Type Functions -- */


/* -- Ray Library Functions -- */

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
		struct Entity* sphere = create_sphere(6.0);
		
		if(sphere != NULL)
		{
			struct Position pos = {i,0,30+2*i};
			sphere->transform.position = pos;

			bool added = add_child(prev, sphere);
		}

	}

	return true;
}

void destroy(){

	destory_entity(world);

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

struct Color render(struct Ray* ray){
	
	float max_dist = MAX_WORLD_DIST;
	
	struct Color c = render_recurse(world, ray, &max_dist);
	
	if(c.r < 0){
		c.r = 0;
		c.g = 0;
		c.b = 0;
	}

	return c;
}

/* END Ray Library Functions -- */

// const struct ray Ray = {
//     .setup = setup,
//     .destroy = destroy,
//     .render = render
// };

int main(void){

	setup(NULL);

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

			struct Vector direction;
			direction.x = (2 * (x + 0.5) / (float)width - 1) * aspect_ratio * scale; 
            direction.y = (1 - 2 * (y + 0.5) / (float)height) * scale; 
            direction.z = 1;
            vec_normalize(&direction);

			struct Ray r = {origin, direction};
			struct Color c = render(&r);

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

	return 0;
}