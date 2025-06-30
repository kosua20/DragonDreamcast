
#include <dc/fmath.h>
#include <dc/pvr.h>
#include <kos.h>
#include <stdio.h>
#include <stdlib.h>

#include <dc/matrix.h>
#include <dc/matrix3d.h> 

#include "data.h"

extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);


typedef struct Object {

    vec3f_t* verts;
    const uint16_t* faces;
    const float* uvs;

    vector_t pos;
    float scale;
    float angle;
    uint32_t vCount;
    uint32_t fCount;
} Object;


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


void updateCamera(Camera* cam){
    cam->pos.x = cam->radius * fcos(cam->angleHoriz) * fcos(cam->angleVert);
    cam->pos.y = cam->radius * fsin(cam->angleVert);
    cam->pos.z = cam->radius * fsin(cam->angleHoriz) * fcos(cam->angleVert);
    // Update view matrix
    mat_load(&cam->proj);
    mat_lookat(&cam->pos, &cam->lookAt, &cam->up);
    mat_store(&cam->viewProj);  
}

void setVector(vector_t* v, float x, float y, float z, float w){
    v->x = x;
    v->y = y;
    v->z = z;
    v->w = w;
}


void update(Camera* camera){

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


void render(Camera* camera, Object* objects, uint8_t objectsCount, vec3f_t* scratchVertices) {
    // Update monkey head angle
    objects[2].angle += 0.05f;
    if(objects[2].angle > 2.0f * F_PI){
        objects[2].angle -= 2.0f * F_PI;
    }

    // Init context
    pvr_poly_cxt_t cxt;
    pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
    cxt.gen.culling = PVR_CULLING_NONE; 
    // Generate header from state.
    pvr_poly_hdr_t hdr;
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));
    // Init draw state
    pvr_dr_state_t drs;
    pvr_dr_init(&drs);

    // General camera transformation    
    for(uint8_t objectId = 0; objectId < objectsCount; ++objectId){
        const Object* obj = &objects[objectId];
        
        // Apply local transform
        mat_load(&camera->viewProj); 
        mat_translate(obj->pos.x, obj->pos.y, obj->pos.z);
        // Models are flipped along X.
        mat_scale(-obj->scale, obj->scale, obj->scale);
        mat_rotate_y(obj->angle);

        // Transform object vertices into scratch

        mat_transform((vector_t*)obj->verts, (vector_t*)scratchVertices, obj->vCount, sizeof(vec3f_t));
       
        for(uint32_t tId = 0; tId < obj->fCount; tId += 3){  

            for(uint8_t vId = 0; vId < 3; ++vId) {
                uint16_t index = obj->faces[tId + vId];
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
                v->argb = 0xFF000000 | (unsigned char)(v->u * 255.0f) << 16 | (unsigned char)(v->v * 255.0f) << 8;
                v->oargb = 0;
                pvr_dr_commit(v);
            }
           
        }
    }
    pvr_dr_finish(); 
}




int main(int argc, char **argv) {

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

    // Camera setup
    Camera camera;
    {
        camera.angleHoriz = F_PI * 0.5f;
        camera.angleVert = F_PI * 0.15f;
        camera.radius = 7.0f;
        setVector(&camera.pos, 0.0f, 0.0f, 0.0f, 1.0f);
        setVector(&camera.lookAt, 0.0f, 0.0f, 0.0f, 1.0f);
        setVector(&camera.up , 0.0f, -1.0f, 0.0f, 0.f);
        // Setup projection matrix once and for all.
        mat_identity();
        const float fov = 45.0f * F_PI / 180.0f;
        float cotFov2 = 1.0f / ftan(fov * 0.5f);
        mat_perspective(320.0f, 240.0f, cotFov2, 0.1f, +10.0f);
        mat_store(&camera.proj);
        updateCamera(&camera);
    }

    // Objects
    struct Object objects[4] = {
        { &vertices_plane[0], &points_plane[0], &texcoords_plane[0], {0.f, -1.5f, 0.0f}, 10.0f, 0.f, vertex_count_plane, points_count_plane},
        { &vertices_dragon[0], &points_dragon[0], &texcoords_dragon[0], {1.0f, 0.0f, -1.0f}, 1.0f, 0.f, vertex_count_dragon, points_count_dragon},
        { &vertices_monkey[0], &points_monkey[0], &texcoords_monkey[0], {-1.0f, 0.0f, 1.0f }, 1.0f, 0.f, vertex_count_monkey, points_count_monkey},
        { &vertices_cube[0], &points_cube[0], &texcoords_cube[0], {0.f, 0.f, 0.f}, 20.0f, 0.f, vertex_count_cube, points_count_cube},
    };

    uint32_t maxVertexCount = 0;
    for(unsigned int i = 0; i < sizeof(objects)/sizeof(objects[0]); ++i){
        if(objects[i].vCount > maxVertexCount)
            maxVertexCount = objects[i].vCount;
    }

    vec3f_t* scratchVerts = memalign(32, sizeof(vec3f_t) * maxVertexCount);

    for(;;) {

        update(&camera);

        pvr_wait_ready();
        pvr_scene_begin();
        pvr_list_begin(PVR_LIST_OP_POLY);

        render(&camera, objects, sizeof(objects)/sizeof(objects[0]), scratchVerts);

        pvr_list_finish();
        pvr_scene_finish();
    }

    free(scratchVerts);

    // Cleanup
    pvr_shutdown();
    vid_shutdown();

    return 0;
}

