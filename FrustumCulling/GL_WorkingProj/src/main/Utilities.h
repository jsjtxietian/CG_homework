#ifndef _UTILITIES_H
#define _UTILITIES_H


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <mmsystem.h>
#include <math.h>
#include <gl\gl.h>										// Header File For The OpenGL32 Library
#include <gl\glu.h>										// Header File For The GLu32 Library

#include "../glext/glext.h"


#define del_it(a) if (a) { delete a; a = NULL; }

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------vertex buffer
const int VBO_STRUCT_MAX_ELEMENTS = 10;

//describes shader vertex declaration element
struct VboElement
{
	int number_of_elements;
	int pointer_offset;
	GLenum elem_type;
};

//structure that contains Vertex buffer data, description
struct RenderElementDescription
{
	int struct_size;
	int num_verts;
	void *data_pointer;

	GLenum vbo_usage;
	VboElement elements[VBO_STRUCT_MAX_ELEMENTS];
	int num_vbo_elements;

	void init(int in_struct_size, int in_num_verts, void* in_data_pointer, GLenum in_vbo_usage, int in_num_vbo_elements, VboElement *in_elements)
	{
		struct_size = in_struct_size;
		num_verts = in_num_verts;
		data_pointer = in_data_pointer;
		vbo_usage = in_vbo_usage;
		num_vbo_elements = in_num_vbo_elements;
		for (int i = 0; i < num_vbo_elements; i++)
		{
			elements[i].number_of_elements = in_elements[i].number_of_elements;
			elements[i].pointer_offset = in_elements[i].pointer_offset;
			elements[i].elem_type = in_elements[i].elem_type;
		}
	}
};



//------------------------------------------------------------------------------------------------------------------------------------------------------------------------other
//create texture
void create_simple_texture(GLuint &tex_id, int width, int height, GLenum internal_format, GLenum format, GLint filtration1, GLint filtration2, GLint wrap, GLenum type);

//create VAO, vertex array
void create_render_element(GLuint &vao_id,
	GLuint &vbo_id, RenderElementDescription &desc,
	bool use_ibo, GLuint &ibo_id, int num_indices, int *indices);

//shader
GLuint init_shader(const char* vertex_shader_file, const char* frag_shader_file, const char* geometry_shader_file = NULL, bool call_link_shader = true);
void link_shader(GLuint shaderProgram);

//debug
void clearDebugLog();
void CALLBACK DebugCallback(unsigned int source, unsigned int type, unsigned int id,
	unsigned int severity, int length,
	const char* message, const void* userParam);

#endif