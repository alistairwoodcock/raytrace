/*

	GOALS:
		scene description —— file describes primitive types in scene (spheres, cube, light source)
		render single ray —— cast ray through scene
		queue additional rays —— return generated rays, have current ray as parent

*/
#ifndef _RAY
#define _RAY

#include "../shared/array.h"
#include "../shared/utils.h"
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