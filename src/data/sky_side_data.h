const uint16_t points_count_sky_side = 6;

const uint16_t vertex_count_sky_side = 4;

uint16_t points_sky_side[6]  __attribute__((aligned(32)))= {
	0, 1, 2, 
	2, 3, 0
};

vec3f_t vertices_sky_side[4]  __attribute__((aligned(32)))= {
	{.x= -10.0000, .y=10.0000, .z=0.0000},
	{.x= 10.0000, .y=10.0000, .z=0.0000},
	{.x= 10.0000, .y=-10.0000, .z=0.0000},
	{.x= -10.0000, .y=-10.0000, .z=0.0000},
};

vec3f_t normals_sky_side[4] = {
	{.x=-0.0000, .y=0.0000, .z=1.0000},
	{.x=-0.0000, .y=0.0000, .z=1.0000},
	{.x=-0.0000, .y=0.0000, .z=1.0000},
	{.x=-0.0000, .y=0.0000, .z=1.0000},
};

float texcoords_sky_side[8] = {
	1.0000, 0.0000,
	0.0000, 0.0000,
	0.0000, 1.0000,
	1.0000, 1.0000,
};

