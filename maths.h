#pragma once

#include <math.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////

const double PI = 3.14159265358979323846264338327;
const double RAD_TO_DEG = 180.0 / PI;
const double DEG_TO_RAD = PI / 180.0;

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////

template <typename T> inline T min(T a, T b) { return a < b ? a : b; }
template <typename T> inline T max(T a, T b) { return a > b ? a : b; }
template <typename T> inline T lerp(T a, T b, float t) { return a + (b - a) * t; }
template <typename T> inline T clamp(T v, T minval, T maxval) { return min(max(v, minval), maxval); }

template <typename T> inline int sign(T v) { return v < 0 ? -1 : 1; }

//////////////////////////////////////////////////////////////////////////
// Vector types.
//////////////////////////////////////////////////////////////////////////

template <typename T>
struct vec2
{
	T x, y;
	vec2() : x(0), y(0) {}
	vec2(T _x, T _y) : x(_x), y(_y) {}
	vec2(T xy) : x(xy), y(xy) {}
	vec2<T> operator+(const vec2<T>& v) const { return vec2(x+v.x, y+v.y); }
	vec2<T> operator-(const vec2<T>& v) const { return vec2(x-v.x, y-v.y); }
	vec2<T> operator*(const vec2<T>& v) const { return vec2(x*v.x, y*v.y); }
	vec2<T> operator/(const vec2<T>& v) const { return vec2(x/v.x, y/v.y); }
	vec2<T>& operator+=(const vec2<T>& v) { x += v.x; y += v.y; return *this; }
	vec2<T>& operator-=(const vec2<T>& v) { x -= v.x; y -= v.y; return *this; }
	vec2<T>& operator*=(const vec2<T>& v) { x *= v.x; y *= v.y; return *this; }
	vec2<T>& operator/=(const vec2<T>& v) { x /= v.x; y /= v.y; return *this; }
	vec2<T> operator+(const T v) const { return vec2(x+v, y+v); }
	vec2<T> operator-(const T v) const { return vec2(x-v, y-v); }
	vec2<T> operator*(const T v) const { return vec2(x*v, y*v); }
	vec2<T> operator/(const T v) const { return vec2(x/v, y/v); }
	vec2<T>& operator+=(const T v) { x += v; y += v; return *this; }
	vec2<T>& operator-=(const T v) { x -= v; y -= v; return *this; }
	vec2<T>& operator*=(const T v) { x *= v; y *= v; return *this; }
	vec2<T>& operator/=(const T v) { x /= v; y /= v; return *this; }
};

template <typename T>
struct vec3
{
	T x, y, z;
	vec3() : x(0), y(0), z(0) {}
	vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
	vec3(T xyz) : x(xyz), y(xyz), z(xyz) {}
	vec3<T> operator+(const vec3<T>& v) const { return vec3(x+v.x, y+v.y, z+v.z); }
	vec3<T> operator-(const vec3<T>& v) const { return vec3(x-v.x, y-v.y, z-v.z); }
	vec3<T> operator*(const vec3<T>& v) const { return vec3(x*v.x, y*v.y, z*v.z); }
	vec3<T> operator/(const vec3<T>& v) const { return vec3(x/v.x, y/v.y, z/v.z); }
	vec3<T>& operator+=(const vec3<T>& v) { x += v.x; y += v.y; z += v.z; return *this; }
	vec3<T>& operator-=(const vec3<T>& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	vec3<T>& operator*=(const vec3<T>& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
	vec3<T>& operator/=(const vec3<T>& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
	vec3<T> operator+(const T v) const { return vec3(x+v, y+v, z+v); }
	vec3<T> operator-(const T v) const { return vec3(x-v, y-v, z-v); }
	vec3<T> operator*(const T v) const { return vec3(x*v, y*v, z*v); }
	vec3<T> operator/(const T v) const { return vec3(x/v, y/v, z/v); }
	vec3<T>& operator+=(const T v) { x += v; y += v; z += v; return *this; }
	vec3<T>& operator-=(const T v) { x -= v; y -= v; z -= v; return *this; }
	vec3<T>& operator*=(const T v) { x *= v; y *= v; z *= v; return *this; }
	vec3<T>& operator/=(const T v) { x /= v; y /= v; z /= v; return *this; }
};

template <typename T>
struct vec4
{
	T x, y, z, w;
	vec4() : x(0), y(0), z(0), w(0) {}
	vec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}
	vec4(T xyzw) : x(xyzw), y(xyzw), z(xyzw), w(xyzw) {}
	vec4<T> operator+(const vec4<T>& v) const { return vec4(x+v.x, y+v.y, z+v.z, w+v.w); }
	vec4<T> operator-(const vec4<T>& v) const { return vec4(x-v.x, y-v.y, z-v.z, w-v.w); }
	vec4<T> operator*(const vec4<T>& v) const { return vec4(x*v.x, y*v.y, z*v.z, w*v.w); }
	vec4<T> operator/(const vec4<T>& v) const { return vec4(x/v.x, y/v.y, z/v.z, w/v.w); }
	vec4<T>& operator+=(const vec4<T>& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
	vec4<T>& operator-=(const vec4<T>& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	vec4<T>& operator*=(const vec4<T>& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
	vec4<T>& operator/=(const vec4<T>& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
	vec4<T> operator+(const T v) const { return vec4(x+v, y+v, z+v, w+v); }
	vec4<T> operator-(const T v) const { return vec4(x-v, y-v, z-v, w-v); }
	vec4<T> operator*(const T v) const { return vec4(x*v, y*v, z*v, w*v); }
	vec4<T> operator/(const T v) const { return vec4(x/v, y/v, z/v, w/v); }
	vec4<T>& operator+=(const T v) { x += v; y += v; z += v; w += v; return *this; }
	vec4<T>& operator-=(const T v) { x -= v; y -= v; z -= v; w -= v; return *this; }
	vec4<T>& operator*=(const T v) { x *= v; y *= v; z *= v; w *= v; return *this; }
	vec4<T>& operator/=(const T v) { x /= v; y /= v; z /= v; w /= v; return *this; }
};

// Typed vector aliases.
typedef vec2<float> f2;
typedef vec3<float> f3;
typedef vec4<float> f4;

//////////////////////////////////////////////////////////////////////////
// 2D vector operations.
//////////////////////////////////////////////////////////////////////////

inline float	dot(f2 a, f2 b)		{ return a.x*b.x + a.y*b.y; }
inline float	squared_len(f2 v)	{ return dot(v, v);  }
inline float	length(f2 v)		{ return sqrt(squared_len(v)); }
inline f2		normalize(f2 v)		{ return v / length(v); }
inline f2		sign(f2 v)			{ return f2(v.x < 0 ? -1.0f : 1.0f, v.y < 0 ? -1.0f : 1.0f); }
inline f2		abs(f2 v)			{ return f2(abs(v.x), abs(v.y)); }
inline f2		min(f2 a, f2 b)		{ return f2(min(a.x, b.x), min(a.y, b.y)); }
inline f2		max(f2 a, f2 b)		{ return f2(max(a.x, b.x), max(a.y, b.y)); }
inline void		print(f2 v)			{ printf("[%f %f]\n", v.x, v.y); }

//////////////////////////////////////////////////////////////////////////
// 3D vector operations.
//////////////////////////////////////////////////////////////////////////

inline float	dot(f3 a, f3 b)		{ return a.x*b.x + a.y*b.y + a.z*b.z; }
inline float	squared_len(f3 v)	{ return dot(v, v); }
inline f3		cross(f3 a, f3 b)	{ return f3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x); }
inline float	length(f3 v)		{ return sqrt(dot(v, v)); }
inline f3		normalize(f3 v)		{ return v / length(v); }
inline f3		sign(f3 v)			{ return f3(v.x < 0 ? -1.0f : 1.0f, v.y < 0 ? -1.0f : 1.0f, v.z < 0 ? -1.0f : 1.0f); }
inline f3		abs(f3 v)			{ return f3(abs(v.x), abs(v.y), abs(v.z)); }
inline f3		min(f3 a, f3 b)		{ return f3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)); }
inline f3		max(f3 a, f3 b)		{ return f3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)); }
inline void		print(f3 v)			{ printf("[%f %f %f]\n", v.x, v.y, v.z); }

//////////////////////////////////////////////////////////////////////////
// 4D vector operations.
//////////////////////////////////////////////////////////////////////////

inline float	dot(f4 a, f4 b)		{ return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
inline float	squared_len(f4 v)	{ return dot(v, v); }
inline float	length(f4 v)		{ return sqrt(squared_len(v)); }
inline f4		normalize(f4 v)		{ return v / length(v); }
inline f4		sign(f4 v)			{ return f4(v.x < 0 ? -1.0f : 1.0f, v.y < 0 ? -1.0f : 1.0f, v.z < 0 ? -1.0f : 1.0f, v.w < 0 ? -1.0f : 1.0f); }
inline f4		abs(f4 v)			{ return f4(abs(v.x), abs(v.y), abs(v.z), abs(v.w)); }
inline f4		min(f4 a, f4 b)		{ return f4(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w)); }
inline f4		max(f4 a, f4 b)		{ return f4(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w)); }
inline void		print(f4 v)			{ printf("[%f %f %f %f]\n", v.x, v.y, v.z, v.w); }

//////////////////////////////////////////////////////////////////////////
// Geometry helpers.
//////////////////////////////////////////////////////////////////////////

bool SquareCircleIntersect(f2 square_pos, f2 square_size, f2 circle_pos, float circle_radius);
bool SquareSquareIntersect(f2 square1_pos, f2 square1_size, f2 square2_pos, f2 square2_size);
