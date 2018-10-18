#ifndef _MAIN_H
#define _MAIN_H


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <mmsystem.h>
#include <math.h>
#include <gl\gl.h>										// Header File For The OpenGL32 Library
#include <gl\glu.h>										// Header File For The GLu32 Library
//#include <gl\glcorearb.h>

#include "../glext/glext.h"




#define SCREEN_WIDTH 1600 //1600 1024 //1900								// We want our screen width 800 pixels
#define SCREEN_HEIGHT 900 //900 768//1000								// We want our screen height 600 pixels
#define SCREEN_DEPTH 32									// We want 16 bits per pixel

extern bool  g_bFullScreen;								// Set full screen as default
extern HWND  g_hWnd;									// This is the handle for the window
extern RECT  g_rRect;									// This holds the window dimensions
extern HDC   g_hDC;										// General HDC - (handle to device context)
extern HGLRC g_hRC;										// General OpenGL_DC - Our Rendering Context for OpenGL
extern HINSTANCE g_hInstance;							// This holds our window hInstance
extern int window_width;
extern int window_height;

/*
#include "../camera/frustum.h"


#include "../manager/ShaderManager.h"
#include "../shaders/shader.h"

#include "../manager/TextureManager.h"
#include "../timer/timer.h"

#include "../random/random.h"
#include "../font/font.h"
#include "../manager/fontmanager.h"

#include "../game_logic/map.h"
#include "../game_logic/list.h"

#include "../procedural_textures/perlinNoise.h"
#include "../procedural_textures/noise2d.h"

//#include "../game_logic/CreaturesEditor.h"
#include "../game_logic/Spline.h"
#include "../game_logic/CreateMesh.h"

#include "Utilities.h"
*/

#include "../timer/timer.h"
#include "../random/random.h"
#include "Utilities.h"

#include "../Camera/Camera.h"
#include "../Camera/Frustum.h"


//------------------------------------------------------------------------------------------------------------------------------------------------------shaders
const int MAX_SHADER_UNIFORMS = 10;
enum ShaderUniformVariableType
{
	FLOAT_UNIFORM_TYPE,
	INT_UNIFORM_TYPE,
};
struct ShaderUniformVariable
{
	ShaderUniformVariable() : type(FLOAT_UNIFORM_TYPE), dimmension(4), address(NULL), location(-1), count(1)
	{}
	ShaderUniformVariableType type;
	int dimmension;
	void *address;
	GLuint location;
	int count;
};

struct Shader
{
	Shader() : programm_id(-1), num_uniforms(0)
	{}
	~Shader()
	{
	}
	void add_uniform(const char* name, int size, void *address, ShaderUniformVariableType type = FLOAT_UNIFORM_TYPE, int num_elements = 1)
	{
		uniform_variables[num_uniforms].type = type;
		uniform_variables[num_uniforms].dimmension = size;
		uniform_variables[num_uniforms].address = address;
		uniform_variables[num_uniforms].location = glGetUniformLocation(programm_id, name);
		uniform_variables[num_uniforms].count = num_elements;
		num_uniforms++;
	}
	void bind(bool just_bind = false)
	{
		glUseProgram(programm_id);

		if (just_bind)
			return;

		for (int i = 0; i < num_uniforms; i++)
		{
			switch (uniform_variables[i].dimmension)
			{
			case 1:
				if (uniform_variables[i].type == FLOAT_UNIFORM_TYPE)
					glUniform1f(uniform_variables[i].location, *static_cast<GLfloat*>(uniform_variables[i].address));
				else
					glUniform1i(uniform_variables[i].location, *static_cast<GLint*>(uniform_variables[i].address));
				break;
			case 2:
				glUniform2fv(uniform_variables[i].location, uniform_variables[i].count, static_cast<GLfloat*>(uniform_variables[i].address));
				break;
			case 3:
				glUniform3fv(uniform_variables[i].location, uniform_variables[i].count, static_cast<GLfloat*>(uniform_variables[i].address));
				break;
			case 4:
				glUniform4fv(uniform_variables[i].location, uniform_variables[i].count, static_cast<GLfloat*>(uniform_variables[i].address));
				break;
			case 9:
				glUniformMatrix3fv(uniform_variables[i].location, 1, false, static_cast<GLfloat*>(uniform_variables[i].address));
				break;
			case 16:
				glUniformMatrix4fv(uniform_variables[i].location, 1, false, static_cast<GLfloat*>(uniform_variables[i].address));
				break;
			}
		}
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, tex_id);
		//glBindSampler(0, Sampler);
		//GLint uniform_mytexture = glGetUniformLocation(selected_shader_program, "tex0");
		//glUniform1i(uniform_mytexture, 0);

	}

	GLuint programm_id;
	ShaderUniformVariable uniform_variables[MAX_SHADER_UNIFORMS];
	int num_uniforms;
};




//------------------------------------------------------------------------------------------------------------------------------------------------------voids
void CheckDebugLog();

// This is our MAIN() for windows
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprev, PSTR cmdline, int ishow);

// The window proc which handles all of window's messages.
LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// This controls our main program loop
WPARAM MainLoop();

// This changes the screen to full screen mode
void ChangeToFullScreen();

// This is our own function that makes creating a window modular and easy
HWND CreateMyWindow(LPSTR strWindowName, int width, int height, DWORD dwStyle, bool bFullScreen, HINSTANCE hInstance);

// This allows us to configure our window for OpenGL and back buffering
bool bSetupPixelFormat(HDC hdc);

// This inits our screen translations and projections
void SizeOpenGLScreen(int width, int height);

// This sets up OpenGL
void InitializeOpenGL(int width, int height);

// This initializes the whole program
void Init(HWND hWnd);

// This draws everything to the screen
void RenderScene();

// This frees all our memory in our program
void DeInit();

void ShutDown();

void process_key(int key);

extern bool opengl_debug_mode_enabled;

#endif