#include "ray.h"

struct Entity* world; //top level world entity containing all others

const int MAX_WORLD_DIST = 10000;

/* -- Entity Functions -- */
struct Entity* createEntity(enum Types type){

	struct Entity *new_entity = NULL;

	new_entity = malloc(sizeof(struct Entity));

	if(new_entity == NULL) return new_entity;

	new_entity->type = type;
	new_entity->type_data = NULL;
	new_entity->parent = NULL;
	new_entity->child_count = 0;
	new_entity->child_limit = 4; //likely atleast 4 children if any are being set
	new_entity->children = NULL;
	new_entity->x = 0;
	new_entity->y = 0;
	new_entity->z = 0;
	new_entity->x_rot = 0;
	new_entity->y_rot = 0;
	new_entity->z_rot = 0;

	return new_entity;
}

void destoryEntity(struct Entity* entity){
	if(entity == NULL) return;

	for(int i = 0; i < entity->child_count; i++)
	{
		destoryEntity(entity->children[i]);
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

bool addChild(struct Entity* parent, struct Entity* child){
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

void printEntityGraphRecursive(struct Entity* e, int indent){
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

	printf("{%f,%f,%f}", e->x,e->y,e->z);

	printf("children: %i", e->child_count);
	printf("\n");

	for(int i = 0; i < e->child_count; i++)
	{
		printEntityGraphRecursive(e->children[i], indent+1);
	}
	
}

void printEntityGraph(struct Entity* e){
	printf("---- ENTITIES ----\n");
	printEntityGraphRecursive(e, 0);
	printf("---- ENTITIES ----\n");
}


/* -- END Entity Functions -- */

/* -- Entity Type Functions -- */

//Hit updates the min_dist if the dist between the ray and entity
// is less than the current min_dist
bool sphere_hit(struct Entity* entity, struct Ray* ray, int* min_dist){

	return false;
}

bool cubeoid_hit(struct Entity* entity, struct Ray* ray, int* min_dist){

	return false;
}

struct Color sphere_color(struct Entity* entity, struct Ray* ray){
	struct Color c = {0,0,0};

	return c;
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

	world = createEntity(EMPTY);

	if(world == NULL) return false;

	struct Entity* prev = world;

	for(int i = 0; i < 100; i++)
	{
		// struct Entity* sphere = createEntity(SPHERE);
		// if(sphere != NULL)
		// {
		// 	sphere->x = (float)i;
		// 	sphere->y = (float)i;
		// 	sphere->z = (float)i;

		// 	bool added = addChild(prev, sphere);
		// 	if(added && i%20 == 0) prev = sphere;

		// }

		// struct Entity* sphere = createSphere();
	}

	return true;
}

void destroy(){

	destoryEntity(world);

}


struct Color render_recurse(struct Entity* e, struct Ray* ray, int* min_dist){
	struct Color c = {0,0,0};
	
	bool hit = false;

	switch(e->type)
	{
		case SPHERE:
			hit = sphere_hit(e, ray, min_dist);
			if(hit) c = sphere_color(e, ray);
		break;

		case CUBEOID: 
			hit = cubeoid_hit(e, ray, min_dist);
			if(hit) c = cubeoid_color(e, ray);
		break;

		case EMPTY: 
		default:
			hit = false;
	}

	for(int i = 0; i < e->child_count; i++)
	{
		c = render_recurse(e->children[i], ray, min_dist);
	}
	
	return c;
}

struct Color render(struct Ray* ray){
	
	int min_dist = MAX_WORLD_DIST;
	
	struct Color c = render_recurse(world, ray, &min_dist);

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
		
	destroy();

	return 0;
}