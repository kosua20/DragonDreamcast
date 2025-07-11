const uint16_t points_count_plane = 96;

const uint16_t vertex_count_plane = 25;

const uint16_t splits_count_plane = 1;

uint16_t points_plane[96]  __attribute__((aligned(32)))= {
	0, 1, 2, 
	0, 2, 3, 
	4, 5, 6, 
	7, 8, 4, 
	3, 9, 7, 
	4, 8, 5, 
	8, 10, 5, 
	7, 9, 8, 
	9, 11, 8, 
	8, 11, 10, 
	11, 12, 10, 
	3, 2, 9, 
	2, 13, 9, 
	9, 13, 11, 
	13, 14, 11, 
	11, 14, 12, 
	14, 15, 12, 
	14, 16, 15, 
	13, 17, 14, 
	2, 18, 13, 
	14, 17, 16, 
	17, 19, 16, 
	13, 18, 17, 
	18, 20, 17, 
	17, 20, 19, 
	20, 21, 19, 
	2, 1, 18, 
	1, 22, 18, 
	18, 22, 20, 
	22, 23, 20, 
	20, 23, 21, 
	23, 24, 21
};

uint16_t splits_plane[1]  __attribute__((aligned(32)))= {
0};

vec3f_t vertices_plane[25]  __attribute__((aligned(32)))= {
	{.x= 1.0000, .y=-0.0000, .z=-1.0000},
	{.x= 1.0000, .y=-0.0000, .z=-0.5000},
	{.x= 0.5000, .y=-0.0000, .z=-0.5000},
	{.x= 0.5000, .y=-0.0000, .z=-1.0000},
	{.x= -0.5000, .y=-0.0000, .z=-1.0000},
	{.x= -1.0000, .y=-0.0000, .z=-0.5000},
	{.x= -1.0000, .y=-0.0000, .z=-1.0000},
	{.x= -0.0000, .y=-0.0000, .z=-1.0000},
	{.x= -0.5000, .y=-0.0000, .z=-0.5000},
	{.x= -0.0000, .y=-0.0000, .z=-0.5000},
	{.x= -1.0000, .y=0.0000, .z=0.0000},
	{.x= -0.5000, .y=0.0000, .z=0.0000},
	{.x= -1.0000, .y=0.0000, .z=0.5000},
	{.x= -0.0000, .y=0.0000, .z=0.0000},
	{.x= -0.5000, .y=0.0000, .z=0.5000},
	{.x= -1.0000, .y=0.0000, .z=1.0000},
	{.x= -0.5000, .y=0.0000, .z=1.0000},
	{.x= -0.0000, .y=0.0000, .z=0.5000},
	{.x= 0.5000, .y=0.0000, .z=0.0000},
	{.x= -0.0000, .y=0.0000, .z=1.0000},
	{.x= 0.5000, .y=0.0000, .z=0.5000},
	{.x= 0.5000, .y=0.0000, .z=1.0000},
	{.x= 1.0000, .y=0.0000, .z=0.0000},
	{.x= 1.0000, .y=0.0000, .z=0.5000},
	{.x= 1.0000, .y=0.0000, .z=1.0000},
};

vec3f_t normals_plane[25] = {
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
	{.x=0.0000, .y=1.0000, .z=-0.0000},
};

float texcoords_plane[50] = {
	0.0000, 0.0000,
	0.0000, 0.2500,
	0.2500, 0.2500,
	0.2500, 0.0000,
	0.7500, 0.0000,
	1.0000, 0.2500,
	1.0000, 0.0000,
	0.5000, 0.0000,
	0.7500, 0.2500,
	0.5000, 0.2500,
	1.0000, 0.5000,
	0.7500, 0.5000,
	1.0000, 0.7500,
	0.5000, 0.5000,
	0.7500, 0.7500,
	1.0000, 1.0000,
	0.7500, 1.0000,
	0.5000, 0.7500,
	0.2500, 0.5000,
	0.5000, 1.0000,
	0.2500, 0.7500,
	0.2500, 1.0000,
	0.0000, 0.5000,
	0.0000, 0.7500,
	0.0000, 1.0000,
};

