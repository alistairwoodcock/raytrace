/*

	GOALS:
		scene description —— file describes primitive types in scene (spheres, cube, light source)
		render single ray —— cast ray through scene
		queue additional rays —— return generated rays, have current ray as parent

*/
#ifndef ALS_RAY
#define ALS_RAY

#include "../shared/utils.h"
#include "../shared/array.h"
#include "../shared/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


struct Vector{
	float x,y,z; 
};

struct Position{
	float x,y,z;
};

struct Rotation{
	float x,y,z;
};

struct Color{
	float r,g,b;
};

struct Transform{
	struct Position position;
	struct Rotation rotation;
};

struct Ray{
	int id;
	int parent;
	int life;
	struct Position origin;
	struct Vector direction;
};

enum Types{
	EMPTY,
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
};

struct Sphere{
	float radius;
	struct Color color;
};

struct Color trace(struct Ray* ray);
bool setup(char* scene_descriptor_fil);
void destroy();

// struct ray{
// 	bool (*setup)(char*);
// 	void (*destroy)(void);
// 	void (*render)();
// };

// extern const struct ray Ray;

#endif