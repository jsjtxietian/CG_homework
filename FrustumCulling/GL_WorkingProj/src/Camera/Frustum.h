#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include <windows.h>
#include <gl\gl.h>
#include <gl\glext.h>
#include "..\glext\glext.h"
#include "..\math\mathlib.h"


class CFrustum 
{
public:
	CFrustum(){};
	~CFrustum(){};

	void CalculateFrustum(mat4 &view_matrix, mat4 &proj_matrix);

	vec4 frustum_planes[6];
};

#endif