#include "Utilities.h"


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------textures
void create_simple_texture(GLuint &tex_id, int width, int height, GLenum internal_format, GLenum format, GLint filtration1, GLint filtration2, GLint wrap, GLenum type)
{
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);

	glTexParameterIiv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &filtration1);
	glTexParameterIiv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &filtration2);
	glTexParameterIiv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrap);
	glTexParameterIiv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &wrap);
	glTexParameterIiv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, &wrap);

	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}



//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------vertex buffers


void create_render_element(GLuint &vao_id,
	GLuint &vbo_id, RenderElementDescription &desc,
	bool use_ibo, GLuint &ibo_id, int num_indices, int *indices)
{
//create vertex buffer
	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, desc.struct_size * desc.num_verts, desc.data_pointer, desc.vbo_usage);

//create index buffer
	if (use_ibo)
	{
		glGenBuffers(1, &ibo_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)* num_indices, &indices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

//create vertex array
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);
	if (use_ibo)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

	//bind vertex attributes
	for (int i = 0; i < desc.num_vbo_elements; i++)
	{
		glEnableVertexAttribArray(i);
		glVertexAttribPointer((GLuint)i, desc.elements[i].number_of_elements, desc.elements[i].elem_type, GL_FALSE, desc.struct_size, (GLvoid*)(desc.elements[i].pointer_offset));
	}

	glBindVertexArray(0);
}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------shaders

void _print_programme_info_log(GLuint programme)
{
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetProgramInfoLog(programme, max_length, &actual_length, log);
	printf("program info log for GL index %u:\n%s", programme, log);
}

bool is_valid(GLuint programme)
{
	glValidateProgram(programme);
	int params = -1;
	glGetProgramiv(programme, GL_VALIDATE_STATUS, &params);
	printf("program %i GL_VALIDATE_STATUS = %i\n", programme, params);
	if (GL_TRUE != params)
	{
		_print_programme_info_log(programme);
		return false;
	}
	return true;
}


const char* GL_type_to_string(GLenum type)
{
	switch (type)
	{
	case GL_BOOL: return "bool";
	case GL_INT: return "int";
	case GL_FLOAT: return "float";
	case GL_FLOAT_VEC2: return "vec2";
	case GL_FLOAT_VEC3: return "vec3";
	case GL_FLOAT_VEC4: return "vec4";
	case GL_FLOAT_MAT2: return "mat2";
	case GL_FLOAT_MAT3: return "mat3";
	case GL_FLOAT_MAT4: return "mat4";
	case GL_SAMPLER_2D: return "sampler2D";
	case GL_SAMPLER_3D: return "sampler3D";
	case GL_SAMPLER_CUBE: return "samplerCube";
	case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
	default: break;
	}
	return "other";
}

void shader_print_all_info(GLuint programme)
{
	printf("shader programme %i info:\n", programme);
	int params = -1;
	glGetProgramiv(programme, GL_LINK_STATUS, &params);
	printf("GL_LINK_STATUS = %i\n", params);

	glGetProgramiv(programme, GL_ATTACHED_SHADERS, &params);
	printf("GL_ATTACHED_SHADERS = %i\n", params);

	glGetProgramiv(programme, GL_ACTIVE_ATTRIBUTES, &params);
	printf("GL_ACTIVE_ATTRIBUTES = %i\n", params);
	for (int i = 0; i < params; i++)
	{
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveAttrib(
			programme,
			i,
			max_length,
			&actual_length,
			&size,
			&type,
			name
		);
		if (size > 1)
		{
			for (int j = 0; j < size; j++)
			{
				char long_name[64];
				sprintf(long_name, "%s[%i]", name, j);
				int location = glGetAttribLocation(programme, long_name);
				printf("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string(type), long_name, location);
			}
		}
		else
		{
			int location = glGetAttribLocation(programme, name);
			printf("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string(type), name, location);
		}
	}

	glGetProgramiv(programme, GL_ACTIVE_UNIFORMS, &params);
	printf("GL_ACTIVE_UNIFORMS = %i\n", params);
	for (int i = 0; i < params; i++)
	{
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveUniform(
			programme,
			i,
			max_length,
			&actual_length,
			&size,
			&type,
			name
		);
		if (size > 1) {
			for (int j = 0; j < size; j++)
			{
				char long_name[64];
				sprintf(long_name, "%s[%i]", name, j);
				int location = glGetUniformLocation(programme, long_name);
				printf("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string(type), long_name, location);
			}
		}
		else
		{
			int location = glGetUniformLocation(programme, name);
			printf("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string(type), name, location);
		}
	}

	_print_programme_info_log(programme);
	printf("\n\n");
}



char* load_file(const char* file_name)
{
//read file
	FILE *file = fopen(file_name, "rb");
	if (!file)
	{
		fprintf(stderr, "Shader::Shader(): can`t open \"%s\" file\n", file_name);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	char *data = new char[size + 1];
	data[size] = '\0';
	fseek(file, 0, SEEK_SET);
	fread(data, 1, size, file);
	fclose(file);

	return data;
}


void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		fprintf(stderr, "ShaderInfoLog\n%s\n", &infoLog[0]);
		free(infoLog);
	}
}

GLuint createShader(GLenum type, const GLchar* src)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);


	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		fprintf(stderr, "---Shader error in vertex shader \"%s\" file\n", "name");
		printShaderInfoLog(shader);
	}
	return shader;
}


GLuint init_shader(const char* vertex_shader_file, const char* frag_shader_file, const char* geometry_shader_file, bool call_link_shader)
{
	char tmp_str[64];

//load vertex shader
	const char *shaders_folder = "data/shaders/";
	sprintf(&tmp_str[0], "%s%s.txt", shaders_folder, vertex_shader_file);
	const char* VS_src = load_file(&tmp_str[0]);
	if (!VS_src)
		return -1;

//load pixel shader
	sprintf(&tmp_str[0], "%s%s.txt", shaders_folder, frag_shader_file);
	const char* PS_src = load_file(&tmp_str[0]);
	if (!PS_src)
		return -1;

//load geometry shader
	char* GS_src = NULL;
	if (geometry_shader_file)
	{
		sprintf(&tmp_str[0], "%s%s.txt", shaders_folder, geometry_shader_file);
		GS_src = load_file(&tmp_str[0]);
		if (!GS_src)
			return -1;
	}


//create shader, program
	GLuint vertexShader = createShader(GL_VERTEX_SHADER, VS_src);
	GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, PS_src);
	GLuint geometryShader = geometry_shader_file ? createShader(GL_GEOMETRY_SHADER, GS_src) : -1;

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);

	if (geometry_shader_file)
		glAttachShader(shaderProgram, geometryShader);

	glBindAttribLocationARB(shaderProgram, 0, "s_attribute_0");
	glBindAttribLocationARB(shaderProgram, 1, "s_attribute_1");
	glBindAttribLocationARB(shaderProgram, 2, "s_attribute_2");
	glBindAttribLocationARB(shaderProgram, 3, "s_attribute_3");
	glBindAttribLocationARB(shaderProgram, 4, "s_attribute_4");
	glBindAttribLocationARB(shaderProgram, 5, "s_attribute_5");
	glBindAttribLocationARB(shaderProgram, 6, "s_attribute_6");

	glBindAttribLocationARB(shaderProgram, 0, "s_pos");
	glBindAttribLocationARB(shaderProgram, 1, "s_normal");
	glBindAttribLocationARB(shaderProgram, 2, "s_uv");

	glAttachShader(shaderProgram, fragmentShader);

	if (call_link_shader)
		link_shader(shaderProgram);

	return shaderProgram;
}


void link_shader(GLuint shaderProgram)
{
	glLinkProgram(shaderProgram);

//check if link successed
	GLint linked;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
	if (!linked) {
		fprintf(stderr, "Shader::Shader(): GLSL relink error\n\n");
		return;
	}

	glValidateProgram(shaderProgram);
	GLint validated;
	glGetObjectParameterivARB(shaderProgram, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
	if (!validated) {
		fprintf(stderr, "Shader::Shader(): GLSL relink error\n\n");
		return;
	}

//bind textures
	glUseProgram(shaderProgram);

	for (int i = 0; i < 20; i++) {
		char texture[32];
		sprintf(texture, "s_texture_%d", i);
		GLint location = glGetUniformLocation(shaderProgram, texture);
		if (location >= 0)
			glUniform1i(location, i);
	}

	glUseProgram(0);
}



//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------debuging
bool opengl_debug_mode_enabled = false;
void clearDebugLog()
{
	FILE* f;
	f = fopen("Debug.txt", "w");
	if (f)
		fclose(f);
}


void DebugOutputToFile(unsigned int source, unsigned int type, unsigned int id,
	unsigned int severity, const char* message)
{
	FILE* f;
	f = fopen("Debug.txt", "a");
	if (f)
	{
		char debSource[16], debType[20], debSev[7];
		if (source == GL_DEBUG_SOURCE_API_ARB)
			strcpy(debSource, "OpenGL");
		else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
			strcpy(debSource, "Windows");
		else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
			strcpy(debSource, "Shader Compiler");
		else if (source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
			strcpy(debSource, "Third Party");
		else if (source == GL_DEBUG_SOURCE_APPLICATION_ARB)
			strcpy(debSource, "Application");
		else if (source == GL_DEBUG_SOURCE_OTHER_ARB)
			strcpy(debSource, "Other");

		if (type == GL_DEBUG_TYPE_ERROR_ARB)
			strcpy(debType, "Error");
		else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
			strcpy(debType, "Deprecated behavior");
		else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
			strcpy(debType, "Undefined behavior");
		else if (type == GL_DEBUG_TYPE_PORTABILITY_ARB)
			strcpy(debType, "Portability");
		else if (type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
			strcpy(debType, "Performance");
		else if (type == GL_DEBUG_TYPE_OTHER_ARB)
			strcpy(debType, "Other");

		if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
			strcpy(debSev, "High");
		else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
			strcpy(debSev, "Medium");
		else if (severity == GL_DEBUG_SEVERITY_LOW_ARB)
			strcpy(debSev, "Low");

		fprintf(f, "Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n",
			debSource, debType, id, debSev, message);
		fclose(f);
	}
}

void CheckDebugLog()
{
	unsigned int count = 10; // max. num. of messages that will be read from the log
	int bufsize = 2048;

	unsigned int* sources = new unsigned int[count];
	unsigned int* types = new unsigned int[count];
	unsigned int* ids = new unsigned int[count];
	unsigned int* severities = new unsigned int[count];
	int* lengths = new int[count];

	char* messageLog = new char[bufsize];

	unsigned int retVal = glGetDebugMessageLogARB(count, bufsize, sources, types, ids,
		severities, lengths, messageLog);
	if (retVal > 0)
	{
		unsigned int pos = 0;
		for (unsigned int i = 0; i<retVal; i++)
		{
			DebugOutputToFile(sources[i], types[i], ids[i], severities[i],
				&messageLog[pos]);
			pos += lengths[i];
		}
	}
	delete[] sources;
	delete[] types;
	delete[] ids;
	delete[] severities;
	delete[] lengths;
	delete[] messageLog;
}

void CALLBACK DebugCallback(unsigned int source, unsigned int type, unsigned int id,
	unsigned int severity, int length,
	const char* message, const void* userParam)
{
	DebugOutputToFile(source, type, id, severity, message);
}
