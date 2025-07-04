
#include <dc/fmath.h>
#include <dc/pvr.h>
#include <kos.h>
#include <stdlib.h>
#include <dc/matrix3d.h> 

#include "helpers.h"
#include "scene.h"

#include "data.h"

void initScene(Scene* scene){
	scene->light.x = 1.0f;
	scene->light.y = 1.0f;
	scene->light.z = 1.0f;
	normalize3(&(scene->light));

	scene->count = 4;
	scene->objects = (Object*)calloc(scene->count, sizeof(Object));
	
	scene->objects[0].vertices = &vertices_monkey[0];
	scene->objects[0].normals = &normals_monkey[0];
	scene->objects[0].uvs = &texcoords_monkey[0];
	scene->objects[0].indices = &points_monkey[0];
	scene->objects[0].vCount = vertex_count_monkey;
	scene->objects[0].iCount = points_count_monkey;
	scene->objects[0].position.x = -1.f;
	scene->objects[0].position.y = 0.f;
	scene->objects[0].position.z = 1.f;
	scene->objects[0].scale = 1.0f;
	scene->objects[0].angle = 0.0f;
	scene->objects[0].shininess = 5.0f;
	scene->objects[0].lit = true;

	// Floor
	scene->objects[1].vertices = &vertices_plane[0];
	scene->objects[1].normals = &normals_plane[0];
	scene->objects[1].uvs = &texcoords_plane[0];
	scene->objects[1].indices = &points_plane[0];
	scene->objects[1].vCount = vertex_count_plane;
	scene->objects[1].iCount = points_count_plane;
	scene->objects[1].position.x = 0.f;
	scene->objects[1].position.y = -1.5f;
	scene->objects[1].position.z = 0.f; 
	scene->objects[1].scale = 10.0f;
	scene->objects[1].angle = 0.0f;
	scene->objects[1].shininess = 20.0f;
	scene->objects[1].lit = true;

	// Dragon
	scene->objects[2].vertices = &vertices_dragon[0];
	scene->objects[2].normals = &normals_dragon[0];
	scene->objects[2].uvs = &texcoords_dragon[0];
	scene->objects[2].indices = &points_dragon[0];
	scene->objects[2].vCount = vertex_count_dragon;
	scene->objects[2].iCount = points_count_dragon;
	scene->objects[2].position.x = 1.f;
	scene->objects[2].position.y = 0.f;
	scene->objects[2].position.z = -1.f; 
	scene->objects[2].scale = 1.0f;
	scene->objects[2].angle = 0.0f;
	scene->objects[2].shininess = 40.0f;
	scene->objects[2].lit = true;

	// Skybox
	scene->objects[3].vertices = &vertices_cube[0];
	scene->objects[3].normals = &normals_cube[0];
	scene->objects[3].uvs = &texcoords_cube[0];
	scene->objects[3].indices = &points_cube[0];
	scene->objects[3].vCount = vertex_count_cube;
	scene->objects[3].iCount = points_count_cube;
	scene->objects[3].position.x = 0.f;
	scene->objects[3].position.y = 0.f;
	scene->objects[3].position.z = 0.f;
	scene->objects[3].scale = 20.0f;
	scene->objects[3].angle = 0.0f;
	scene->objects[3].shininess = 0.0f;
	scene->objects[3].lit = false;
	
	scene->maxVertexCount = 0;
	for(unsigned int i = 0; i < scene->count; ++i){
		scene->maxVertexCount = MAX(scene->maxVertexCount, scene->objects[i].vCount);
	}

}

void updateScene(Scene* scene, float time){
	// Update monkey head angle
	Object* monkey =  &scene->objects[0];
	monkey->angle -= 1.20f / 60.0f;
	if( monkey->angle < 0.0f){
		monkey->angle += 2.0f * F_PI;
	}

	scene->light.x = 1.0f;
	scene->light.y = 0.5f * fsin(time) + 1.0f;
	scene->light.z = 1.0f;
	normalize3(&scene->light);
}

void cleanupScene(Scene* scene){
	free(scene->objects);
}


void updateCamera(Camera* cam){
	cam->pos.x = cam->radius * fcos(cam->angleHoriz) * fcos(cam->angleVert);
	cam->pos.y = cam->radius * fsin(cam->angleVert);
	cam->pos.z = cam->radius * fsin(cam->angleHoriz) * fcos(cam->angleVert);
	// Update view matrix
	mat_load(&cam->proj);
	mat_lookat(&cam->pos, &cam->lookAt, &cam->up);
	mat_store(&cam->viewProj);  
}

void initCamera(Camera* cam){
	cam->angleHoriz = F_PI * 0.5f;
	cam->angleVert = F_PI * 0.15f;
	cam->radius = 7.0f;
	setVector(&cam->pos, 0.0f, 0.0f, 0.0f, 1.0f);
	setVector(&cam->lookAt, 0.0f, 0.0f, 0.0f, 1.0f);
	setVector(&cam->up , 0.0f, -1.0f, 0.0f, 0.f);
	// Setup projection matrix once and for all.
	mat_identity();
	const float fov = 45.0f * F_PI / 180.0f;
	float cotFov2 = 1.0f / ftan(fov * 0.5f);
	mat_perspective(320.0f, 240.0f, cotFov2, 0.1f, +10.0f);
	mat_store(&cam->proj);
	updateCamera(cam);
}


