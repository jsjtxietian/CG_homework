#pragma comment(lib, "winmm.lib")

#include "camera.h"

extern HWND g_hWnd;

// Our global float that stores the elapsed time between the current and last frame
double g_FrameInterval = 0.0f;


void CalculateFrameRate(bool show_framerate)
{
	static float framesPerSecond    = 0.0f;		// This will store our fps
    static double lastTime			= 0.0f;		// This will hold the time from the last frame
	static char strFrameRate[50] = {0};			// We will store the string here for the window title

	static double frameTime = 0.0f;				// This stores the last frame's time

	// Get the current time in seconds
    double currentTime = timeGetTime() * 0.001f;				

	// Here we store the elapsed time between the current and last frame,
	// then keep the current frame in our static variable for the next frame.
 	g_FrameInterval = currentTime - frameTime;
	frameTime = currentTime;

	// Increase the frame counter
    ++framesPerSecond;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
    if( currentTime - lastTime > 1.0f )
    {
		// Here we set the lastTime to the currentTime
	    lastTime = currentTime;
		
		// Copy the frames per second into a string to display in the window title bar
		float time_in_ms = 1000.f / (float)framesPerSecond;
		sprintf(strFrameRate, "FPS: %d (%.2f ms)", int(framesPerSecond), time_in_ms);

		// Set the window title bar to our string
		if (show_framerate)
			SetWindowText(g_hWnd, strFrameRate);

		// Reset the frames per second
        framesPerSecond = 0;
    }
}


CCamera::CCamera()
{
	m_vPosition = vec3(0.0, 0.0, 0.0);
	m_vView = vec3(0.0, 0.5, 1.0);
	m_vUpVector = vec3(0.0, 1.0, 0.0);

	kSpeed = 10.f;

	show_framerate = true;
}


void CCamera::PositionCamera(float positionX, float positionY, float positionZ,
				  		     float viewX,     float viewY,     float viewZ,
							 float upVectorX, float upVectorY, float upVectorZ)
{
	m_vPosition  = vec3(positionX, positionY, positionZ);
	m_vView  = vec3(viewX, viewY, viewZ);
	m_vUpVector = vec3(upVectorX, upVectorY, upVectorZ);
}

vec3 RotateAroundVector(const vec3 &B ,const vec3 &R, float delta)
{
  vec3 dz = R * (B * R);
  vec3 dx = B - dz;
  vec3 dy = dx ^ R;
  return dx*cosf(delta)+dy*sinf(delta)+dz;
}

void CCamera::SetViewByMouse()
{
//rotate camera only when we pressed right mouse button
	static int mouseX = 0;
	static int mouseY = 0;
	static bool r_button_pressed = false;
	if ((GetKeyState(VK_RBUTTON) & 0x80) && !r_button_pressed)
	{
		POINT mousePos;
		GetCursorPos(&mousePos);
		mouseX = mousePos.x;
		mouseY = mousePos.y;
	}
	r_button_pressed = (GetKeyState(VK_RBUTTON) & 0x80);
	if (!r_button_pressed)
		return;


	POINT mousePos;
	float angleY = 0.0f; // for looking up or down
	float angleZ = 0.0f; // to rotate around the Y axis (Left and Right)
	static float currentRotX = 0.0f;
	
	// get the mouse's current X,Y position
	GetCursorPos(&mousePos);						
	if( (mousePos.x == mouseX) && (mousePos.y == mouseY) ) return;

	// restore mouse position
	SetCursorPos(mouseX, mouseY);							

//roate camera view
	angleY = (float)( (mousePos.x - mouseX) ) / 200.0f;
	angleZ = (float)( (mousePos.y - mouseY) ) / 200.0f;

	static float lastRotX = 0.0f; 
 	lastRotX = currentRotX; // We store off the currentRotX and will use it in when the angle is capped
	
	// Here we keep track of the current rotation (for up and down) so that
	// we can restrict the camera from doing a full 360 loop.
	currentRotX += angleZ;
 

	vec3 vView = m_vView - m_vPosition;
	vec3 new_dir = RotateAroundVector(vView, vec3(0,1,0), angleY);
	vec3 vAxis = new_dir ^ m_vUpVector;
	vAxis.y = 0;
	vAxis.normalize();
	vec3 new_dir2 = RotateAroundVector(new_dir, vAxis, angleZ);
	m_vView = m_vPosition + new_dir2;

	m_vUpVector = vAxis^new_dir2;
	m_vUpVector.normalize();
}


void CCamera::RotateView(float angle, float x, float y, float z)
{
	vec3 vNewView;

	// Get the view vector (The direction we are facing)
	vec3 vView = m_vView - m_vPosition;		

	// Calculate the sine and cosine of the angle once
	float cosTheta = (float)cos(angle);
	float sinTheta = (float)sin(angle);

	// Find the new x position for the new rotated point
	vNewView.x  = (cosTheta + (1 - cosTheta) * x * x)		* vView.x;
	vNewView.x += ((1 - cosTheta) * x * y - z * sinTheta)	* vView.y;
	vNewView.x += ((1 - cosTheta) * x * z + y * sinTheta)	* vView.z;

	// Find the new y position for the new rotated point
	vNewView.y  = ((1 - cosTheta) * x * y + z * sinTheta)	* vView.x;
	vNewView.y += (cosTheta + (1 - cosTheta) * y * y)		* vView.y;
	vNewView.y += ((1 - cosTheta) * y * z - x * sinTheta)	* vView.z;

	// Find the new z position for the new rotated point
	vNewView.z  = ((1 - cosTheta) * x * z - y * sinTheta)	* vView.x;
	vNewView.z += ((1 - cosTheta) * y * z + x * sinTheta)	* vView.y;
	vNewView.z += (cosTheta + (1 - cosTheta) * z * z)		* vView.z;

	// Now we just add the newly rotated vector to our position to set
	// our new rotated view of our camera.
	m_vView = m_vPosition + vNewView;
}


void CCamera::StrafeCamera(float speed)
{	
	// Add the strafe vector to our position
	m_vPosition.x += m_vStrafe.x * speed;
	m_vPosition.z += m_vStrafe.z * speed;

	// Add the strafe vector to our view
	m_vView.x += m_vStrafe.x * speed;
	m_vView.z += m_vStrafe.z * speed;
}

void CCamera::MoveCamera(float speed)
{
	// Get the current view vector (the direction we are looking)
	vec3 vVector = m_vView - m_vPosition;
	vVector.normalize();
	m_vPosition += vVector * speed;
	m_vView += vVector * speed;
}



void CCamera::CheckForMovement()
{	
	// Once we have the frame interval, we find the current speed
	float speed = kSpeed * g_FrameInterval;

	if(GetKeyState('W') & 0x80) {				
		MoveCamera(speed);				
	}

	if(GetKeyState('S') & 0x80) {			
		MoveCamera(-speed);				
	}

	if(GetKeyState('A') & 0x80) {			
		StrafeCamera(-speed);
	}

	if(GetKeyState('D') & 0x80) {			
		StrafeCamera(speed);
	}	
}


void CCamera::Update() 
{
	// Calculate our frame rate and set our frame interval for time-based movement
	CalculateFrameRate(show_framerate);

	// Move the camera's view by the mouse
	SetViewByMouse();

	// This checks to see if the keyboard was pressed
	CheckForMovement();

	// Initialize a variable for the cross product result
	vec3 vCross = (m_vView - m_vPosition) ^ m_vUpVector;

	// Normalize the strafe vector
	m_vStrafe = vCross;
	m_vStrafe.normalize();
}