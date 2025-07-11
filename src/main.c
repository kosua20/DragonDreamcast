
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

	pvr_list_begin(PVR_LIST_OP_POLY);


	for(uint8_t objectId = 0; objectId < scene->count; ++objectId){
		const Object* obj = &(scene->objects[objectId]);

		// Compute diffuse and specular lighting once for each vertex, if needed..
		uint32_t* packedLightings = (uint32_t*)(((vec3f_t*)scratchVertices) + obj->vCount);		
		vec3f_t* srcVertices = obj->vertices;

		if(obj->lit){

			// No translation for light direction
			mat_identity();
			mat_rotate_x(-obj->angleZ);
			mat_rotate_y(-obj->angleY);
			vector_t localLight = scene->light; 
			mat_trans_single3(localLight.x, localLight.y, localLight.z);
			// Norm *should* be preserved by transformation
			normalize3(&localLight);

			// Reuse the current matrix, adding the inverse scale and translate this time.
			mat_scale(1.f/obj->scale, 1.f/obj->scale, 1.f/obj->scale);
			mat_translate(-obj->position.x, -obj->position.y, -obj->position.z);
			vector_t localCamera = camera->pos;
			mat_trans_single3(localCamera.x, localCamera.y, localCamera.z);
			
			for(uint16_t i = 0; i < obj->vCount; ++i){
				// Get the normal
				vec3f_t* n = &obj->normals[i];
				const float dotNL = n->x * localLight.x + n->y * localLight.y + n->z * localLight.z;
				
				// Diffuse
				float diffuse = clamp(dotNL, 0.f, 1.0f);
				// Ambient
				diffuse += 0.1f;
				unsigned char diffClamped = (unsigned char)(clamp(diffuse, 0.f, 1.0f) * 255.f);
				packedLightings[2 * i] = 0xFF000000 | (diffClamped << 16u) | (diffClamped << 8u) | diffClamped;
				
				// Specular
				unsigned char specClamped = 0u;
				// Only if diffuse positive.
				if(dotNL >= 0.f){
					// Reflect light dir around normal
					float dotNL2 = 2.f * dotNL;
					vector_t r;
					r.x = dotNL2 * n->x - localLight.x;
					r.y = dotNL2 * n->y - localLight.y;
					r.z = dotNL2 * n->z - localLight.z;
					normalize3(&r);
					// Compute vector from surface to camera.
					vec3f_t* vObj = &obj->vertices[i];
					vector_t v; 
					v.x = localCamera.x - vObj->x;
					v.y = localCamera.y - vObj->y;
					v.z = localCamera.z - vObj->z;
					normalize3(&v);
	
					float dotRV = (r.x * v.x + r.y * v.y + r.z * v.z);
					dotRV = MAX(dotRV, 0.f);
					float specular = 1.f;
					for(uint8_t sId = 0; sId < obj->shininess; ++sId){
						specular *= dotRV;
					}
					specClamped = (unsigned char)(clamp(specular, 0.f, 1.0f) * 255.f);
				}
				packedLightings[2 * i + 1] =  0xFF000000 | (specClamped << 16u) | (specClamped << 8u) | specClamped;
			
			}
		}

		// Apply local transform
		mat_load(&camera->viewProj); 
		mat_translate(obj->position.x, obj->position.y, obj->position.z);
		mat_scale(obj->scale, obj->scale, obj->scale);
		mat_rotate_y(obj->angleY);
		mat_rotate_x(obj->angleZ);

		// Transform object vertices into scratch buffer.
		mat_transform((vector_t*)srcVertices, (vector_t*)scratchVertices, obj->vCount, sizeof(vec3f_t));

		// Prepare state for draw call
		// Init context
		const Texture* tex = &(obj->texture);
		uint32_t formatFlags = 0u;
		if(tex->palette != 0xFF){
			formatFlags = PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL(tex->palette);
		} else {
			formatFlags = PVR_TXRFMT_RGB565;
		}
		pvr_poly_cxt_t cxt;
		pvr_poly_hdr_t hdr;

		pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_VQ_ENABLE | PVR_TXRFMT_TWIDDLED | formatFlags, tex->w, tex->h, tex->texture, PVR_FILTER_BILINEAR);
		cxt.gen.culling = PVR_CULLING_CW; 
		cxt.gen.shading = PVR_SHADE_GOURAUD;
		cxt.gen.specular = PVR_SPECULAR_ENABLE;
		if(obj->shadowReceiving){
			cxt.gen.modifier_mode = PVR_MODIFIER_CHEAP_SHADOW;
			cxt.fmt.modifier = PVR_MODIFIER_ENABLE;
		}
		cxt.fmt.color = PVR_CLRFMT_ARGBPACKED; // PVR_CLRFMT_INTENSITY seemed incompatible with specular.
		cxt.depth.comparison = PVR_DEPTHCMP_GREATER;
		cxt.depth.write = PVR_DEPTHWRITE_ENABLE;
		cxt.txr.mipmap = PVR_MIPMAP_DISABLE;
		cxt.txr.mipmap_bias = PVR_MIPBIAS_NORMAL;
		pvr_poly_compile(&hdr, &cxt);

		// Generate header from state.
		pvr_prim(&hdr, sizeof(hdr));

		// Init draw state
		pvr_dr_state_t drs;
		pvr_dr_init(&drs);

		for(uint32_t tId = 0; tId < obj->iCount; tId += 3){

			uint16_t indices[3];  
			vec3f_t* vProjs[3];
			bool skipPrimitive = false;  
			for(uint8_t vId = 0; vId < 3; ++vId) {
				indices[vId] = obj->indices[tId + vId];
				vProjs[vId] = &scratchVertices[indices[vId]];
				if(vProjs[vId]->z < 0.f)
					skipPrimitive = true;
			}
			if(skipPrimitive)
				continue;

			for(uint8_t vId = 0; vId < 3; ++vId) {
				vec3f_t* vProj = vProjs[vId];
				uint16_t index2 = 2 * indices[vId];

				// Create hardware vert
				pvr_vertex_t* v = pvr_dr_target(drs);
				// Close strip if needed.
				v->flags = (vId == 2) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
				v->x = vProj->x;
				v->y = vProj->y;
				v->z = vProj->z;
				v->u = obj->uvs[index2];
				v->v = obj->uvs[index2 + 1];
				uint32_t packedDiffuse = 0xFFFFFFFF;
				uint32_t packedSpecular = 0xFF000000;
				if(obj->lit){
					packedDiffuse = packedLightings[index2];
					packedSpecular = packedLightings[index2 + 1];
				}
				v->argb  = packedDiffuse;
				v->oargb = packedSpecular;
				pvr_dr_commit(v);
			}
		} 
		pvr_dr_finish(); 
	}
	pvr_list_finish();
	
	// Second pass for shadows
	// These headers are simpler and can be shared.
	pvr_mod_hdr_t hdrMod;
	pvr_mod_hdr_t hdrModEnd;
	pvr_mod_compile(&hdrMod, PVR_LIST_OP_MOD, PVR_MODIFIER_OTHER_POLY, PVR_CULLING_NONE);
	pvr_mod_compile(&hdrModEnd, PVR_LIST_OP_MOD, PVR_MODIFIER_INCLUDE_LAST_POLY, PVR_CULLING_NONE);
	// These won't change either.
	pvr_modifier_vol_t mod;	
	mod.flags = PVR_CMD_VERTEX_EOL;
	mod.d1 = mod.d2 = mod.d3 = mod.d4 = mod.d5 = mod.d6 = 0;

	pvr_list_begin(PVR_LIST_OP_MOD);

	for(unsigned int objectId = 0; objectId < scene->count; ++objectId){
		
		const Object* obj = &(scene->objects[objectId]);
		if(!obj->shadowCasting){
			continue;
		}
		
		// Compute diffuse and specular lighting once for each vertex, if needed..
		vec3f_t* srcVertices = obj->verticesShadow;
		if(obj->lit){
			memcpy(scratchVertices, srcVertices, sizeof(vec3f_t) * obj->vCountShadow);
			srcVertices = scratchVertices;

			// No translation for light direction
			mat_identity();
			mat_rotate_x(-obj->angleZ);
			mat_rotate_y(-obj->angleY);
			vector_t localLight = scene->light; 
			mat_trans_single3(localLight.x, localLight.y, localLight.z);
			// Norm *should* be preserved by transformation
			normalize3(&localLight);
			
			for(uint16_t i = 0; i < obj->vCountShadow; ++i){
				// Get the normal
				vec3f_t* n = &obj->normalsShadow[i];
				const float dotNL = n->x * localLight.x + n->y * localLight.y + n->z * localLight.z;
				if(dotNL < -0.01f){
					scratchVertices[i].x -= 10.f * localLight.x;
					scratchVertices[i].y -= 10.f * localLight.y;
					scratchVertices[i].z -= 10.f * localLight.z;
				}
			}
		}

		// Apply local transform
		mat_load(&camera->viewProj); 
		mat_translate(obj->position.x, obj->position.y, obj->position.z);
		mat_scale(obj->scale, obj->scale, obj->scale);
		mat_rotate_y(obj->angleY);
		mat_rotate_x(obj->angleZ);

		// Transform object vertices into scratch buffer.
		mat_transform((vector_t*)srcVertices, (vector_t*)scratchVertices, obj->vCountShadow, sizeof(vec3f_t));

		// Apply header.
		for(uint16_t split = 0; split < obj->sCountShadow; ++split)
		{
			uint16_t indexStart = obj->splitsShadow[split];
			uint16_t indexAfterEnd = (split == obj->sCountShadow - 1) ? obj->iCountShadow : obj->splitsShadow[split + 1];
			uint16_t indexOfLastTri = indexAfterEnd - 3;

			pvr_prim(&hdrMod, sizeof(hdrMod));
			for(uint16_t tId = indexStart; tId < indexAfterEnd; tId += 3u)
			{
				if(tId == indexOfLastTri){
					pvr_prim(&hdrModEnd, sizeof(hdrModEnd));
				}
				// Populate primitive
				vec3f_t* a = &scratchVertices[obj->indicesShadow[tId ]];
				mod.ax = a->x;
	    		mod.ay = a->y;
	    		mod.az = a->z;
				vec3f_t* b = &scratchVertices[obj->indicesShadow[tId + 1]];
	    		mod.bx = b->x;
	    		mod.by = b->y;
	    		mod.bz = b->z;
				vec3f_t* c = &scratchVertices[obj->indicesShadow[tId + 2]];
	    		mod.cx = c->x;
	    		mod.cy = c->y;
	    		mod.cz = c->z;
	    		pvr_prim(&mod, sizeof(mod));
			}
		}
	}
	pvr_list_finish();
}

int main(int argc, char **argv) {

	// Mount romdisk
	fs_romdisk_mount("/data", romdisk, 1);


	pvr_init_params_t params = {
		/* Enable opaque polygons with size 16 */
		{ PVR_BINSIZE_16, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0 },

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
	pvr_set_shadow_scale(true, 0.5f);
	pvr_set_zclip(0.0001f);
	// Camera setup
	Camera camera;
	initCamera(&camera);
	
	// Objects
	Scene scene;
	initScene(&scene);

	vec3f_t* scratchVerts = memalign(32, (sizeof(vec3f_t) + 2 * sizeof(uint32_t)) * scene.maxVertexCount);
	uint32_t frame = 0;
	for(;;) {

		float time = (float)frame/60.0f;
		update(&camera, &scene, time);

		pvr_wait_ready();
		pvr_scene_begin();
		
		render(&camera, &scene, scratchVerts);

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

