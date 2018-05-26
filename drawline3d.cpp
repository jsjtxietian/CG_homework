#include<GL/freeglut.h>
#include <xutility>
#include <iostream>
#include <vector>
#include <algorithm>

//点
typedef struct vector3
{
	int x;
	int y;
	int z;

	vector3(int i, int j, int k)
		:x(i), y(j), z(k)
	{}

	vector3()
		:x(0), y(0), z(0)
	{}

}Vector3;

//旋转参数
static GLfloat xRot = 0.0f;
static GLfloat yRot = 0.0f;
std::vector<Vector3> points;

Vector3 start(0, 0, 0);
Vector3 end(25, 17, 5);

void calcPoint(Vector3 start, Vector3 end);


void drawCube(float x, float y, float z)
{
	const float sizex = 0.05f;
	const float sizey = 0.05f;
	const float sizez = 0.05f;

	glTranslatef(x, y, z);

	glBegin(GL_QUADS);

	glColor3f(1.0, 1.0, 0.0);

	// FRONT
	glVertex3f(-sizex, -sizey, sizez);
	glVertex3f(sizex, -sizey, sizez);
	glVertex3f(sizex, sizey, sizez);
	glVertex3f(-sizex, sizey, sizez);

	// BACK
	glVertex3f(-sizex, -sizey, -sizez);
	glVertex3f(-sizex, sizey, -sizez);
	glVertex3f(sizex, sizey, -sizez);
	glVertex3f(sizex, -sizey, -sizez);

	glColor3f(0.0, 1.0, 0.0);

	// LEFT
	glVertex3f(-sizex, -sizey, sizez);
	glVertex3f(-sizex, sizey, sizez);
	glVertex3f(-sizex, sizey, -sizez);
	glVertex3f(-sizex, -sizey, -sizez);

	// RIGHT
	glVertex3f(sizex, -sizey, -sizez);
	glVertex3f(sizex, sizey, -sizez);
	glVertex3f(sizex, sizey, sizez);
	glVertex3f(sizex, -sizey, sizez);

	glColor3f(0.0, 0.0, 1.0);

	// TOP
	glVertex3f(-sizex, sizey, sizez);
	glVertex3f(sizex, sizey, sizez);
	glVertex3f(sizex, sizey, -sizez);
	glVertex3f(-sizex, sizey, -sizez);

	// BOTTOM
	glVertex3f(-sizex, -sizey, sizez);
	glVertex3f(-sizex, -sizey, -sizez);
	glVertex3f(sizex, -sizey, -sizez);
	glVertex3f(sizex, -sizey, sizez);

	glEnd();

	glTranslatef(-x, -y, -z);
}

void paint(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	//旋转
	glRotatef(xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(yRot, 0.0f, 1.0f, 0.0f);

	calcPoint(start ,end);

	//0.1间隔代表1 否则太大
	for(auto a : points)
	{
		drawCube((float)a.x/10, (float)a.y / 10, (float)a.z / 10);
	}

	glPopMatrix();
	glutSwapBuffers();
}

void SpecialKeys(int key, int x, int y)
{
	if (key == GLUT_KEY_UP)   xRot -= 5.0f;
	if (key == GLUT_KEY_DOWN)  xRot += 5.0f;
	if (key == GLUT_KEY_LEFT)  yRot -= 5.0f;
	if (key == GLUT_KEY_RIGHT)  yRot += 5.0f;

	if (xRot > 356.0f)  xRot = 0.0f;
	if (xRot < -1.0f)  xRot = 355.0f;
	if (yRot > 356.0f)  yRot = 0.0f;
	if (yRot < -1.0f)  yRot = 355.0f;

	//刷新窗口
	glutPostRedisplay();
}

void reshapeFunction(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-2.0, 2.0, -2.0, 2.0, -8.0, 8.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(3.0, 3.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void Init()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.3, 0.3, 0.3, 1.0f);
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Bresenham3D ");
	Init();

	glutReshapeFunc(reshapeFunction);
	glutDisplayFunc(paint);
	glutSpecialFunc(SpecialKeys);
	glutMainLoop();
	return 0;
}


void calcPoint(Vector3 start , Vector3 end)
{
	Vector3 result;
	int steps = 1;

	int xd, yd, zd;
	int x, y, z;
	int ax, ay, az;
	int sx, sy, sz;
	int dx, dy, dz;

	dx = (int)(end.x - start.x);
	dy = (int)(end.y - start.y);
	dz = (int)(end.z - start.z);

	ax = abs(dx) << 1;
	ay = abs(dy) << 1;
	az = abs(dz) << 1;

	sx = ((float)dx) >= 0 ? 1 : -1;
	sy = ((float)dy) >= 0 ? 1 : -1;
	sz = ((float)dz) >= 0 ? 1 : -1;

	x = (int)start.x;
	y = (int)start.y;
	z = (int)start.z;

	if (ax >= std::max(ay, az)) // x dominant
	{
		yd = ay - (ax >> 1);
		zd = az - (ax >> 1);
		for (; ; )
		{
			result.x = (int)(x / steps);
			result.y = (int)(y / steps);
			result.z = (int)(z / steps);
			points.push_back(result);

			if (x == (int)end.x)
				return;

			if (yd >= 0)
			{
				y += sy;
				yd -= ax;
			}

			if (zd >= 0)
			{
				z += sz;
				zd -= ax;
			}

			x += sx;
			yd += ay;
			zd += az;
		}
	}
	else if (ay >= std::max(ax, az)) // y dominant
	{
		xd = ax - (ay >> 1);
		zd = az - (ay >> 1);
		for (; ; )
		{
			result.x = (int)(x / steps);
			result.y = (int)(y / steps);
			result.z = (int)(z / steps);
			points.push_back(result);

			if (y == (int)end.y)
				return;

			if (xd >= 0)
			{
				x += sx;
				xd -= ay;
			}

			if (zd >= 0)
			{
				z += sz;
				zd -= ay;
			}

			y += sy;
			xd += ax;
			zd += az;
		}
	}
	else if (az >= std::max(ax, ay)) // z dominant
	{
		xd = ax - (az >> 1);
		yd = ay - (az >> 1);
		for (; ; )
		{
			result.x = (int)(x / steps);
			result.y = (int)(y / steps);
			result.z = (int)(z / steps);
			points.push_back(result);

			if (z == (int)end.z)
				return;

			if (xd >= 0)
			{
				x += sx;
				xd -= az;
			}

			if (yd >= 0)
			{
				y += sy;
				yd -= az;
			}

			z += sz;
			xd += ax;
			yd += ay;
		}
	}
}
