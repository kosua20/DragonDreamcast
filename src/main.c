
#include <dc/fmath.h>
#include <dc/pvr.h>
#include <kos.h>
#include <stdio.h>
#include <stdlib.h>
#include <dc/matrix3d.h> 

#include "helpers.h"
#include "scene.h"

extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);

void update(Camera* camera, Scene* scene, float time){

	updateScene(scene, time);

	const float kAngleSpeed = 10.0f;
	const float kRadiusSpeed = 0.5f;

	// Loop over all controllers.
	bool changed = false;
	MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, state)
  
	if (abs(state->joyx) > 16){
		changed = true;
		camera->angleHoriz += (state->joyx / 32768.0f) * kAngleSpeed;
	}
	if (abs(state->joyy) > 16){
		changed = true;
		camera->angleVert -= (state->joyy / 32768.0f) * kAngleSpeed;
	}
	if (state->ltrig > 16){
		changed = true;
		camera->radius += (state->ltrig / 255.0f) * kRadiusSpeed;
	}
	if (state->rtrig > 16){
		changed = true;
		camera->radius -= (state->rtrig / 255.0f) * kRadiusSpeed;
	}
  
	MAPLE_FOREACH_END()

	// Refresh camera position if needed.
	if(changed ){
		if(camera->radius < 0.1f){
			camera->radius = 0.1f;
		}
		if(camera->angleHoriz > F_PI * 2.0f){
			camera->angleHoriz -= F_PI * 2.0f;
		}
		if(camera->angleHoriz < 0.f){
			camera->angleHoriz += F_PI * 2.0f;
		}
		if(camera->angleVert > F_PI * 0.5f){
			camera->angleVert = F_PI * 0.5f;
		}
		if(camera->angleVert < -F_PI * 0.5f){
			camera->angleVert = -F_PI * 0.5f;
		}
		updateCamera(camera);
	}

}


void render(Camera* camera, Scene* scene, vec3f_t* scratchVertices) {
	


	// General camera transformation    
	for(uint8_t objectId = 0; objectId < scene->count; ++objectId){
		const Object* obj = &(scene->objects[objectId]);
		
		// Apply local transform
		mat_load(&camera->viewProj); 
		mat_translate(obj->position.x, obj->position.y, obj->position.z);
		// Models are flipped along X.
		mat_scale(-obj->scale, obj->scale, obj->scale);
		mat_rotate_y(obj->angle);

		// Transform object vertices into scratch buffer.
		mat_transform((vector_t*)obj->vertices, (vector_t*)scratchVertices, obj->vCount, sizeof(vec3f_t));
	   
		// Transform light dir in local space
		vector_t localLight;
		if(obj->lit){
			mat_identity();
			// No translation for light direction
			mat_rotate_y(-obj->angle);
			localLight = scene->light; 
			mat_trans_single3(localLight.x, localLight.y, localLight.z);
			// Norm *should* be preserved by transformation
			normalize3(&localLight);
		}

		// Prepare state for draw call
		// Init context
		pvr_poly_cxt_t cxt;
		pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED | PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(obj->tPalette), 
		//pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565,
			obj->tSide, obj->tSide, obj->texture, PVR_FILTER_NONE);
		cxt.gen.culling = PVR_CULLING_CW; 
		cxt.gen.shading = PVR_SHADE_GOURAUD;
		cxt.gen.specular = PVR_SPECULAR_DISABLE; //TODO
		cxt.fmt.color = PVR_CLRFMT_INTENSITY;
		cxt.depth.comparison = PVR_DEPTHCMP_GREATER;
		cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
		cxt.txr.mipmap = PVR_MIPMAP_DISABLE;
		cxt.txr.mipmap_bias = PVR_MIPBIAS_NORMAL;
		// Generate header from state.
		pvr_poly_hdr_t hdr;
		pvr_poly_compile(&hdr, &cxt);
		pvr_prim(&hdr, sizeof(hdr));

		// Init draw state
		pvr_dr_state_t drs;
		pvr_dr_init(&drs);
		
		for(uint32_t tId = 0; tId < obj->iCount; tId += 3){  

			for(uint8_t vId = 0; vId < 3; ++vId) {
				uint16_t index = obj->indices[tId + vId];
				vec3f_t* vProj = &scratchVertices[index];
				// Create hardware vert
				pvr_vertex_t* v = pvr_dr_target(drs);
				// Close strip if needed.
				v->flags = (vId == 2) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
				v->x = vProj->x;
				v->y = vProj->y;
				v->z = vProj->z;
				v->u = obj->uvs[2 * index + 0];
				v->v = obj->uvs[2 * index + 1];
				float diffuse = 1.f;
				float specular = 0.f;
				if(obj->lit){
					// Get the normal
					vec3f_t* n = &obj->normals[index];
					float dotNL = n->x * localLight.x + n->y * localLight.y + n->z * localLight.z;
					// Diffuse
					diffuse = clamp(dotNL, 0.0f, 1.0f);
					// Ambient
					diffuse += 0.1f;
					// Specular
					// TODO
					specular = clamp(-dotNL, 0.0f, 1.0f);
				};
				v->argb = *(uint32_t*)(&diffuse);
				v->oargb = *(uint32_t*)(&specular);
				pvr_dr_commit(v);
			}
		   
		} 
		pvr_dr_finish(); 
	}
}

int main(int argc, char **argv) {

	// Mount romdisk
	fs_romdisk_mount("/data", romdisk, 1);


	pvr_init_params_t params = {
		/* Enable opaque polygons with size 16 */
		{ PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0 },

		/* Vertex buffer size 512K */
		512 * 1024,
		0, // No DMA
		0, //  No FSAA
		0  // Translucent Autosort enabled.
	};

	// Init PVR.
	if(pvr_init(&params) < 0){
		return -1;
	}
	pvr_set_bg_color(0.2f, 0.0f, 0.4f);
	pvr_set_pal_format(PVR_PAL_RGB565);
	// Camera setup
	Camera camera;
	initCamera(&camera);
	
	// Objects
	Scene scene;
	initScene(&scene);

	vec3f_t* scratchVerts = memalign(32, sizeof(vec3f_t) * scene.maxVertexCount);
	uint32_t frame = 0;
	for(;;) {

		float time = (float)frame/60.0f;
		update(&camera, &scene, time);

		pvr_wait_ready();
		pvr_scene_begin();
		pvr_list_begin(PVR_LIST_OP_POLY);

		render(&camera, &scene, scratchVerts);

		pvr_list_finish();
		pvr_scene_finish();
		++frame;
	}

	free(scratchVerts);
	cleanupScene(&scene);

	// Cleanup
	pvr_shutdown();
	vid_shutdown();

	return 0;
}

