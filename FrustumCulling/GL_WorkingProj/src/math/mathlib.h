#ifndef __MATHLIB_H__
#define __MATHLIB_H__

#include <math.h>

#define EPSILON 1e-6f
#define PI 3.14159265358979323846f
#define PI2 6.28318530717958647692f
#define PI1_2 1.570796326794897f
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

#define MAX(a,b)					((a < b) ? (b) : (a))
#define MIN(a,b)					((a < b) ? (a) : (b))
#define CLAMP(value,a,b)			(MIN(MAX((value), a),b))


#define MIX(x,y, a)					((x) * (1 - (a)) + (y) * (a))
#define FRACT(x)					(x - int(x))

struct vec2;
struct vec3;
struct vec4;
struct mat3;
struct mat4;



/*****************************************************************************/
/*                                                                           */
/* vec2                                                                      */
/*                                                                           */
/*****************************************************************************/

struct vec2 {

	inline vec2() : x(0), y(0) { }
	inline vec2(float x, float y) : x(x), y(y) { }
	inline vec2(const float *v) : x(v[0]), y(v[1]) { }
	inline vec2(const vec2 &v) : x(v.x), y(v.y) { }

	inline int operator==(const vec2 &v) { return (fabs(x - v.x) < EPSILON && fabs(y - v.y) < EPSILON); }
	inline int operator!=(const vec2 &v) { return !(*this == v); }

	inline const vec2 operator*(float f) const { return vec2(x * f, y * f); }
	inline const vec2 operator/(float f) const { return vec2(x / f, y / f); }
	inline const vec2 operator+(const vec2 &v) const { return vec2(x + v.x, y + v.y); }
	inline const vec2 operator-() const { return vec2(-x, -y); }
	inline const vec2 operator-(const vec2 &v) const { return vec2(x - v.x, y - v.y); }

	inline vec2 &operator*=(float f) { return *this = *this * f; }
	inline vec2 &operator/=(float f) { return *this = *this / f; }
	inline vec2 &operator+=(const vec2 &v) { return *this = *this + v; }
	inline vec2 &operator-=(const vec2 &v) { return *this = *this - v; }

	inline float operator*(const vec2 &v) const { return x * v.x + y * v.y; }

	inline operator float*() { return (float*)&x; }
	inline operator const float*() const { return (float*)&x; }

	inline vec2 mul(const vec2 &v) const {
		return vec2(x * v.x, y * v.y);
	}


	inline float &operator[](int i) { return ((float*)&x)[i]; }
	inline const float operator[](int i) const { return ((float*)&x)[i]; }

	inline float length() const { return sqrtf(x * x + y * y); }
	inline float length2() const { return (x * x + y * y); }
	inline float normalize() {
		float inv, length = sqrtf(x * x + y * y);
		if (length < EPSILON) return 0.0;
		inv = 1.0f / length;
		x *= inv;
		y *= inv;
		return length;
	}

	union {
		struct {
			float x, y;
		};
		float v[2];
	};
};



/*****************************************************************************/
/*                                                                           */
/* vec3                                                                      */
/*                                                                           */
/*****************************************************************************/

struct vec3 {

	inline vec3() : x(0), y(0), z(0) { }
	inline vec3(float x, float y, float z) : x(x), y(y), z(z) { }
	inline vec3(const float *v) : x(v[0]), y(v[1]), z(v[2]) { }
	inline vec3(const vec3 &v) : x(v.x), y(v.y), z(v.z) { }
	inline vec3(vec2 v, float z) : x(v.x), y(v.y), z(z) { }
	inline vec3(const vec4 &v);

	inline int operator==(const vec3 &v) { return (fabs(x - v.x) < EPSILON && fabs(y - v.y) < EPSILON && fabs(z - v.z) < EPSILON); }
	inline int operator!=(const vec3 &v) { return !(*this == v); }

	inline const vec3 operator*(float f) const { return vec3(x * f, y * f, z * f); }
	inline const vec3 operator/(float f) const { return vec3(x / f, y / f, z / f); }
	inline const vec3 operator/(vec3 f) const { return vec3(x / f.x, y / f.y, z / f.z); }
	inline const vec3 operator+(const vec3 &v) const { return vec3(x + v.x, y + v.y, z + v.z); }
	inline const vec3 operator-() const { return vec3(-x, -y, -z); }
	inline const vec3 operator-(const vec3 &v) const { return vec3(x - v.x, y - v.y, z - v.z); }

	inline vec3 &operator*=(float f) { return *this = *this * f; }
	inline vec3 &operator/=(float f) { return *this = *this / f; }
	inline vec3 &operator+=(const vec3 &v) { return *this = *this + v; }
	inline vec3 &operator-=(const vec3 &v) { return *this = *this - v; }

	inline float operator*(const vec3 &v) const { return x * v.x + y * v.y + z * v.z; }
	inline float operator*(const vec4 &v) const;
	inline vec3 operator^(const vec3 &v) const {
		return vec3
			(
			y * v.z - z * v.y,
			z * v.x - x * v.z,
			x * v.y - y * v.x
			);
	}

	inline operator float*() { return (float*)&x; }
	inline operator const float*() const { return (float*)&x; }

	inline float &operator[](int i) { return ((float*)&x)[i]; }
	inline const float operator[](int i) const { return ((float*)&x)[i]; }

	inline float length() const { return sqrtf(x * x + y * y + z * z); }
	inline float length2() const { return x * x + y * y + z * z; }
	inline float normalize() {
		float len = sqrtf(x * x + y * y + z * z);
		if (len < EPSILON) return 0.0;
		float inv = 1.0f / len;
		x *= inv;
		y *= inv;
		z *= inv;
		return len;
	}
	inline vec3 get_min(const vec3 &v) const {
		return vec3(MIN(x, v.x), MIN(y, v.y), MIN(z, v.z));
	}
	inline vec3 get_max(const vec3 &v) const {
		return vec3(MAX(x, v.x), MAX(y, v.y), MAX(z, v.z));
	}

	inline vec3 mul(const vec3 &v) const {
		return vec3(x * v.x, y * v.y, z * v.z);
	}
	inline void cross(const vec3 &v1, const vec3 &v2) {
		x = v1.y * v2.z - v1.z * v2.y;
		y = v1.z * v2.x - v1.x * v2.z;
		z = v1.x * v2.y - v1.y * v2.x;
	}

	union {
		struct {
			float x, y, z;
		};
		float v[3];
	};
};

inline vec3 normalize(const vec3 &v) {
	float length = v.length();
	if (length < EPSILON) return vec3(0, 0, 0);
	return v / length;
}

inline vec3 cross(const vec3 &v1, const vec3 &v2) {
	vec3 ret;
	ret.x = v1.y * v2.z - v1.z * v2.y;
	ret.y = v1.z * v2.x - v1.x * v2.z;
	ret.z = v1.x * v2.y - v1.y * v2.x;
	return ret;
}

inline vec3 saturate(const vec3 &v) {
	vec3 ret = v;
	if (ret.x < 0.0) ret.x = 0.0;
	else if (ret.x > 1.0f) ret.x = 1.0f;
	if (ret.y < 0.0) ret.y = 0.0;
	else if (ret.y > 1.0f) ret.y = 1.0f;
	if (ret.z < 0.0) ret.z = 0.0;
	else if (ret.z > 1.0f) ret.z = 1.0f;
	return ret;
}


/*****************************************************************************/
/*                                                                           */
/* vec4                                                                      */
/*                                                                           */
/*****************************************************************************/

struct vec4 {

	inline vec4() : x(0), y(0), z(0), w(0) { } //w(1)
	inline vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }
	inline vec4(const float *v) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) { }
	inline vec4(const vec3 &v) : x(v.x), y(v.y), z(v.z), w(1) { }
	inline vec4(const vec3 &v, float w) : x(v.x), y(v.y), z(v.z), w(w) { }
	inline vec4(const vec4 &v) : x(v.x), y(v.y), z(v.z), w(v.w) { }
	inline vec4(const vec2 &v1, const vec2 &v2) : x(v1.x), y(v1.y), z(v2.x), w(v2.y) { }

	inline bool operator==(const vec4 &v) { return (fabs(x - v.x) < EPSILON && fabs(y - v.y) < EPSILON && fabs(z - v.z) < EPSILON && fabs(w - v.w) < EPSILON); }
	inline bool operator!=(const vec4 &v) { return !(*this == v); }

	inline const vec4 operator*(float f) const { return vec4(x * f, y * f, z * f, w * f); }
	inline const vec4 operator/(float f) const { return vec4(x / f, y / f, z / f, w / f); }
	inline const vec4 operator+(const vec4 &v) const { return vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
	inline const vec4 operator-() const { return vec4(-x, -y, -z, -w); }
	inline const vec4 operator-(const vec4 &v) const { return vec4(x - v.x, y - v.y, z - v.z, w - v.w); }

	inline vec4 &operator*=(float f) { return *this = *this * f; }
	inline vec4 &operator/=(float f) { return *this = *this / f; }
	inline vec4 &operator+=(const vec4 &v) { return *this = *this + v; }
	inline vec4 &operator-=(const vec4 &v) { return *this = *this - v; }
	inline vec4 mul(const vec4 &v) {
		return vec4(x * v.x, y * v.y, z * v.z, w * v.w);
	}

	inline float operator*(const vec3 &v) const { return x * v.x + y * v.y + z * v.z + w; }
	inline float operator*(const vec4 &v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

	inline operator float*() { return (float*)&x; }
	inline operator const float*() const { return (float*)&x; }

	inline float &operator[](int i) { return ((float*)&x)[i]; }
	inline const float operator[](int i) const { return ((float*)&x)[i]; }

	union {
		struct {
			float x, y, z, w;
		};
		float v[4];
	};
};

inline vec3::vec3(const vec4 &v) {
	x = v.x;
	y = v.y;
	z = v.z;
}

inline float vec3::operator*(const vec4 &v) const {
	return x * v.x + y * v.y + z * v.z + v.w;
}



/*****************************************************************************/
/*                                                                           */
/* mat4                                                                      */
/*                                                                           */
/*****************************************************************************/

struct mat4 {

	mat4() {
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	mat4(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9, float v10, float v11, float v12, float v13, float v14, float v15, float v16) {
		mat[0] = v1; mat[4] = v5; mat[8] = v9; mat[12] = v13;
		mat[1] = v2; mat[5] = v6; mat[9] = v10; mat[13] = v14;
		mat[2] = v3; mat[6] = v7; mat[10] = v11; mat[14] = v15;
		mat[3] = v4; mat[7] = v8; mat[11] = v12; mat[15] = v16;
	}
	mat4(const vec3 &v) {
		translate(v);
	}
	mat4(float x, float y, float z) {
		translate(x, y, z);
	}
	mat4(const vec3 &axis, float angle) {
		rotate(axis, angle);
	}
	mat4(float x, float y, float z, float angle) {
		rotate(x, y, z, angle);
	}
	mat4(const float *m) {
		mat[0] = m[0]; mat[4] = m[4]; mat[8] = m[8]; mat[12] = m[12];
		mat[1] = m[1]; mat[5] = m[5]; mat[9] = m[9]; mat[13] = m[13];
		mat[2] = m[2]; mat[6] = m[6]; mat[10] = m[10]; mat[14] = m[14];
		mat[3] = m[3]; mat[7] = m[7]; mat[11] = m[11]; mat[15] = m[15];
	}
	mat4(const mat4 &m) {
		mat[0] = m[0]; mat[4] = m[4]; mat[8] = m[8]; mat[12] = m[12];
		mat[1] = m[1]; mat[5] = m[5]; mat[9] = m[9]; mat[13] = m[13];
		mat[2] = m[2]; mat[6] = m[6]; mat[10] = m[10]; mat[14] = m[14];
		mat[3] = m[3]; mat[7] = m[7]; mat[11] = m[11]; mat[15] = m[15];
	}

	vec3 operator*(const vec3 &v) const {
		vec3 ret;
		ret[0] = mat[0] * v[0] + mat[4] * v[1] + mat[8] * v[2] + mat[12];
		ret[1] = mat[1] * v[0] + mat[5] * v[1] + mat[9] * v[2] + mat[13];
		ret[2] = mat[2] * v[0] + mat[6] * v[1] + mat[10] * v[2] + mat[14];
		return ret;
	}
	vec3 rotate_vector(const vec3 &v) const {
		vec3 ret;
		ret[0] = mat[0] * v[0] + mat[4] * v[1] + mat[8] * v[2];
		ret[1] = mat[1] * v[0] + mat[5] * v[1] + mat[9] * v[2];
		ret[2] = mat[2] * v[0] + mat[6] * v[1] + mat[10] * v[2];
		return ret;
	}
	vec4 operator*(const vec4 &v) const {
		vec4 ret;
		ret[0] = mat[0] * v[0] + mat[4] * v[1] + mat[8] * v[2] + mat[12] * v[3];
		ret[1] = mat[1] * v[0] + mat[5] * v[1] + mat[9] * v[2] + mat[13] * v[3];
		ret[2] = mat[2] * v[0] + mat[6] * v[1] + mat[10] * v[2] + mat[14] * v[3];
		ret[3] = mat[3] * v[0] + mat[7] * v[1] + mat[11] * v[2] + mat[15] * v[3];
		return ret;
	}
	mat4 operator*(float f) const {
		mat4 ret;
		ret[0] = mat[0] * f; ret[4] = mat[4] * f; ret[8] = mat[8] * f; ret[12] = mat[12] * f;
		ret[1] = mat[1] * f; ret[5] = mat[5] * f; ret[9] = mat[9] * f; ret[13] = mat[13] * f;
		ret[2] = mat[2] * f; ret[6] = mat[6] * f; ret[10] = mat[10] * f; ret[14] = mat[14] * f;
		ret[3] = mat[3] * f; ret[7] = mat[7] * f; ret[11] = mat[11] * f; ret[15] = mat[15] * f;
		return ret;
	}
	mat4 operator*(const mat4 &m) const {
		mat4 ret;
		ret[0] = mat[0] * m[0] + mat[4] * m[1] + mat[8] * m[2] + mat[12] * m[3];
		ret[1] = mat[1] * m[0] + mat[5] * m[1] + mat[9] * m[2] + mat[13] * m[3];
		ret[2] = mat[2] * m[0] + mat[6] * m[1] + mat[10] * m[2] + mat[14] * m[3];
		ret[3] = mat[3] * m[0] + mat[7] * m[1] + mat[11] * m[2] + mat[15] * m[3];
		ret[4] = mat[0] * m[4] + mat[4] * m[5] + mat[8] * m[6] + mat[12] * m[7];
		ret[5] = mat[1] * m[4] + mat[5] * m[5] + mat[9] * m[6] + mat[13] * m[7];
		ret[6] = mat[2] * m[4] + mat[6] * m[5] + mat[10] * m[6] + mat[14] * m[7];
		ret[7] = mat[3] * m[4] + mat[7] * m[5] + mat[11] * m[6] + mat[15] * m[7];
		ret[8] = mat[0] * m[8] + mat[4] * m[9] + mat[8] * m[10] + mat[12] * m[11];
		ret[9] = mat[1] * m[8] + mat[5] * m[9] + mat[9] * m[10] + mat[13] * m[11];
		ret[10] = mat[2] * m[8] + mat[6] * m[9] + mat[10] * m[10] + mat[14] * m[11];
		ret[11] = mat[3] * m[8] + mat[7] * m[9] + mat[11] * m[10] + mat[15] * m[11];
		ret[12] = mat[0] * m[12] + mat[4] * m[13] + mat[8] * m[14] + mat[12] * m[15];
		ret[13] = mat[1] * m[12] + mat[5] * m[13] + mat[9] * m[14] + mat[13] * m[15];
		ret[14] = mat[2] * m[12] + mat[6] * m[13] + mat[10] * m[14] + mat[14] * m[15];
		ret[15] = mat[3] * m[12] + mat[7] * m[13] + mat[11] * m[14] + mat[15] * m[15];
		return ret;
	}
	mat4 operator+(const mat4 &m) const {
		mat4 ret;
		ret[0] = mat[0] + m[0]; ret[4] = mat[4] + m[4]; ret[8] = mat[8] + m[8]; ret[12] = mat[12] + m[12];
		ret[1] = mat[1] + m[1]; ret[5] = mat[5] + m[5]; ret[9] = mat[9] + m[9]; ret[13] = mat[13] + m[13];
		ret[2] = mat[2] + m[2]; ret[6] = mat[6] + m[6]; ret[10] = mat[10] + m[10]; ret[14] = mat[14] + m[14];
		ret[3] = mat[3] + m[3]; ret[7] = mat[7] + m[7]; ret[11] = mat[11] + m[11]; ret[15] = mat[15] + m[15];
		return ret;
	}
	mat4 operator-(const mat4 &m) const {
		mat4 ret;
		ret[0] = mat[0] - m[0]; ret[4] = mat[4] - m[4]; ret[8] = mat[8] - m[8]; ret[12] = mat[12] - m[12];
		ret[1] = mat[1] - m[1]; ret[5] = mat[5] - m[5]; ret[9] = mat[9] - m[9]; ret[13] = mat[13] - m[13];
		ret[2] = mat[2] - m[2]; ret[6] = mat[6] - m[6]; ret[10] = mat[10] - m[10]; ret[14] = mat[14] - m[14];
		ret[3] = mat[3] - m[3]; ret[7] = mat[7] - m[7]; ret[11] = mat[11] - m[11]; ret[15] = mat[15] - m[15];
		return ret;
	}

	mat4 &operator*=(float f) { return *this = *this * f; }
	mat4 &operator*=(const mat4 &m) { return *this = *this * m; }
	mat4 &operator+=(const mat4 &m) { return *this = *this + m; }
	mat4 &operator-=(const mat4 &m) { return *this = *this - m; }

	operator float*() { return mat; }
	operator const float*() const { return mat; }

	float &operator[](int i) { return mat[i]; }
	const float operator[](int i) const { return mat[i]; }

	mat4 rotation() const {
		mat4 ret;
		ret[0] = mat[0]; ret[4] = mat[4]; ret[8] = mat[8]; ret[12] = 0;
		ret[1] = mat[1]; ret[5] = mat[5]; ret[9] = mat[9]; ret[13] = 0;
		ret[2] = mat[2]; ret[6] = mat[6]; ret[10] = mat[10]; ret[14] = 0;
		ret[3] = 0; ret[7] = 0; ret[11] = 0; ret[15] = 1;
		return ret;
	}
	mat4 transpose() const {
		mat4 ret;
		ret[0] = mat[0]; ret[4] = mat[1]; ret[8] = mat[2]; ret[12] = mat[3];
		ret[1] = mat[4]; ret[5] = mat[5]; ret[9] = mat[6]; ret[13] = mat[7];
		ret[2] = mat[8]; ret[6] = mat[9]; ret[10] = mat[10]; ret[14] = mat[11];
		ret[3] = mat[12]; ret[7] = mat[13]; ret[11] = mat[14]; ret[15] = mat[15];
		return ret;
	}
	mat4 transpose_rotation() const {
		mat4 ret;
		ret[0] = mat[0]; ret[4] = mat[1]; ret[8] = mat[2]; ret[12] = mat[12];
		ret[1] = mat[4]; ret[5] = mat[5]; ret[9] = mat[6]; ret[13] = mat[13];
		ret[2] = mat[8]; ret[6] = mat[9]; ret[10] = mat[10]; ret[14] = mat[14];
		ret[3] = mat[3]; ret[7] = mat[7]; ret[14] = mat[14]; ret[15] = mat[15];
		return ret;
	}

	mat4 invert_view_matrix() const {
		mat4 ret;
		//transpose rotation
		ret[0] = mat[0]; ret[4] = mat[1]; ret[8] = mat[2]; ret[12] = 0.f;
		ret[1] = mat[4]; ret[5] = mat[5]; ret[9] = mat[6]; ret[13] = 0.f;
		ret[2] = mat[8]; ret[6] = mat[9]; ret[10] = mat[10]; ret[14] = 0.f;
		ret[3] = 0.f; ret[7] = 0.f; ret[11] = 0.f; ret[15] = 1.f;

		mat4 m_translate;
		m_translate.translate(-vec3(mat[12], mat[13], mat[14]));
		return ret * m_translate;
	}

	float det() const {
		float det;
		det = mat[0] * mat[5] * mat[10];
		det += mat[4] * mat[9] * mat[2];
		det += mat[8] * mat[1] * mat[6];
		det -= mat[8] * mat[5] * mat[2];
		det -= mat[4] * mat[1] * mat[10];
		det -= mat[0] * mat[9] * mat[6];
		return det;
	}



	mat4 inverse()
	{
		mat4 res;
		double inv[16], det;
		int i;

		const float *m = &mat[0];

		inv[0] = m[5] * m[10] * m[15] -
			m[5] * m[11] * m[14] -
			m[9] * m[6] * m[15] +
			m[9] * m[7] * m[14] +
			m[13] * m[6] * m[11] -
			m[13] * m[7] * m[10];

		inv[4] = -m[4] * m[10] * m[15] +
			m[4] * m[11] * m[14] +
			m[8] * m[6] * m[15] -
			m[8] * m[7] * m[14] -
			m[12] * m[6] * m[11] +
			m[12] * m[7] * m[10];

		inv[8] = m[4] * m[9] * m[15] -
			m[4] * m[11] * m[13] -
			m[8] * m[5] * m[15] +
			m[8] * m[7] * m[13] +
			m[12] * m[5] * m[11] -
			m[12] * m[7] * m[9];

		inv[12] = -m[4] * m[9] * m[14] +
			m[4] * m[10] * m[13] +
			m[8] * m[5] * m[14] -
			m[8] * m[6] * m[13] -
			m[12] * m[5] * m[10] +
			m[12] * m[6] * m[9];

		inv[1] = -m[1] * m[10] * m[15] +
			m[1] * m[11] * m[14] +
			m[9] * m[2] * m[15] -
			m[9] * m[3] * m[14] -
			m[13] * m[2] * m[11] +
			m[13] * m[3] * m[10];

		inv[5] = m[0] * m[10] * m[15] -
			m[0] * m[11] * m[14] -
			m[8] * m[2] * m[15] +
			m[8] * m[3] * m[14] +
			m[12] * m[2] * m[11] -
			m[12] * m[3] * m[10];

		inv[9] = -m[0] * m[9] * m[15] +
			m[0] * m[11] * m[13] +
			m[8] * m[1] * m[15] -
			m[8] * m[3] * m[13] -
			m[12] * m[1] * m[11] +
			m[12] * m[3] * m[9];

		inv[13] = m[0] * m[9] * m[14] -
			m[0] * m[10] * m[13] -
			m[8] * m[1] * m[14] +
			m[8] * m[2] * m[13] +
			m[12] * m[1] * m[10] -
			m[12] * m[2] * m[9];

		inv[2] = m[1] * m[6] * m[15] -
			m[1] * m[7] * m[14] -
			m[5] * m[2] * m[15] +
			m[5] * m[3] * m[14] +
			m[13] * m[2] * m[7] -
			m[13] * m[3] * m[6];

		inv[6] = -m[0] * m[6] * m[15] +
			m[0] * m[7] * m[14] +
			m[4] * m[2] * m[15] -
			m[4] * m[3] * m[14] -
			m[12] * m[2] * m[7] +
			m[12] * m[3] * m[6];

		inv[10] = m[0] * m[5] * m[15] -
			m[0] * m[7] * m[13] -
			m[4] * m[1] * m[15] +
			m[4] * m[3] * m[13] +
			m[12] * m[1] * m[7] -
			m[12] * m[3] * m[5];

		inv[14] = -m[0] * m[5] * m[14] +
			m[0] * m[6] * m[13] +
			m[4] * m[1] * m[14] -
			m[4] * m[2] * m[13] -
			m[12] * m[1] * m[6] +
			m[12] * m[2] * m[5];

		inv[3] = -m[1] * m[6] * m[11] +
			m[1] * m[7] * m[10] +
			m[5] * m[2] * m[11] -
			m[5] * m[3] * m[10] -
			m[9] * m[2] * m[7] +
			m[9] * m[3] * m[6];

		inv[7] = m[0] * m[6] * m[11] -
			m[0] * m[7] * m[10] -
			m[4] * m[2] * m[11] +
			m[4] * m[3] * m[10] +
			m[8] * m[2] * m[7] -
			m[8] * m[3] * m[6];

		inv[11] = -m[0] * m[5] * m[11] +
			m[0] * m[7] * m[9] +
			m[4] * m[1] * m[11] -
			m[4] * m[3] * m[9] -
			m[8] * m[1] * m[7] +
			m[8] * m[3] * m[5];

		inv[15] = m[0] * m[5] * m[10] -
			m[0] * m[6] * m[9] -
			m[4] * m[1] * m[10] +
			m[4] * m[2] * m[9] +
			m[8] * m[1] * m[6] -
			m[8] * m[2] * m[5];

		det = float(m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12]);

		if (det == 0)
			res;

		det = 1.0 / det;

		for (i = 0; i < 16; i++)
			res[i] = float(inv[i] * det);

		return res;
	}


	void zero() {
		mat[0] = 0.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 0.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 0.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 0.0;
	}
	void identity() {
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}
	void rotate(const vec3 &axis, float angle) {
		float rad = angle * DEG2RAD;
		float c = cosf(rad);
		float s = sinf(rad);
		vec3 v = axis;
		v.normalize();
		float xx = v.x * v.x;
		float yy = v.y * v.y;
		float zz = v.z * v.z;
		float xy = v.x * v.y;
		float yz = v.y * v.z;
		float zx = v.z * v.x;
		float xs = v.x * s;
		float ys = v.y * s;
		float zs = v.z * s;
		mat[0] = (1.0f - c) * xx + c; mat[4] = (1.0f - c) * xy - zs; mat[8] = (1.0f - c) * zx + ys; mat[12] = 0.0;
		mat[1] = (1.0f - c) * xy + zs; mat[5] = (1.0f - c) * yy + c; mat[9] = (1.0f - c) * yz - xs; mat[13] = 0.0;
		mat[2] = (1.0f - c) * zx - ys; mat[6] = (1.0f - c) * yz + xs; mat[10] = (1.0f - c) * zz + c; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}
	void rotate(float x, float y, float z, float angle) {
		rotate(vec3(x, y, z), angle);
	}
	void rotate_x(float angle) {
		float rad = angle * DEG2RAD;
		float c = cosf(rad);
		float s = sinf(rad);
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = c; mat[9] = -s; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = s; mat[10] = c; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}
	void rotate_y(float angle) {
		float rad = angle * DEG2RAD;
		float c = cosf(rad);
		float s = sinf(rad);
		mat[0] = c; mat[4] = 0.0; mat[8] = s; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = -s; mat[6] = 0.0; mat[10] = c; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}
	void rotate_z(float angle) {
		float rad = angle * DEG2RAD;
		float c = cosf(rad);
		float s = sinf(rad);
		mat[0] = c; mat[4] = -s; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = s; mat[5] = c; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}
	void scale(const vec3 &v) {
		mat[0] = v.x; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = v.y; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = v.z; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}
	void scale(float x, float y, float z) {
		scale(vec3(x, y, z));
	}
	void translate(const vec3 &v)
	{
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = v.x;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = v.y;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = v.z;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}
	void translate(float x, float y, float z) {
		translate(vec3(x, y, z));
	}
	void set_translation(const vec3 &v) {
		mat[12] = v.x;
		mat[13] = v.y;
		mat[14] = v.z;
	}

	vec3 get_translation() {
		return vec3(mat[12], mat[13], mat[14]);
	}

	void reflect(const vec4 &plane) {
		float x = plane.x;
		float y = plane.y;
		float z = plane.z;
		float x2 = x * 2.0f;
		float y2 = y * 2.0f;
		float z2 = z * 2.0f;
		mat[0] = 1.0f - x * x2; mat[4] = -y * x2; mat[8] = -z * x2; mat[12] = -plane.w * x2;
		mat[1] = -x * y2; mat[5] = 1.0f - y * y2; mat[9] = -z * y2; mat[13] = -plane.w * y2;
		mat[2] = -x * z2; mat[6] = -y * z2; mat[10] = 1.0f - z * z2; mat[14] = -plane.w * z2;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}
	void reflect(float x, float y, float z, float w) {
		reflect(vec4(x, y, z, w));
	}

	void perspective_extended(float r, float l, float t, float b, float znear, float zfar) {
		float r_l = r - l;
		float t_b = t - b;
		if (fabs(r_l) < 0.01f) r_l = 0.01f;
		if (fabs(t_b) < 0.01f) t_b = 0.01f;
		mat[0] = 2.0f*znear / r_l; mat[4] = 0.0; mat[8] = (r + l) / r_l; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 2.0f*znear / t_b; mat[9] = (t + b) / t_b; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = -(zfar + znear) / (zfar - znear); mat[14] = -(2.0f * zfar * znear) / (zfar - znear);
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = -1.0; mat[15] = 0.0;
	}

	void perspective(float fov, float aspect, float znear, float zfar) {
		if (fabs(fov - 90.0f) < EPSILON) fov = 89.9f;
		float y = tanf(fov * PI / 360.0f);
		float x = y * aspect;
		mat[0] = 1.0f / x; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0f / y; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = -(zfar + znear) / (zfar - znear); mat[14] = -(2.0f * zfar * znear) / (zfar - znear);
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = -1.0; mat[15] = 0.0;
	}

	void OrthographicProjection(float znear, float zfar, float left, float right, float top, float bottom)
	{
		mat[0] = 2.0f / (right - left); mat[4] = 0.0; mat[8] = 0.0f; mat[12] = -(right + left) / (right - left);
		mat[1] = 0.0; mat[5] = 2.0f / (top - bottom); mat[9] = 0.0f; mat[13] = -(top + bottom) / (top - bottom);
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = -2.0f / (zfar - znear); mat[14] = -(zfar + znear) / (zfar - znear);
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	void look_at(const vec3 &eye, const vec3 &view, const vec3 &up) {
		vec3 x, y, z;
		mat4 m0, m1;
		z = eye - view;
		//z = dir - eye;
		z.normalize();
		x.cross(up, z);
		x.normalize();
		y.cross(z, x);
		y.normalize();
		m0[0] = x.x; m0[4] = x.y; m0[8] = x.z; m0[12] = 0.0;
		m0[1] = y.x; m0[5] = y.y; m0[9] = y.z; m0[13] = 0.0;
		m0[2] = z.x; m0[6] = z.y; m0[10] = z.z; m0[14] = 0.0;
		m0[3] = 0.0; m0[7] = 0.0; m0[11] = 0.0; m0[15] = 1.0;
		m1.translate(-eye);
		*this = m0 * m1;
	}
	void look_at(const float *eye, const float *dir, const float *up) {
		look_at(vec3(eye), vec3(dir), vec3(up));
	}

	void look_at_dx(const vec3 &eye, const vec3 &view, const vec3 &up) {
		vec3 right, up_vec, forward;
		mat4 m0, m1;
		forward = view - eye;

		forward.normalize();
		right.cross(forward, up);
		right.normalize();
		up_vec.cross(right, forward);
		up_vec.normalize();
		m0[0] = right.x; m0[4] = right.y; m0[8] = right.z; m0[12] = 0.0;
		m0[1] = up_vec.x; m0[5] = up_vec.y; m0[9] = up_vec.z; m0[13] = 0.0;
		m0[2] = forward.x; m0[6] = forward.y; m0[10] = forward.z; m0[14] = 0.0;
		m0[3] = 0.0; m0[7] = 0.0; m0[11] = 0.0; m0[15] = 1.0;
		m1.translate(-eye);
		*this = m0 * m1;
	}

	float mat[16];
};


#endif /* __MATHLIB_H__ */
