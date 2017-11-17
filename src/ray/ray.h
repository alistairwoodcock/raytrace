/*

	GOALS:
		scene description —— file describes primitive types in scene (spheres, cube, light source)
		render single ray —— cast ray through scene
		queue additional rays —— return generated rays, have current ray as parent

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef enum { false, true } bool;

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
	

	float x,y,z; //position relative to parent
	float x_rot, y_rot, z_rot; //rotation relative to parent
};

struct Ray{
	float x,y,z;
};

struct Color{
	char r,g,b;
};

// struct ray{
// 	bool (*setup)(char*);
// 	void (*destroy)(void);
// 	void (*render)();
// };

// extern const struct ray Ray;