#include "src/main/main.h"


//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------main loop, key handler

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG    lRet = 0;

	switch (uMsg)
	{
	case WM_SIZE:										// If the window is resized
		if (!g_bFullScreen)								// Do this only if we are NOT in full screen
		{
			SizeOpenGLScreen(LOWORD(lParam), HIWORD(lParam));// LoWord=Width, HiWord=Height
			GetClientRect(hWnd, &g_rRect);				// Get the window rectangle
		}
		break;


	case WM_KEYDOWN:
		switch (wParam) {							// Check if we hit a key
		case VK_ESCAPE:								// If we hit the escape key
			PostQuitMessage(0);						// Send a QUIT message to the window
			break;
		default:
			process_key(wParam);
			break;
		}
		break;


	case WM_DESTROY:									// If the window is destroyed
		PostQuitMessage(0);								// Send a QUIT Message to the window
		break;

	default:											// Return by default
		lRet = (long)DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}
	return lRet;										// Return by default
}


WPARAM MainLoop()
{
	MSG msg;

	while (1)											// Do our infinite loop
	{													// Check if there was a message
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)					// If the message wasn't to quit
				break;
			TranslateMessage(&msg);						// Find out what the message does
			DispatchMessage(&msg);						// Execute the message
		}
		else											// If there wasn't a message
		{
			RenderScene();								// Render the scene every frame 
		}
	}

	if (opengl_debug_mode_enabled)
		CheckDebugLog();
	DeInit();											// Clean up and free all allocated memory

	ShutDown();

	return(msg.wParam);									// Return from the program
}


//-------------------------------------------------------------------------------------------------------------------------------------app init
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprev, PSTR cmdline, int ishow);

int main(int argc, char* argv[])
{
	HINSTANCE hInst = (HINSTANCE)GetWindowLong(GetActiveWindow(), GWL_HINSTANCE);
	WinMain(hInst, NULL, NULL, 1);
	return 0;
}

void ChangeToFullScreen()
{
	DEVMODE dmSettings;									// Device Mode variable
	memset(&dmSettings, 0, sizeof(dmSettings));			// Makes Sure Memory's Cleared
	if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmSettings))
	{
		MessageBox(NULL, "Could Not Enum Display Settings", "Error", MB_OK);
		return;
	}

	dmSettings.dmPelsWidth = SCREEN_WIDTH;				// Selected Screen Width
	dmSettings.dmPelsHeight = SCREEN_HEIGHT;			// Selected Screen Height
	int result = ChangeDisplaySettings(&dmSettings, CDS_FULLSCREEN);

	if (result != DISP_CHANGE_SUCCESSFUL)
	{
		MessageBox(NULL, "Display Mode Not Compatible", "Error", MB_OK);
		PostQuitMessage(0);
	}
}

HWND CreateMyWindow(LPSTR strWindowName, int width, int height, DWORD dwStyle, bool bFullScreen, HINSTANCE hInstance)
{
	HWND hWnd;
	WNDCLASS wndclass;

	memset(&wndclass, 0, sizeof(WNDCLASS));				// Init the size of the class
	wndclass.style = CS_HREDRAW | CS_VREDRAW;			// Regular drawing capabilities
	wndclass.lpfnWndProc = WinProc;						// Pass our function pointer as the window procedure
	wndclass.hInstance = hInstance;						// Assign our hInstance
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	// General icon
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);		// An arrow for the cursor
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);	// A white window
	wndclass.lpszClassName = "Engine";			// Assign the class name

	RegisterClass(&wndclass);							// Register the class

	if (bFullScreen && !dwStyle) 						// Check if we wanted full screen mode
	{													// Set the window properties for full screen mode
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		ChangeToFullScreen();							// Go to full screen
		ShowCursor(FALSE);								// Hide the cursor
	}
	else if (!dwStyle)									// Assign styles to the window depending on the choice
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	g_hInstance = hInstance;							// Assign our global hInstance to the window's hInstance

	RECT rWindow;
	rWindow.left = 0;								// Set Left Value To 0
	rWindow.right = width;							// Set Right Value To Requested Width
	rWindow.top = 0;								// Set Top Value To 0
	rWindow.bottom = height;							// Set Bottom Value To Requested Height

	AdjustWindowRect(&rWindow, dwStyle, false);		// Adjust Window To True Requested Size

													// Create the window
	hWnd = CreateWindow("Engine", strWindowName, dwStyle, 0, 0,
		rWindow.right - rWindow.left, rWindow.bottom - rWindow.top,
		NULL, NULL, hInstance, NULL);

	if (!hWnd) return NULL;								// If we could get a handle, return NULL

	ShowWindow(hWnd, SW_SHOWNORMAL);					// Show the window
	UpdateWindow(hWnd);									// Draw the window

	SetFocus(hWnd);										// Sets Keyboard Focus To The Window	

	return hWnd;
}

bool bSetupPixelFormat(HDC hdc)
{
	int pixelformat, format;
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;

	if ((pixelformat = ChoosePixelFormat(hdc, &pfd)) == FALSE)
	{
		MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
		return FALSE;
	}

	if (SetPixelFormat(hdc, pixelformat, &pfd) == FALSE)
	{
		MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
		return FALSE;
	}

	return TRUE;
}



#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

void InitializeOpenGL(int width, int height)
{
	g_hDC = GetDC(g_hWnd);								// This sets our global HDC
														// We don't free this hdc until the end of our program
	if (!bSetupPixelFormat(g_hDC))						// This sets our pixel format/information
		PostQuitMessage(0);							    // If there's an error, quit

	g_hRC = wglCreateContext(g_hDC);					// This creates a rendering context from our hdc
	wglMakeCurrent(g_hDC, g_hRC);						// This makes the rendering context we just created the one we want to use

//get max gl version
	int  major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	//major = 3; minor = 0;

	typedef HGLRC(APIENTRYP PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int *);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	int attributes[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB |
		WGL_CONTEXT_DEBUG_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
		wglGetProcAddress("wglCreateContextAttribsARB");

	if (NULL == wglCreateContextAttribsARB)
	{
		wglDeleteContext(g_hRC);
		return;
	}

	HGLRC hRC = wglCreateContextAttribsARB(g_hDC, 0, attributes);
	if (!hRC)
	{
		wglDeleteContext(g_hRC);
		return;
	}
	if (!wglMakeCurrent(g_hDC, hRC))
	{
		wglDeleteContext(g_hRC);
		wglDeleteContext(hRC);
		return;
	}
	wglDeleteContext(g_hRC);
}


void DeInit()
{
	if (g_hRC)
	{
		wglMakeCurrent(NULL, NULL);						// This frees our rendering memory and sets everything back to normal
		wglDeleteContext(g_hRC);						// Delete our OpenGL Rendering Context	
	}

	if (g_hDC)
		ReleaseDC(g_hWnd, g_hDC);						// Release our HDC from memory

	if (g_bFullScreen)									// If we were in full screen:
	{
		ChangeDisplaySettings(NULL, 0);					// Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	UnregisterClass("Engine", g_hInstance);				// Free the window class

	PostQuitMessage(0);									// Post a QUIT message to the window
}


void set_vsync(bool enabled)
{
	typedef BOOL(APIENTRY * wglSwapIntervalEXT_Func)(int);
	wglSwapIntervalEXT_Func wglSwapIntervalEXT = wglSwapIntervalEXT_Func(wglGetProcAddress("wglSwapIntervalEXT"));
	if (wglSwapIntervalEXT) wglSwapIntervalEXT(enabled ? 1 : 0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprev, PSTR cmdline, int ishow)
{
	HWND hWnd;
	g_bFullScreen = false;

	hWnd = CreateMyWindow("Frustum culling", SCREEN_WIDTH, SCREEN_HEIGHT, 0, g_bFullScreen, hInstance);
	if (hWnd == NULL) return TRUE;

	Init(hWnd);
	set_vsync(false);

	return (int)MainLoop();
}