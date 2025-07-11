
#include <dc/fmath.h>
#include <dc/pvr.h>
#include <kos.h>
#include <stdlib.h>
#include <dc/matrix3d.h> 

#include "helpers.h"
#include "scene.h"

#include "data.h"

void loadTexture(const char* texturePath, const char* palettePath, uint8_t * nextPalette, Texture * output){
	// We expect all textures to be compressed and RGB565.
	// Some might use a 8bits palette, others not.
	FILE* f = fopen(texturePath, "rb");

	if(!f){
		return;
	}

	unsigned char fourCC[4];
	fread(&fourCC[0], sizeof(unsigned char), 4, f);
	
	uint32_t sizeInBytes;
	fread(&sizeInBytes, sizeof(uint32_t), 1, f);

	unsigned char flags[4];
	fread(&flags[0], sizeof(char), 4, f);
	// Assume compressed.
	uint16_t codebookSize = flags[2] != 0 ? flags[2]+1 : 0;
	uint16_t paletteSize  = flags[3] != 0 ? flags[3]+1 : 0;

	if(paletteSize > 0){
		output->palette = *nextPalette;
		*nextPalette = output->palette + 1;
	} else {
		output->palette = 0xFF;
	}
	
	fread(&(output->w), sizeof(uint16_t), 1, f);
	fread(&(output->h), sizeof(uint16_t), 1, f);

	sizeInBytes -= 32u; //Skip header.

	fseek(f, 32, SEEK_SET);
	char* data = (char*)malloc(sizeInBytes);
	fread(data, sizeof(char), sizeInBytes, f);
	fclose(f);

	output->textureAlloc = pvr_mem_malloc(sizeInBytes);
	pvr_txr_load(data, output->textureAlloc, sizeInBytes);
	free(data);
	// Texture pointer might be offset compared to allocation becuase of compression codebook, stored just before the texture data.
	output->texture = output->textureAlloc;
	if(codebookSize > 0){
		output->texture -= 2048;
		output->texture += codebookSize * 8;
	}
	
	if((paletteSize > 0) && (palettePath != NULL)){
		// Open the palette
		FILE *fpal = fopen(palettePath, "rb");
		
		fread(&fourCC[0], sizeof(char), 4, fpal);
		
		uint32_t palSize;
		fread(&palSize, sizeof(uint32_t), 1, fpal);

		uint32_t paletteOffset = output->palette * 256;
		for(unsigned i = 0; i < palSize; i++) {
			uint32_t packedColor; 
			fread(&packedColor, sizeof(uint32_t), 1, fpal);
			// ARGB8888 to RGB565
			uint16_t r = (packedColor >> 16u) & 0xFF;
			uint16_t g = (packedColor >>  8u) & 0xFF;
			uint16_t b = (packedColor >>  0u) & 0xFF;
			// Truncate
			r >>= 3u;
			g >>= 2u;
			b >>= 3u;
			// Pack in lower 16bits
			uint32_t color = (r << 11u) | (g << 5u) | b;
			pvr_set_pal_entry(i + paletteOffset, color );
		}

		fclose(fpal);
	}
}


void initScene(Scene* scene){
	uint8_t nextPalette = 0;

	scene->light.x = -1.0f;
	scene->light.y = 1.0f;
	scene->light.z = 1.0f;
	normalize3(&(scene->light));

	scene->count = 3 + 6; // 3 objects, 6 sky faces
	scene->objects = (Object*)calloc(scene->count, sizeof(Object));
	
	scene->objects[0].vertices = &vertices_monkey[0];
	scene->objects[0].normals = &normals_monkey[0];
	scene->objects[0].uvs = &texcoords_monkey[0];
	scene->objects[0].indices = &points_monkey[0];
	scene->objects[0].vCount = vertex_count_monkey;
	scene->objects[0].iCount = points_count_monkey;

	scene->objects[0].verticesShadow = &vertices_monkey_shadow[0];
	scene->objects[0].normalsShadow = &normals_monkey_shadow[0];
	scene->objects[0].indicesShadow = &points_monkey_shadow[0];
	scene->objects[0].splitsShadow = &splits_monkey_shadow[0];
	scene->objects[0].vCountShadow = vertex_count_monkey_shadow;
	scene->objects[0].iCountShadow = points_count_monkey_shadow;
	scene->objects[0].sCountShadow = splits_count_monkey_shadow;

	scene->objects[0].position.x = -1.5f;
	scene->objects[0].position.y = 0.f;
	scene->objects[0].position.z = 1.f;
	scene->objects[0].scale = 1.25f;
	scene->objects[0].angleY = 0.0f;
	scene->objects[0].angleZ = 0.0f;
	scene->objects[0].shininess = 5u;
	scene->objects[0].flags = LIT | SHADOW_CAST; // Avoid self shadowing
	loadTexture("/data/monkey.dt", "/data/monkey.dt.pal", &nextPalette, &(scene->objects[0].texture) );
	
	// Floor
	scene->objects[1].vertices = &vertices_plane[0];
	scene->objects[1].normals = &normals_plane[0];
	scene->objects[1].uvs = &texcoords_plane[0];
	scene->objects[1].indices = &points_plane[0];
	scene->objects[1].vCount = vertex_count_plane;
	scene->objects[1].iCount = points_count_plane;
	scene->objects[1].position.x = 0.f;
	scene->objects[1].position.y = -2.0f;
	scene->objects[1].position.z = 0.f; 
	scene->objects[1].scale = 10.0f;
	scene->objects[1].angleY = 0.0f;
	scene->objects[1].angleZ = 0.0f;
	scene->objects[1].shininess = 20u;
	scene->objects[1].flags = LIT | SHADOW_RECEIVE; // Receive dragon and monkey shadows
	loadTexture("/data/floor.dt", "/data/floor.dt.pal", &nextPalette, &(scene->objects[1].texture) );
	
	// Dragon
	scene->objects[2].vertices = &vertices_dragon[0];
	scene->objects[2].normals = &normals_dragon[0];
	scene->objects[2].uvs = &texcoords_dragon[0];
	scene->objects[2].indices = &points_dragon[0];
	scene->objects[2].vCount = vertex_count_dragon;
	scene->objects[2].iCount = points_count_dragon;
	scene->objects[2].verticesShadow = &vertices_dragon_shadow[0];
	scene->objects[2].normalsShadow = &normals_dragon_shadow[0];
	scene->objects[2].indicesShadow = &points_dragon_shadow[0];
	scene->objects[2].splitsShadow = &splits_dragon_shadow[0];
	scene->objects[2].vCountShadow = vertex_count_dragon_shadow;
	scene->objects[2].iCountShadow = points_count_dragon_shadow;
	scene->objects[2].sCountShadow = splits_count_dragon_shadow;

	scene->objects[2].position.x = 1.5f;
	scene->objects[2].position.y = 0.1f;
	scene->objects[2].position.z = -2.1f; 
	scene->objects[2].scale = 1.5f;
	scene->objects[2].angleY = 0.0f;
	scene->objects[2].angleZ = 0.0f;
	scene->objects[2].shininess = 40u;
	scene->objects[2].flags = LIT | SHADOW_CAST | SHADOW_RECEIVE; // Receive monkey shadow
	loadTexture("/data/dragon.dt", "/data/dragon.dt.pal", &nextPalette, &(scene->objects[2].texture) );
	
	// Skybox faces, all using the same geometry with different transformations and textures.
	const float offset = 199.0f; // Hide seams.
	vector_t facePositions[] = {
		{0.0f, 0.0f, -offset},
		{0.0f, 0.0f, offset},
		{offset, 0.0f, 0.f},
		{-offset, 0.0f, 0.f},
		{0.0f, offset, 0.f},
		{0.0f, -offset, 0.f},
	};

	float faceAnglesY[] = {
		0.0f, F_PI, F_PI*0.5f, -0.5f * F_PI, 0.0f, 0.0f
	};

	float faceAnglesZ[] = {
		0.0f, 0.0f, 0.0f, 0.0f, -F_PI * 0.5f, F_PI*0.5f
	};

	const char* faceNames[] = {
		"/data/cubemap_f.dt",
		"/data/cubemap_b.dt",
		"/data/cubemap_r.dt",
		"/data/cubemap_l.dt",
		"/data/cubemap_u.dt",
		"/data/cubemap_d.dt",
	};

	for(unsigned int i = 0; i < 6; ++i)
	{
		Object* face = &scene->objects[3+i];
		face->vertices = &vertices_sky_side[0];
		face->normals = &normals_sky_side[0];
		face->uvs = &texcoords_sky_side[0];
		face->indices = &points_sky_side[0];
		face->vCount = vertex_count_sky_side;
		face->iCount = points_count_sky_side;
		face->position = facePositions[i];
		face->scale = 20.0f;
		face->angleY = faceAnglesY[i];
		face->angleZ = faceAnglesZ[i];
		face->shininess = 0u;
		face->flags = NONE; // Sky is unlit.
		loadTexture(faceNames[i], NULL, &nextPalette, &(face->texture) );
	}
	
	// Counter for temporary allocations.
	scene->maxVertexCount = 0;
	for(unsigned int i = 0; i < scene->count; ++i){
		scene->maxVertexCount = MAX(scene->maxVertexCount, scene->objects[i].vCount);
	}

}

void updateScene(Scene* scene, float time){
	// Update monkey head angle
	Object* monkey =  &scene->objects[0];
	monkey->angleY -= 1.20f / 60.0f;
	if( monkey->angleY < 0.0f){
		monkey->angleY += 2.0f * F_PI;
	}
	// Oscillates light vertically
	scene->light.x = -1.0f;
	scene->light.y = 0.5f * fsin(time) + 1.0f;
	scene->light.z = 1.0f;
	normalize3(&scene->light);
}

void cleanupScene(Scene* scene){
	for(unsigned int i = 0; i < scene->count; ++i){
		pvr_mem_free(scene->objects[i].texture.textureAlloc);
	}
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
	cam->radius = 10.0f;
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


