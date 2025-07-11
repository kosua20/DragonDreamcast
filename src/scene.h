#ifndef SCENE_H
#define SCENE_H

enum MaterialFlag {
	NONE = 0,
	LIT = 1u << 0u,
	SHADOW_RECEIVE = 1u << 1u,
	SHADOW_CAST = 1u << 2u
};

typedef struct Texture {
	pvr_ptr_t textureAlloc;
	pvr_ptr_t texture;
	uint16_t w;
	uint16_t h;
	uint8_t palette;
} Texture;

typedef struct Object {
	// Diffuse appearance
	Texture texture;
	
	// Geometry data
	vec3f_t* vertices;
	vec3f_t* normals;
	const float* uvs;
	const uint16_t* indices;
	uint32_t vCount;
	uint32_t iCount;

	// Shadowcaster geometry data (optional)
	vec3f_t* verticesShadow;
	vec3f_t* normalsShadow;
	const uint16_t* indicesShadow;
	uint32_t vCountShadow;
	uint32_t iCountShadow;
	const uint16_t* splitsShadow;
	uint32_t sCountShadow;

	// Pose
	vector_t position;
	float scale;
	float angleY;
	float angleZ;

	// Material
	uint8_t shininess;
	uint8_t flags;
	bool lit;
	bool shadowCasting;
	bool shadowReceiving;
} Object;

typedef struct Scene {
	Object * objects;
	uint16_t count;
	vector_t light;
	uint32_t maxVertexCount; // For temp allocations
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
