#include "main.h"
#include "Utilities.h"

#include <xmmintrin.h>
#include <mmintrin.h>
#include <emmintrin.h>
#include <new.h>  

//AABB - axis-aligned bounding box
//OBB - oriented Bounding Box


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------data & settings
//------------screen
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900
#define SCREEN_DEPTH 32

bool  g_bFullScreen = TRUE;
HWND  g_hWnd;
RECT  g_rRect;
HDC   g_hDC;
HGLRC g_hRC;
HINSTANCE g_hInstance;

int window_width;
int window_height;


//------------demo settings
bool use_multithreading = false;
const int num_workers = 4;

bool use_gpu_culling = false;
bool enable_rendering_objects = true;
bool culling_enabled = true;

const bool only_culling_measurements = false;

enum CULLING_MODE
{
	SIMPLE_SPHERES,
	SIMPLE_AABB,
	SIMPLE_OBB,

	SSE_SPHERES,
	SSE_AABB,
	SSE_OBB
};
CULLING_MODE culling_mode = SIMPLE_SPHERES;

//------------time
double total_time = 0.0;

//------------camera
CCamera camera;
CFrustum frustum;
float zNear = 0.1f;
float zFar = 100.f;

mat4 camera_view_matrix, camera_proj_matrix, camera_view_proj_matrix, saved_inv_view_proj_matrix;

//------------area
const float AREA_SIZE = 20.f;
const float half_box_size = 0.05f;
const vec3 box_max = vec3(half_box_size, half_box_size, half_box_size);
const vec3 box_min = -vec3(half_box_size, half_box_size, half_box_size);
const vec3 box_half_size = vec3(half_box_size, half_box_size, half_box_size);
const float bounding_radius = sqrtf(3.f) * half_box_size;

//------------sse
#define sse_align 16
#define ALIGN_SSE __declspec( align( sse_align ) )

ALIGN_SSE struct mat4_sse
{
	__m128 col0;
	__m128 col1;
	__m128 col2;
	__m128 col3;

	mat4_sse() {}
	mat4_sse(mat4 &m)
	{
		set(m);
	}
	inline void set(mat4 &m)
	{
		col0 = _mm_loadu_ps(&m.mat[0]);
		col1 = _mm_loadu_ps(&m.mat[4]);
		col2 = _mm_loadu_ps(&m.mat[8]);
		col3 = _mm_loadu_ps(&m.mat[12]);
	}
};

//------------scene geometry
ALIGN_SSE struct BSphere
{
	vec3 pos;
	float r;
};

ALIGN_SSE struct AABB
{
	vec4 box_min;
	vec4 box_max;
};


const int MAX_SCENE_OBJECTS = 100000;
BSphere *sphere_data = NULL;
AABB *aabb_data = NULL;
int *culling_res = NULL;
mat4_sse *sse_obj_mat = NULL;
mat4 *obj_mat = NULL;


//------------geometry rendering data

GLuint wire_box_vao_id = -1;
GLuint wire_box_vbo_id = -1;
GLuint wire_box_ibo_id = -1;


GLuint ground_vao_id = -1;
GLuint ground_vbo_id = -1;
GLuint ground_ibo_id = -1;

GLuint geometry_vao_id = -1;
GLuint geometry_vbo_id = -1;
GLuint geometry_ibo_id = -1;

//tbo example https://gist.github.com/roxlu/5090067
GLuint dips_texture_buffer = -1;
GLuint dips_texture_buffer_tex = -1;

GLuint all_instances_data_vao = -1;
GLuint all_instances_data_vbo = -1;


vec4 instance_info[MAX_SCENE_OBJECTS * 2]; //pos + color
vec4 visible_instance_info[MAX_SCENE_OBJECTS * 2]; //just visible instances data
int num_visible_instances = MAX_SCENE_OBJECTS;


//------------shaders
Shader ground_shader;
Shader geometry_shader;
Shader show_frustum_shader;
Shader culling_shader;
GLuint num_visible_instances_query[2];

int frame_index = 0;


//------------multithreading
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------threads
class Worker
{
public:
	Worker();
	~Worker();

	//worker info
	HANDLE thread_handle;
	unsigned thread_id;
	volatile bool stop_work;
	HANDLE has_jobs_event;
	HANDLE jobs_finished_event;

	//make job
	void doJob();
	int first_processing_oject;
	int num_processing_ojects;
};

Worker workers[num_workers];
HANDLE thread_handles[num_workers];

void create_threads();
void threads_close();
void process_multithreading_culling();
void wate_multithreading_culling_done();


void cull_objects(int first_processing_oject, int num_processing_ojects);



//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------window stuff
void SizeOpenGLScreen(int width, int height)
{
	height = max(height, 1);
	window_width = width;
	window_height = height;
	glViewport(0, 0, width, height);
}


void CheckGLErrors()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
		fprintf(stderr, "OpenGL Error: %s\n", gluErrorString(err));
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------SSE base

//https://github.com/nsf/sseculling/blob/master/Core/Memory.cpp
template <typename T>
inline T *new_sse(size_t size)
{
	return static_cast<T*>(_aligned_malloc(sizeof(T)* size, sse_align));
}

template <typename T>
inline void delete_sse(T *ptr)
{
	_aligned_free(static_cast<void*>(ptr));
}

template <typename T>
inline T* new_sse_array(int array_size)
{
	T* arr = static_cast<T*>(_aligned_malloc(sizeof(T)* array_size, sse_align));
	for (size_t i = 0; i < size_t(array_size); i++)
		new (&arr[i]) T;
	return arr;
}

template <typename T>
inline void delete_sse_array(T *arr, size_t array_size)
{
	for (size_t i = 0; i < array_size; i++)
		arr[i].~T();//destroy all initialized elements
	_aligned_free(static_cast<void*>(arr));
}


template <typename T>
inline T *new_sse32(size_t size)
{
	return static_cast<T*>(_aligned_malloc(sizeof(T)* size, 32));
}



//http://stackoverflow.com/questions/38090188/matrix-multiplication-using-sse
//https://gist.github.com/rygorous/4172889

__forceinline __m128 sse_mat4_mul_vec4(mat4_sse &m, __m128 v)
{
	__m128 xxxx = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 yyyy = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 zzzz = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
	__m128 wwww = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));

	return _mm_add_ps(
		_mm_add_ps(_mm_mul_ps(xxxx, m.col0), _mm_mul_ps(yyyy, m.col1)),
		_mm_add_ps(_mm_mul_ps(zzzz, m.col2), _mm_mul_ps(wwww, m.col3))
	);
}

__forceinline void sse_mat4_mul(mat4_sse &dest, mat4_sse &m1, mat4_sse &m2)
{
	dest.col0 = sse_mat4_mul_vec4(m1, m2.col0);
	dest.col1 = sse_mat4_mul_vec4(m1, m2.col1);
	dest.col2 = sse_mat4_mul_vec4(m1, m2.col2);
	dest.col3 = sse_mat4_mul_vec4(m1, m2.col3);
}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------init



void generate_instances()
{
//allocate data
	sphere_data = new_sse_array<BSphere>(MAX_SCENE_OBJECTS);
	aabb_data = new_sse_array<AABB>(MAX_SCENE_OBJECTS);

	culling_res = new_sse<int>(MAX_SCENE_OBJECTS);
	memset(&culling_res[0], 0, sizeof(int) * MAX_SCENE_OBJECTS);

	sse_obj_mat = new_sse_array<mat4_sse>(MAX_SCENE_OBJECTS);
	memset(&sse_obj_mat[0], 0, sizeof(mat4_sse) * MAX_SCENE_OBJECTS);

	obj_mat = new mat4[MAX_SCENE_OBJECTS];

//generate instances data
	int i;
	vec3 pos;
	mat4 obj_transform_mat; //identity by default
	for (i = 0; i<MAX_SCENE_OBJECTS; i++)
	{
		//random position inside area
		pos = vec3(rnd(-1.f, 1.f)*AREA_SIZE, half_box_size * 0.95f, rnd(-1.f, 1.f)*AREA_SIZE);

		instance_info[i * 2 + 0] = vec4(pos, bounding_radius); //pos
		instance_info[i * 2 + 1] = vec4(rnd01(), rnd01(), rnd01(), 1.f); //color

		sphere_data[i].pos = pos;
		sphere_data[i].r = bounding_radius;

		aabb_data[i].box_min = vec4(pos - box_half_size, 1.f);
		aabb_data[i].box_max = vec4(pos + box_half_size, 1.f);

		obj_transform_mat.set_translation(pos);
		obj_mat[i] = obj_transform_mat;
		sse_obj_mat[i].set(obj_transform_mat);
	}
}


struct SimpleVertex
{
	vec3 p;
	vec3 n;
	vec2 uv;

	void init(vec3 in_p, vec3 in_n, vec2 in_uv)
	{
		p = in_p; n = in_n; uv = in_uv;
	};
};

void create_instance_geometry()
{
//simple box
	int i, j;
	SimpleVertex vertex_buffer[24];
	int index_buffer[36];

//separate attribs
	//verts
	vec3 box_iffset = vec3(0, 0, 0);
	vec3 box_verts[8] = {
		vec3(-half_box_size, -half_box_size, half_box_size),
		vec3(half_box_size, -half_box_size, half_box_size),
		vec3(half_box_size, half_box_size, half_box_size),
		vec3(-half_box_size, half_box_size, half_box_size),
		vec3(-half_box_size, -half_box_size, -half_box_size),
		vec3(half_box_size, -half_box_size, -half_box_size),
		vec3(half_box_size, half_box_size, -half_box_size),
		vec3(-half_box_size, half_box_size, -half_box_size),
	};
	for (i = 0; i < 8; i++)
		box_verts[i] += box_iffset;

	//indices
	int box_verts_i[24] = { 1, 5, 6, 2, 3, 7, 4, 0, 0, 4, 5, 1, 2, 6, 7, 3, 0, 1, 2, 3, 7, 6, 5, 4 };

	//normals
	vec3 box_face_normals[6] = {
		vec3(1, 0, 0),
		vec3(-1, 0, 0),
		vec3(0, -1, 0),
		vec3(0, 1, 0),
		vec3(0, 0, 1),
		vec3(0, 0, -1)
	};

	//uv
	vec2 uv[4] = { vec2(0.f,0.f), vec2(1.f,0.f), vec2(1.f,1.f), vec2(0.f,1.f) };

//combine to single buffer, combine vertex buffer
	for (i = 0; i<24; i++)
	{
		vertex_buffer[i].p = box_verts[box_verts_i[i]];
		vertex_buffer[i].n = box_face_normals[int(i / 4)];
		vertex_buffer[i].uv = uv[i % 4];
	}

//index buffer
	int box_face_indices[6] = { 0, 1, 2, 0, 2, 3 };
	for (i = 0; i<6; i++)
		for (j = 0; j<6; j++)
			index_buffer[i * 6 + j] = box_face_indices[j] + 4 * i;

//create vao
	RenderElementDescription desc;
	VboElement vbo_elements[3] = { 3,0,GL_FLOAT,   3,sizeof(vec3),GL_FLOAT,   2,sizeof(vec3)*2,GL_FLOAT };
	desc.init(sizeof(SimpleVertex), 24, (void*)&vertex_buffer[0], GL_STATIC_DRAW, 3, &vbo_elements[0]);

	create_render_element(geometry_vao_id, geometry_vbo_id, desc, true, geometry_ibo_id, 36, &index_buffer[0]);
}

void create_ground()
{
	SimpleVertex vertices[4];
	memset(&vertices[0], 0, sizeof(SimpleVertex) * 4);

	const vec3 groundNormal = vec3(0.f, 1.f, 0.f);
	vertices[0].init(vec3(-AREA_SIZE, 0.f, -AREA_SIZE), groundNormal, vec2(0.f, 0.f));
	vertices[1].init(vec3(AREA_SIZE, 0.f, -AREA_SIZE), groundNormal, vec2(1.f, 0.f));
	vertices[2].init(vec3(AREA_SIZE, 0.f, AREA_SIZE), groundNormal, vec2(1.f, 1.f));
	vertices[3].init(vec3(-AREA_SIZE, 0.f, AREA_SIZE), groundNormal, vec2(0.f, 1.f));

	int indices[6] = { 0, 1, 2, 0, 2, 3 };

	RenderElementDescription desc;
	VboElement vbo_elements[3] = { 3,0,GL_FLOAT,   3,sizeof(vec3),GL_FLOAT,   2,sizeof(vec3) * 2,GL_FLOAT };
	desc.init(sizeof(SimpleVertex), 4, (void*)&vertices[0], GL_STATIC_DRAW, 3, &vbo_elements[0]);

	create_render_element(ground_vao_id, ground_vbo_id, desc, true, ground_ibo_id, 6, &indices[0]);
}


vec4 gl_unit_cube[8] =
{
	vec4(-1,-1,-1, 1),
	vec4(1,-1,-1, 1),
	vec4(1,1,-1, 1),
	vec4(-1,1,-1, 1),

	vec4(-1,-1,1, 1),
	vec4(1,-1,1, 1),
	vec4(1,1,1, 1),
	vec4(-1,1,1, 1)
};

int unit_box_index_buffer[24] =
{
	0,1, 1,2, 2,3, 3,0,
	4,5, 5,6, 6,7, 7,4,
	0,4, 1,5, 2,6, 3,7
};

void create_cube_wire_box()
{
	RenderElementDescription desc;
	VboElement vbo_elements[3] = { 4,0,GL_FLOAT };
	desc.init(sizeof(vec4), 8, (void*)&gl_unit_cube[0], GL_STATIC_DRAW, 1, &vbo_elements[0]);
	create_render_element(wire_box_vao_id, wire_box_vbo_id, desc, true, wire_box_ibo_id, 24, &unit_box_index_buffer[0]);
}

void create_scene()
{
	create_ground();
	create_instance_geometry();
	create_cube_wire_box();

	generate_instances();

//create texture buffer which will contain visible instances data
	glGenBuffers(1, &dips_texture_buffer);
	glBindBuffer(GL_TEXTURE_BUFFER, dips_texture_buffer);
	glBufferData(GL_TEXTURE_BUFFER, MAX_SCENE_OBJECTS * 2 * sizeof(vec4), &instance_info[0], GL_STATIC_DRAW);
	glGenTextures(1, &dips_texture_buffer_tex);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

//for gpu culling, vbo with all instances data
	RenderElementDescription desc;
	VboElement vbo_elements[2] = { 4,0,GL_FLOAT,   4,sizeof(vec4),GL_FLOAT };
	desc.init(sizeof(vec4)*2, MAX_SCENE_OBJECTS, (void*)&instance_info[0], GL_STATIC_DRAW, 2, &vbo_elements[0]);
	create_render_element(all_instances_data_vao, geometry_vbo_id, desc, true, geometry_ibo_id, 0, NULL);
}



void init_shaders()
{
	ground_shader.programm_id = init_shader("ground_vs", "ground_ps");
	ground_shader.add_uniform("ModelViewProjectionMatrix", 16, &camera_view_proj_matrix.mat[0]);

	geometry_shader.programm_id = init_shader("geometry_vs", "geometry_ps");
	geometry_shader.add_uniform("ModelViewProjectionMatrix", 16, &camera_view_proj_matrix.mat[0]);

	show_frustum_shader.programm_id = init_shader("show_frustum_vs", "show_frustum_ps");
	show_frustum_shader.add_uniform("PrevInvModelViewProjectionMatrix", 16, &saved_inv_view_proj_matrix.mat[0]);
	show_frustum_shader.add_uniform("ModelViewProjectionMatrix", 16, &camera_view_proj_matrix.mat[0]);

//gpu culling shader
	culling_shader.programm_id = init_shader("culling_vs", "culling_ps", "culling_gs");
	culling_shader.add_uniform("ModelViewProjectionMatrix", 16, &camera_view_proj_matrix.mat[0]);
	culling_shader.add_uniform("frustum_planes", 4, &frustum.frustum_planes[0].x, FLOAT_UNIFORM_TYPE, 6);

	//http://steps3d.narod.ru/tutorials/tf3-tutorial.html
	//https://open.gl/feedback

	//prepare transform feedback
	const char *vars[] = { "output_instance_data1", "output_instance_data2" };
	glTransformFeedbackVaryings(culling_shader.programm_id, 2, vars, GL_INTERLEAVED_ATTRIBS);
	link_shader(culling_shader.programm_id); // relink required
	glUseProgram(0);

//queries for getting feedback from gpu - num visible instances
	glGenQueries(2, &num_visible_instances_query[0]);
}


void Init(HWND hWnd)
{
	int i, j, k;

	//init gl
	g_hWnd = hWnd;										// Assign the window handle to a global window handle
	GetClientRect(g_hWnd, &g_rRect);					// Assign the windows rectangle to a global RECT
	InitializeOpenGL(g_rRect.right, g_rRect.bottom);	// Init OpenGL with the global rect

	glext_init();
	clearDebugLog();

	opengl_debug_mode_enabled = glGetDebugMessageLogARB != NULL;
	if (opengl_debug_mode_enabled)
	{
		glDebugMessageCallbackARB(&DebugCallback, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	}

	glEnable(GL_DEPTH_TEST);
	SizeOpenGLScreen(g_rRect.right, g_rRect.bottom);	// Setup the screen translations and viewport

//rnd
	rndInit();
	int rnd_seed = (int)timeGetTime() % 65535;
	rndSeed(rnd_seed);

//timing
	InitTimeOperation();

//camera
	camera.PositionCamera(10.f, 10.f, 10.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
	camera.show_framerate = !only_culling_measurements;

//shaders
	init_shaders();

//scene
	create_scene();

//init gl states
	glDepthFunc(GL_LESS);

//check errors
	CheckGLErrors();

//multithreading
	create_threads();
}


void ShutDown()
{
	threads_close();

//clear all data
	delete_sse_array(sphere_data, MAX_SCENE_OBJECTS);
	delete_sse_array(aabb_data, MAX_SCENE_OBJECTS);
	delete_sse(culling_res);
	delete_sse_array(sse_obj_mat, MAX_SCENE_OBJECTS);
	if (obj_mat) {
		delete[]obj_mat; obj_mat = NULL;
	}

//clear buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wire_box_ibo_id);
	glDeleteBuffers(1, &wire_box_ibo_id);
	glBindBuffer(GL_ARRAY_BUFFER, wire_box_vbo_id);
	glDeleteBuffers(1, &wire_box_vbo_id);
	glBindVertexArray(wire_box_vao_id);
	glDeleteBuffers(1, &wire_box_vao_id);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ground_ibo_id);
	glDeleteBuffers(1, &ground_ibo_id);
	glBindBuffer(GL_ARRAY_BUFFER, ground_vbo_id);
	glDeleteBuffers(1, &ground_vbo_id);
	glBindVertexArray(ground_vao_id);
	glDeleteBuffers(1, &ground_vao_id);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_ibo_id);
	glDeleteBuffers(1, &geometry_ibo_id);
	glBindBuffer(GL_ARRAY_BUFFER, geometry_vbo_id);
	glDeleteBuffers(1, &geometry_vbo_id);
	glBindVertexArray(geometry_vao_id);
	glDeleteBuffers(1, &geometry_vao_id);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, dips_texture_buffer_tex);
	glDeleteTextures(1, &dips_texture_buffer_tex);
	glBindBuffer(GL_TEXTURE_BUFFER, dips_texture_buffer);
	glDeleteBuffers(1, &dips_texture_buffer);


	glBindBuffer(GL_ARRAY_BUFFER, all_instances_data_vbo);
	glDeleteBuffers(1, &all_instances_data_vbo);
	glBindVertexArray(all_instances_data_vao);
	glDeleteBuffers(1, &all_instances_data_vao);


//shaders
	glDeleteProgram(ground_shader.programm_id);
	glDeleteProgram(geometry_shader.programm_id);
	glDeleteProgram(show_frustum_shader.programm_id);
	glDeleteProgram(culling_shader.programm_id);

//queries
	glDeleteQueries(2, &num_visible_instances_query[0]);
}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------simple culling

__forceinline bool SphereInFrustum(vec3 &pos, float &radius, vec4 *frustum_planes)
{
	bool res = true;
	//test all 6 frustum planes
	for (int i = 0; i < 6; i++)
	{
		//calculate distance from sphere center to plane.
		//if distance larger then sphere radius - sphere is outside frustum
		if (frustum_planes[i].x * pos.x + frustum_planes[i].y * pos.y + frustum_planes[i].z * pos.z + frustum_planes[i].w <= -radius)
			res = false;
			//return false; //with flag works faster
	}
	return res;
	//return true;
}


__forceinline bool RightParallelepipedInFrustum(vec4 &Min, vec4 &Max, vec4 *frustum_planes)
{
	bool inside = true;
	//test all 6 frustum planes
	for (int i = 0; i<6; i++)
	{
		//pick closest point to plane and check if it behind the plane
		//if yes - object outside frustum
		float d = max(Min.x * frustum_planes[i].x, Max.x * frustum_planes[i].x)
				+ max(Min.y * frustum_planes[i].y, Max.y * frustum_planes[i].y)
				+ max(Min.z * frustum_planes[i].z, Max.z * frustum_planes[i].z)
				+ frustum_planes[i].w;
		inside &= d > 0;
		//return false; //with flag works faster
	}
	return inside;
}



__forceinline bool RightParallelepipedInFrustum2(vec4 &Min, vec4 &Max, vec4 *frustum_planes)
{
	//this is just example of basic idea - how BOX culling works, both AABB and OBB
		//Min & Max are 2 world space box points. For AABB-drustum culling
		//We may use transformed (by object matrix) to world space 8 box points. Replace Min & Max in equations and we get OBB-frustum.

	//test all 6 frustum planes
	for (int i = 0; i<6; i++)
	{
		//try to find such plane for which all 8 box points behind it
			//test all 8 box points against frustum plane
			//calculate distance from point to plane
			//if point infront of the plane (dist > 0) - this is not separating plane
		if (frustum_planes[i][0] * Min[0] + frustum_planes[i][1] * Max[1] + frustum_planes[i][2] * Min[2] + frustum_planes[i][3]>0)
			continue;
		if (frustum_planes[i][0] * Min[0] + frustum_planes[i][1] * Max[1] + frustum_planes[i][2] * Max[2] + frustum_planes[i][3]>0)
			continue;
		if (frustum_planes[i][0] * Max[0] + frustum_planes[i][1] * Max[1] + frustum_planes[i][2] * Max[2] + frustum_planes[i][3]>0)
			continue;
		if (frustum_planes[i][0] * Max[0] + frustum_planes[i][1] * Max[1] + frustum_planes[i][2] * Min[2] + frustum_planes[i][3]>0)
			continue;
		if (frustum_planes[i][0] * Max[0] + frustum_planes[i][1] * Min[1] + frustum_planes[i][2] * Min[2] + frustum_planes[i][3]>0)
			continue;
		if (frustum_planes[i][0] * Max[0] + frustum_planes[i][1] * Min[1] + frustum_planes[i][2] * Max[2] + frustum_planes[i][3]>0)
			continue;
		if (frustum_planes[i][0] * Min[0] + frustum_planes[i][1] * Min[1] + frustum_planes[i][2] * Max[2] + frustum_planes[i][3]>0)
			continue;
		if (frustum_planes[i][0] * Min[0] + frustum_planes[i][1] * Min[1] + frustum_planes[i][2] * Min[2] + frustum_planes[i][3]>0)
			continue;

		return false;
	}
	return true;
}

__forceinline bool OBBInFrustum(const vec3 &Min, const vec3 &Max, mat4 &obj_transform_mat, mat4 &cam_modelview_proj_mat)
{
	//transform all 8 box points to clip space
	//clip space because we easily can test points outside required unit cube
		//NOTE: for DirectX we should test z coordinate from 0 to w (-w..w - for OpenGL), look for transformations / clipping box differences

	//matrix to transfrom points to clip space
	mat4 to_clip_space_mat = cam_modelview_proj_mat * obj_transform_mat;

	//transform all 8 box points to clip space
	vec4 obb_points[8];
	obb_points[0] = to_clip_space_mat * vec4(Min[0], Max[1], Min[2], 1.f);
	obb_points[1] = to_clip_space_mat * vec4(Min[0], Max[1], Max[2], 1.f);
	obb_points[2] = to_clip_space_mat * vec4(Max[0], Max[1], Max[2], 1.f);
	obb_points[3] = to_clip_space_mat * vec4(Max[0], Max[1], Min[2], 1.f);
	obb_points[4] = to_clip_space_mat * vec4(Max[0], Min[1], Min[2], 1.f);
	obb_points[5] = to_clip_space_mat * vec4(Max[0], Min[1], Max[2], 1.f);
	obb_points[6] = to_clip_space_mat * vec4(Min[0], Min[1], Max[2], 1.f);
	obb_points[7] = to_clip_space_mat * vec4(Min[0], Min[1], Min[2], 1.f);

	bool outside = false, outside_positive_plane, outside_negative_plane;
	//we have 6 frustum planes, which in clip space is unit cube (for GL) with -1..1 range
	for (int i = 0; i < 3; i++) //3 because we test positive & negative plane at once
	{
		//if all 8 points outside one of the plane
		//actually it is vertex normalization xyz / w, then compare if all 8points coordinates < -1 or > 1

		outside_positive_plane =
			obb_points[0][i] > obb_points[0].w &&
			obb_points[1][i] > obb_points[1].w &&
			obb_points[2][i] > obb_points[2].w &&
			obb_points[3][i] > obb_points[3].w &&
			obb_points[4][i] > obb_points[4].w &&
			obb_points[5][i] > obb_points[5].w &&
			obb_points[6][i] > obb_points[6].w &&
			obb_points[7][i] > obb_points[7].w;

		outside_negative_plane =
			 obb_points[0][i] < -obb_points[0].w &&
			 obb_points[1][i] < -obb_points[1].w &&
			 obb_points[2][i] < -obb_points[2].w &&
			 obb_points[3][i] < -obb_points[3].w &&
			 obb_points[4][i] < -obb_points[4].w &&
			 obb_points[5][i] < -obb_points[5].w &&
			 obb_points[6][i] < -obb_points[6].w &&
			 obb_points[7][i] < -obb_points[7].w;

		outside = outside || outside_positive_plane || outside_negative_plane;
		//if (outside_positive_plane || outside_negative_plane)
			//return false;
	}
	return !outside;
	//return true;
}




void simple_culling_spheres(BSphere *sphere_data, int num_objects, int *culling_res, vec4 *frustum_planes)
{
	for (int i = 0; i < num_objects; i++)
		culling_res[i] = !SphereInFrustum(sphere_data[i].pos, sphere_data[i].r, &frustum_planes[0]);
}

void simple_culling_aabb(AABB *aabb_data, int num_objects, int *culling_res, vec4 *frustum_planes)
{
	for (int i = 0; i < num_objects; i++)
		culling_res[i] = !RightParallelepipedInFrustum(aabb_data[i].box_min, aabb_data[i].box_max, &frustum_planes[0]);
}

void simple_culling_obb(int first_processing_oject, int num_objects, int *culling_res, vec4 *frustum_planes)
{
	for (int i = first_processing_oject; i < first_processing_oject + num_objects; i++)
		culling_res[i] = !OBBInFrustum(box_min, box_max, obj_mat[i], camera_view_proj_matrix);

}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------sse culling

void sse_culling_spheres(BSphere *sphere_data, int num_objects, int *culling_res, vec4 *frustum_planes)
{
	float *sphere_data_ptr = reinterpret_cast<float*>(&sphere_data[0]);
	int *culling_res_sse = &culling_res[0];

	//to optimize calculations we gather xyzw elements in separate vectors
	__m128 zero_v = _mm_setzero_ps();
	__m128 frustum_planes_x[6];
	__m128 frustum_planes_y[6];
	__m128 frustum_planes_z[6];
	__m128 frustum_planes_d[6];
	int i, j;
	for (i = 0; i < 6; i++)
	{
		frustum_planes_x[i] = _mm_set1_ps(frustum_planes[i].x);
		frustum_planes_y[i] = _mm_set1_ps(frustum_planes[i].y);
		frustum_planes_z[i] = _mm_set1_ps(frustum_planes[i].z);
		frustum_planes_d[i] = _mm_set1_ps(frustum_planes[i].w);
	}

	//we process 4 objects per step
	for (i = 0; i < num_objects; i += 4)
	{
		//load bounding sphere data
		__m128 spheres_pos_x = _mm_load_ps(sphere_data_ptr);
		__m128 spheres_pos_y = _mm_load_ps(sphere_data_ptr + 4);
		__m128 spheres_pos_z = _mm_load_ps(sphere_data_ptr + 8);
		__m128 spheres_radius = _mm_load_ps(sphere_data_ptr + 12);
		sphere_data_ptr += 16;

		//but for our calculations we need transpose data, to collect x, y, z and w coordinates in separate vectors
		_MM_TRANSPOSE4_PS(spheres_pos_x, spheres_pos_y, spheres_pos_z, spheres_radius);

		__m128 spheres_neg_radius = _mm_sub_ps(zero_v, spheres_radius);  // negate all elements
																		 //http://fastcpp.blogspot.ru/2011/03/changing-sign-of-float-values-using-sse.html

		__m128 intersection_res = _mm_setzero_ps();

		for (j = 0; j < 6; j++) //plane index
		{
			//1. calc distance to plane dot(sphere_pos.xyz, plane.xyz) + plane.w
			//2. if distance < sphere radius, then sphere outside frustum
			__m128 dot_x = _mm_mul_ps(spheres_pos_x, frustum_planes_x[j]);
			__m128 dot_y = _mm_mul_ps(spheres_pos_y, frustum_planes_y[j]);
			__m128 dot_z = _mm_mul_ps(spheres_pos_z, frustum_planes_z[j]);

			__m128 sum_xy = _mm_add_ps(dot_x, dot_y);
			__m128 sum_zw = _mm_add_ps(dot_z, frustum_planes_d[j]);
			__m128 distance_to_plane = _mm_add_ps(sum_xy, sum_zw);

			__m128 plane_res = _mm_cmple_ps(distance_to_plane, spheres_neg_radius); //dist < -sphere_r ?
			intersection_res = _mm_or_ps(intersection_res, plane_res); //if yes - sphere behind the plane & outside frustum
		}

		//store result
		__m128i intersection_res_i = _mm_cvtps_epi32(intersection_res); //convert to integers
		_mm_store_si128((__m128i *)&culling_res_sse[i], intersection_res_i); //store result in culling_res_sse[]
	}
}


void sse_culling_aabb(AABB *aabb_data, int num_objects, int *culling_res, vec4 *frustum_planes)
{
	float *aabb_data_ptr = reinterpret_cast<float*>(&aabb_data[0]);
	int *culling_res_sse = &culling_res[0];

	//to optimize calculations we gather xyzw elements in separate vectors
	__m128 zero_v = _mm_setzero_ps();
	__m128 frustum_planes_x[6];
	__m128 frustum_planes_y[6];
	__m128 frustum_planes_z[6];
	__m128 frustum_planes_d[6];
	int i, j;
	for (i = 0; i < 6; i++)
	{
		frustum_planes_x[i] = _mm_set1_ps(frustum_planes[i].x);
		frustum_planes_y[i] = _mm_set1_ps(frustum_planes[i].y);
		frustum_planes_z[i] = _mm_set1_ps(frustum_planes[i].z);
		frustum_planes_d[i] = _mm_set1_ps(frustum_planes[i].w);
	}

	__m128 zero = _mm_setzero_ps();
	//we process 4 objects per step
	for (i = 0; i < num_objects; i += 4)
	{
		//load objects data
		//load aabb min
		__m128 aabb_min_x = _mm_load_ps(aabb_data_ptr);
		__m128 aabb_min_y = _mm_load_ps(aabb_data_ptr + 8);
		__m128 aabb_min_z = _mm_load_ps(aabb_data_ptr + 16);
		__m128 aabb_min_w = _mm_load_ps(aabb_data_ptr + 24);

		//load aabb max
		__m128 aabb_max_x = _mm_load_ps(aabb_data_ptr + 4);
		__m128 aabb_max_y = _mm_load_ps(aabb_data_ptr + 12);
		__m128 aabb_max_z = _mm_load_ps(aabb_data_ptr + 20);
		__m128 aabb_max_w = _mm_load_ps(aabb_data_ptr + 28);

		aabb_data_ptr += 32;

		//for now we have points in vectors aabb_min_x..w, but for calculations we need to xxxx yyyy zzzz vectors representation - just transpose data
		_MM_TRANSPOSE4_PS(aabb_min_x, aabb_min_y, aabb_min_z, aabb_min_w);
		_MM_TRANSPOSE4_PS(aabb_max_x, aabb_max_y, aabb_max_z, aabb_max_w);

		__m128 intersection_res = _mm_setzero_ps();
		for (j = 0; j < 6; j++) //plane index
		{
			//this code is similar to what we make in simple culling
				//pick closest point to plane and check if it begind the plane. if yes - object outside frustum

			//dot product, separate for each coordinate, for min & max aabb points
			__m128 aabbMin_frustumPlane_x = _mm_mul_ps(aabb_min_x, frustum_planes_x[j]);
			__m128 aabbMin_frustumPlane_y = _mm_mul_ps(aabb_min_y, frustum_planes_y[j]);
			__m128 aabbMin_frustumPlane_z = _mm_mul_ps(aabb_min_z, frustum_planes_z[j]);

			__m128 aabbMax_frustumPlane_x = _mm_mul_ps(aabb_max_x, frustum_planes_x[j]);
			__m128 aabbMax_frustumPlane_y = _mm_mul_ps(aabb_max_y, frustum_planes_y[j]);
			__m128 aabbMax_frustumPlane_z = _mm_mul_ps(aabb_max_z, frustum_planes_z[j]);

			//we have 8 box points, but we need pick closest point to plane. Just take max
			__m128 res_x = _mm_max_ps(aabbMin_frustumPlane_x, aabbMax_frustumPlane_x);
			__m128 res_y = _mm_max_ps(aabbMin_frustumPlane_y, aabbMax_frustumPlane_y);
			__m128 res_z = _mm_max_ps(aabbMin_frustumPlane_z, aabbMax_frustumPlane_z);

			//dist to plane = dot(aabb_point.xyz, plane.xyz) + plane.w
			__m128 sum_xy = _mm_add_ps(res_x, res_y);
			__m128 sum_zw = _mm_add_ps(res_z, frustum_planes_d[j]);
			__m128 distance_to_plane = _mm_add_ps(sum_xy, sum_zw);

			__m128 plane_res = _mm_cmple_ps(distance_to_plane, zero); //dist from closest point to plane < 0 ?
			intersection_res = _mm_or_ps(intersection_res, plane_res); //if yes - aabb behind the plane & outside frustum
		}

		//store result
		__m128i intersection_res_i = _mm_cvtps_epi32(intersection_res); //convert to integers
		_mm_store_si128((__m128i *)&culling_res_sse[i], intersection_res_i); //store result in culling_res_sse[]
	}
}


void sse_culling_obb(int firs_processing_object, int num_objects, int *culling_res, mat4 &cam_modelview_proj_mat)
{
	mat4_sse sse_camera_mat(cam_modelview_proj_mat);
	mat4_sse sse_clip_space_mat;

//box points in local space
	__m128 obb_points_sse[8];
	obb_points_sse[0] = _mm_set_ps(1.f, box_min[2], box_max[1], box_min[0]);
	obb_points_sse[1] = _mm_set_ps(1.f, box_max[2], box_max[1], box_min[0]);
	obb_points_sse[2] = _mm_set_ps(1.f, box_max[2], box_max[1], box_max[0]);
	obb_points_sse[3] = _mm_set_ps(1.f, box_min[2], box_max[1], box_max[0]);
	obb_points_sse[4] = _mm_set_ps(1.f, box_min[2], box_min[1], box_max[0]);
	obb_points_sse[5] = _mm_set_ps(1.f, box_max[2], box_min[1], box_max[0]);
	obb_points_sse[6] = _mm_set_ps(1.f, box_max[2], box_min[1], box_min[0]);
	obb_points_sse[7] = _mm_set_ps(1.f, box_min[2], box_min[1], box_min[0]);

	ALIGN_SSE int obj_culling_res[4];

	__m128 zero_v = _mm_setzero_ps();
	int i, j;

	//process one object per step
	for (i = firs_processing_object; i < firs_processing_object+num_objects; i++)
	{
	//clip space matrix = camera_view_proj * obj_mat
		sse_mat4_mul(sse_clip_space_mat, sse_camera_mat, sse_obj_mat[i]);

		//initially assume that planes are separating
		//if any axis is separating - we get 0 in certain outside_* place
		__m128 outside_positive_plane = _mm_set1_ps(-1.f); //NOTE: there should be negative value..
		__m128 outside_negative_plane = _mm_set1_ps(-1.f); //because _mm_movemask_ps (while storing result) cares abount 'most significant bits' (it is sign of float value)

		//for all 8 box points
		for (j = 0; j < 8; j++)
		{
		//transform point to clip space
			__m128 obb_transformed_point = sse_mat4_mul_vec4(sse_clip_space_mat, obb_points_sse[j]);

		//gather w & -w
			__m128 wwww = _mm_shuffle_ps(obb_transformed_point, obb_transformed_point, _MM_SHUFFLE(3, 3, 3, 3)); //get w
			__m128 wwww_neg = _mm_sub_ps(zero_v, wwww);  // negate all elements

		//box_point.xyz > box_point.w || box_point.xyz < -box_point.w ?
		//similar to point normalization: point.xyz /= point.w; And compare: point.xyz > 1 && point.xyz < -1
			__m128 outside_pos_plane = _mm_cmpge_ps(obb_transformed_point, wwww);
			__m128 outside_neg_plane = _mm_cmple_ps(obb_transformed_point, wwww_neg);

		//if at least 1 of 8 points in front of the plane - we get 0 in outside_* flag
			outside_positive_plane = _mm_and_ps(outside_positive_plane, outside_pos_plane);
			outside_negative_plane = _mm_and_ps(outside_negative_plane, outside_neg_plane);
		}

		//all 8 points xyz < -1 or > 1 ?
		__m128 outside = _mm_or_ps(outside_positive_plane, outside_negative_plane);

		//store result, if any of 3 axes is separating (i.e. outside != 0) - object outside frustum
		//so, object inside frustum only if outside == 0 (there are no separating axes)
		culling_res[i] = _mm_movemask_ps(outside) & 0x7; //& 0x7 mask, because we interested only in 3 axes
	}
}



//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------culling
void do_cpu_culling()
{
//culling
	Timer timer;
	if (only_culling_measurements)
		timer.StartTiming();
	if (use_multithreading)
	{
		process_multithreading_culling();
		wate_multithreading_culling_done();
	} else
		cull_objects(0, MAX_SCENE_OBJECTS);

//computing time
	if (only_culling_measurements)
	{
		//calculate avg dt for 1k frames
		static int num_attempts = 0;
		static double accumulated_dt = 0.0;
		accumulated_dt += timer.TimeElapsedInMS();
		num_attempts++;
		if (num_attempts == 1000)
		{
			double avg_dt = accumulated_dt / 1000.0;
			char str[64];
			sprintf(str, "culling takes: %.2f", float(avg_dt));
			SetWindowText(g_hWnd, str);
			num_attempts = 0;
			accumulated_dt = 0.0;
		}
	}

//collect & transfer visible instances data to gpu
	num_visible_instances = 0;
	for (int i = 0; i < MAX_SCENE_OBJECTS; i++) if (culling_res[i] == 0)
	{
		visible_instance_info[num_visible_instances * 2 + 0] = instance_info[i * 2 + 0];
		visible_instance_info[num_visible_instances * 2 + 1] = instance_info[i * 2 + 1];
		num_visible_instances++;
	}

//copy to gpu
	if (enable_rendering_objects)
	{
		//lock buffer & copy visible instances data
		glBindBuffer(GL_TEXTURE_BUFFER, dips_texture_buffer);
		float* tbo_data = (float*)glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);

		//copy instances data
		memcpy(&tbo_data[0], &visible_instance_info[0], sizeof(vec4)*num_visible_instances * 2);

		glUnmapBuffer(GL_TEXTURE_BUFFER);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}
}



void do_gpu_culling()
{
	culling_shader.bind();

	int cur_frame = frame_index % 2;
	int prev_frame = (frame_index + 1) % 2;

	//prepare feedback & query
	glEnable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, dips_texture_buffer);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, num_visible_instances_query[cur_frame]);
	glBeginTransformFeedback(GL_POINTS);

	//render cloud of points which we interprent as objects data
	glBindVertexArray(all_instances_data_vao);
	glDrawArrays(GL_POINTS, 0, MAX_SCENE_OBJECTS);
	glBindVertexArray(0);

	//disable all
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	glDisable(GL_RASTERIZER_DISCARD);

	//get feedback from prev frame
	num_visible_instances = 0;
	glGetQueryObjectiv(num_visible_instances_query[prev_frame], GL_QUERY_RESULT, &num_visible_instances);

	//next frame
	frame_index++;
}


void cull_objects(int first_processing_oject, int num_processing_ojects)
{
	switch (culling_mode)
	{
	case SIMPLE_SPHERES:
		simple_culling_spheres(&sphere_data[first_processing_oject], num_processing_ojects, &culling_res[first_processing_oject], &frustum.frustum_planes[0]);
		break;
	case SIMPLE_AABB:
		simple_culling_aabb(&aabb_data[first_processing_oject], num_processing_ojects, &culling_res[first_processing_oject], &frustum.frustum_planes[0]);
		break;
	case SIMPLE_OBB:
		simple_culling_obb(first_processing_oject, num_processing_ojects, &culling_res[0], &frustum.frustum_planes[0]);
		break;

	case SSE_SPHERES:
		sse_culling_spheres(&sphere_data[first_processing_oject], num_processing_ojects, &culling_res[first_processing_oject], &frustum.frustum_planes[0]);
		break;
	case SSE_AABB:
		sse_culling_aabb(&aabb_data[first_processing_oject], num_processing_ojects, &culling_res[first_processing_oject], &frustum.frustum_planes[0]);
		break;
	case SSE_OBB:
		sse_culling_obb(first_processing_oject, num_processing_ojects, &culling_res[0], camera_view_proj_matrix);
		break;
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------threads
Worker::Worker() : first_processing_oject(0), num_processing_ojects(0)
{
	//create 2 events: 1. to signal that we have a job 2.signal that we finished job
	has_jobs_event = CreateEvent(NULL, false, false, NULL);
	jobs_finished_event = CreateEvent(NULL, false, true, NULL);
}

Worker::~Worker()
{
	CloseHandle(has_jobs_event);
	CloseHandle(jobs_finished_event);
}

void Worker::doJob()
{
	//make our part of work
	cull_objects(first_processing_oject, num_processing_ojects);
}

unsigned __stdcall thread_func(void* arguments)
{
	printf("In thread...\n");
	Worker *worker = static_cast<Worker*>(arguments);

	//each worker has endless loop untill we signal to quit (stop_work flag)
	while (true)
	{
		//wait for starting jobs
		//if we have no job - just wait (has_jobs_event event). We do not wasting cpu work. Events designed for this.
		WaitForSingleObject(worker->has_jobs_event, INFINITE);

		//if we have signal to break - exit endless loop
		if (worker->stop_work)
			break;

		//do job
		worker->doJob();

		//signal that we finished the job
		SetEvent(worker->jobs_finished_event);
	}

	_endthreadex(0);
	return 0;
}

void create_threads()
{
	//create the threads

	//split the work into parts between threads
	int worker_num_processing_ojects = MAX_SCENE_OBJECTS / num_workers;
	int first_processing_oject = 0;

	int i;
	for (i = 0; i < num_workers; i++)
	{
		//create threads
		workers[i].thread_handle = (HANDLE)_beginthreadex(NULL, 0, &thread_func, &workers[i], CREATE_SUSPENDED, &workers[i].thread_id);
		thread_handles[i] = workers[i].thread_handle;

		//set threads parameters
		workers[i].first_processing_oject = first_processing_oject;
		workers[i].num_processing_ojects = worker_num_processing_ojects;
		first_processing_oject += worker_num_processing_ojects;
	}

	//run workers to do their jobs
	for (int i = 0; i < num_workers; i++)
		ResumeThread(workers[i].thread_handle);
}

void threads_close()
{
	//signal to stop the work for all threads
	for (int i = 0; i < num_workers; i++)
	{
		workers[i].stop_work = true;
		SetEvent(workers[i].has_jobs_event);
	}

	//wait all workers to finish current jobs
	WaitForMultipleObjects(num_workers, &thread_handles[0], true, INFINITE);

	//remove workers
	//_endthreadex is called automatically when the thread returns from the routine passed as a parameter
}


void process_multithreading_culling()
{
	//signal workers that they have the job
	for (int i = 0; i < num_workers; i++)
		SetEvent(workers[i].has_jobs_event);
}

void wate_multithreading_culling_done()
{
	//wait threads to do their jobs
	HANDLE wait_events[num_workers];
	for (int i = 0; i < num_workers; i++)
		wait_events[i] = workers[i].jobs_finished_event;
	WaitForMultipleObjects(num_workers, &wait_events[0], true, INFINITE);
}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------render
void process_key(int key)
{
	switch(key)
	{
	case VK_SPACE:
		culling_enabled = !culling_enabled;
		break;

	case 'H':
		enable_rendering_objects = !enable_rendering_objects;
		break;

	case VK_NUMPAD0:
	case '0':
		use_multithreading = !use_multithreading;
		use_gpu_culling = false;
		break;

	case VK_NUMPAD1:
	case '1':
		culling_mode = SIMPLE_SPHERES;
		use_gpu_culling = false;
		break;
	case VK_NUMPAD2:
	case '2':
		culling_mode = SIMPLE_AABB;
		use_gpu_culling = false;
		break;
	case VK_NUMPAD3:
	case '3':
		culling_mode = SIMPLE_OBB;
		use_gpu_culling = false;
		break;

	case VK_NUMPAD4:
	case '4':
		culling_mode = SSE_SPHERES;
		use_gpu_culling = false;
		break;
	case VK_NUMPAD5:
	case '5':
		culling_mode = SSE_AABB;
		use_gpu_culling = false;
		break;
	case VK_NUMPAD6:
	case '6':
		culling_mode = SSE_OBB;
		use_gpu_culling = false;
		break;

	case VK_NUMPAD7:
	case '7':
		use_gpu_culling = !use_gpu_culling;
		break;
	};
}

void RenderScene()
{
	int i, j, k;

//time
	static double lastTime = timeGetTime() * 0.001;
	double timeleft = 0.0;
	double currentTime = timeGetTime() * 0.001;
	timeleft = currentTime - lastTime;
	const double min_dt = 1.0f / 20.0;
	if (timeleft > min_dt) timeleft = min_dt;
	lastTime = currentTime;
	total_time += timeleft;


//camera
	camera.Update();
	camera_view_matrix.look_at(camera.Position(), camera.View(), camera.UpVector());
	camera_proj_matrix.perspective(45.f, (float)window_width / (float)window_height, zNear, zFar);
	camera_view_proj_matrix = camera_proj_matrix * camera_view_matrix;

//RENDER
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window_width, window_height);
	glClearColor(110.0f / 255.0f, 149.0f / 255.0f, 224.0f / 255.0f, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);


//objects culling
	if (culling_enabled)
	{
		//prepare camera & frustum
		saved_inv_view_proj_matrix = camera_view_proj_matrix.inverse();
		frustum.CalculateFrustum(camera_view_matrix, camera_proj_matrix);

		//switch between gpu and cpu culling
		if (use_gpu_culling)
			do_gpu_culling();
		else
			do_cpu_culling();
	}

//render ground (just a plane)
	ground_shader.bind();
	glBindVertexArray(ground_vao_id);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);


//geometry (colored boxes)
	if (enable_rendering_objects)
	{
		glEnable(GL_CULL_FACE);
		geometry_shader.bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, dips_texture_buffer_tex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, dips_texture_buffer);

		glBindVertexArray(geometry_vao_id);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL, num_visible_instances);
		glBindVertexArray(0);
	}
	
//show frustum
	if (!culling_enabled)
	{
		show_frustum_shader.bind();

		glBindVertexArray(wire_box_vao_id);
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);
	}

	glFlush();
	SwapBuffers(g_hDC);
}