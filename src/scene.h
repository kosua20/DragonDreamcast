#ifndef SCENE_H
#define SCENE_H

typedef struct Texture {
	pvr_ptr_t textureAlloc;
	pvr_ptr_t texture;
	uint16_t w;
	uint16_t h;
	uint8_t palette;
} Texture;

typedef struct Object {
	Texture texture;
	
	vec3f_t* vertices;
	vec3f_t* normals;
	const float* uvs;
	const uint16_t* indices;

	vector_t position;
	float scale;
	float angleY;
	float angleZ;
	uint32_t vCount;
	uint32_t iCount;
	uint8_t shininess;
	bool lit;
	bool shadowCasting;
} Object;

typedef struct Scene {
	Object * objects;
	uint16_t count;
	vector_t light;
	uint32_t maxVertexCount;
} Scene;

typedef struct Camera {
	matrix_t proj __attribute__((aligned(32)));
	matrix_t viewProj __attribute__((aligned(32)));
	vector_t pos;
	vector_t lookAt;
	vector_t up;
	float angleHoriz;
	float angleVert;
	float radius;

} Camera;

void initScene(Scene* scene);

void updateScene(Scene* scene, float time);

void cleanupScene(Scene* scene);

void updateCamera(Camera* cam);

void initCamera(Camera* cam);

#endif
