const uint16_t points_count_plane = 6;

const uint16_t vertex_count_plane = 4;

uint16_t points_plane[6]  __attribute__((aligned(32)))= {
	0, 2, 1, 
	0, 1, 3, 
};

vec3f_t vertices_plane[4]  __attribute__((aligned(32)))= {
	{.x= -1.0000, .y=0.0000, .z=-1.0000},
	{.x= 1.0000, .y=0.0000, .z=1.0000},
	{.x= -1.0000, .y=0.0000, .z=1.0000},
	{.x= 1.0000, .y=0.0000, .z=-1.0000},
};

vec3f_t normals_plane[4] = {
	{.x=0.0000, .y=1.0000, .z=0.0000},
	{.x=0.0000, .y=1.0000, .z=0.0000},
	{.x=0.0000, .y=1.0000, .z=0.0000},
	{.x=0.0000, .y=1.0000, .z=0.0000},
};

float texcoords_plane[8] = {
	0.0000, 0.0000,
	1.0000, 1.0000,
	0.0000, 1.0000,
	1.0000, 0.0000,
};

