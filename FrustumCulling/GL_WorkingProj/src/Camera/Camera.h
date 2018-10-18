#ifndef _CAMERA_H
#define _CAMERA_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gl\gl.h>
#include <gl\glu.h>

#include "..\math\mathlib.h"


// This is our camera class
class CCamera {

public:
	CCamera();

	// These are are data access functions for our camera's private data
	vec3 Position() {	return m_vPosition;		}
	vec3 View()		{	return m_vView;			}
	vec3 UpVector() {	return m_vUpVector;		}
	vec3 Strafe()	{	return m_vStrafe;		}
	
	// This changes the position, view, and up vector of the camera.
	// This is primarily used for initialization
	void PositionCamera(float positionX, float positionY, float positionZ,
			 		    float viewX,     float viewY,     float viewZ,
						float upVectorX, float upVectorY, float upVectorZ);

	// This rotates the camera's view around the position depending on the values passed in.
	void RotateView(float angle, float X, float Y, float Z);

	// This moves the camera's view by the mouse movements (First person view)
	void SetViewByMouse(); 

	// This strafes the camera left or right depending on the speed (+/-) 
	void StrafeCamera(float speed);

	// This will move the camera forward or backward depending on the speed
	void MoveCamera(float speed);

	// This checks for keyboard movement 
	void CheckForMovement();

	// This updates the camera's view and other data (Should be called each frame)
	void Update();

	bool show_framerate;

private:
	vec3 m_vPosition; // The camera's position
	vec3 m_vView; // The camera's view
	vec3 m_vUpVector; // The camera's up vector
	vec3 m_vStrafe; // The camera's strafe vector
	float kSpeed; // This is how fast our camera moves
};

#endif