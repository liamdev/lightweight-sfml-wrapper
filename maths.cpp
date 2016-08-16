#include "maths.h"

//////////////////////////////////////////////////////////////////////////
// Geometry helpers.
//////////////////////////////////////////////////////////////////////////

bool CircleCircleIntersect(f2 circle1_pos, float circle1_radius, f2 circle2_pos, float circle2_radius)
{
	f2 dist = circle2_pos - circle1_pos;
	return dot(dist, dist) <= pow(circle1_radius + circle1_radius, 2);
}

bool SquareCircleIntersect(f2 square_pos, f2 square_size, f2 circle_pos, float circle_radius)
{
	f2 sc = square_pos + square_size * 0.5f;
	f2 dist = abs(sc - circle_pos);

	if (dist.x > square_size.x * 0.5f + circle_radius) return false;
	if (dist.y > square_size.y * 0.5f + circle_radius) return false;

	if (dist.x <= square_size.x * 0.5f) return true;
	if (dist.y <= square_size.y * 0.5f) return true;

	float cornerdist = pow(dist.x - square_size.x * 0.5f, 2) + pow(dist.y - square_size.y * 0.5f, 2);
	return cornerdist <= circle_radius*circle_radius;
}

bool SquareSquareIntersect(f2 square1_pos, f2 square1_size, f2 square2_pos, f2 square2_size)
{
	bool xintersect = abs(square1_pos.x - square2_pos.x) * 2 < abs(square1_size.x + square2_size.y);
	bool yintersect = abs(square1_pos.y - square2_pos.y) * 2 < abs(square1_size.x + square2_size.y);
	return xintersect && yintersect;
}

bool OBBCircleIntersect(f2 bbstart, f2 bbend, float bbwidth, f2 circle_pos, float circle_radius)
{
	// Turn this into square circle intersect by moving the circle into OBB local space.
	f2 dir = normalize(bbend-bbstart);
	f2 perpdir = f2(dir.y, -dir.x);
	f2 worldXorigin = bbstart-perpdir*bbwidth*0.5f;
	float bbang = float(atan2(dir.y, dir.x)-atan2(1, 0));
	float sang=sin(-bbang);
	float cang=cos(-bbang);
	f2 localcirc = circle_pos-worldXorigin;
	localcirc = f2(localcirc.x*cang-localcirc.y*sang, localcirc.x*sang+localcirc.y*cang);
	return SquareCircleIntersect(0, f2(bbwidth, length(bbend-bbstart)), localcirc, circle_radius);
}

//////////////////////////////////////////////////////////////////////////
// Spline functionality.
//////////////////////////////////////////////////////////////////////////

f2 EvalHermite(f2 p1, f2 t1, f2 p2, f2 t2, float t)
{
	float tt = t * t;
	float ttt = tt * t;

	float ttt2 = 2 * ttt;
	float tt3 = 3 * tt;

	float a = ttt2 - tt3 + 1;
	float b = -ttt2 + tt3;
	float c = ttt - 2*tt + t;
	float d = ttt - tt;

	return p1*a + p2*b + t1*c + t2*d;
}

f2 EvalCatmullRom(f2 p1, f2 p2, f2 p3, f2 p4, float t)
{
	const float s = 0.5f;
	float tt = t * t;
	float ttt = tt * t;

	float a = -s*ttt + 2*s*tt - s*t;
	float b = (2-s)*ttt + (s-3)*tt + 1;
	float c = (s-2)*ttt + (3-2*s)*tt + s*t;
	float d = s*ttt - s*tt;

	return p1*a + p2*b + p3*c + p4*d;
}

f2 EvalBezier(f2 p1, f2 c1, f2 p2, f2 c2, float t)
{
	float tt = t * t;
	float ttt = tt * t;
	float u = 1 - t;
	float uu = u * u;
	float uuu = uu * u;

	float a = uuu;
	float b = 3 * uu * t;
	float c = 3 * u * tt;
	float d = ttt;

	return p1*a + c1*b + c2*c + p2*d;
}

//////////////////////////////////////////////////////////////////////////
// Easing curves.
//////////////////////////////////////////////////////////////////////////

float EaseIn(float t, EaseType easing)
{
	switch(easing)
	{
		case EaseType::Linear:		return t;
		case EaseType::Quadratic:	return t*t;
		case EaseType::Cubic:		return t*t*t;
		case EaseType::Quartic:		return t*t*t*t;
		case EaseType::Quintic:		return t*t*t*t*t;
		case EaseType::Sine:		return float(1-cos(PI*0.5f*t));
		case EaseType::Exp:			return exp(10*(t-1));
		default:					printf("Unrecognised easing requested.\n"); return t;
	}
}

float EaseOut(float t, EaseType easing)
{
	switch(easing)
	{
		case EaseType::Linear:		return t;
		case EaseType::Quadratic:	{ float i=t-1; return 1-i*i; }
		case EaseType::Cubic:		{ float i=t-1; return 1+i*i*i; }
		case EaseType::Quartic:		{ float i=t-1; return 1-i*i*i*i; }
		case EaseType::Quintic:		{ float i=t-1; return 1+i*i*i*i*i; }
		case EaseType::Sine:		return float(sin(t*PI*0.5f));
		case EaseType::Exp:			return 1-exp(-10*t);
		default:					printf("Unrecognised easing requested.\n"); return t;
	}
}

float EaseInOut(float t, EaseType easing)
{
	switch(easing)
	{
		case EaseType::Linear:		return t;
		case EaseType::Quadratic:	{ float i=t-1; float v=t*2; if(v < 1) return t*v; return 1-i*i*2; };
		case EaseType::Cubic:		{ float i=t-1; float v=t*2; if(v < 1) return t*v*v; return 1+i*i*i*4; };
		case EaseType::Quartic:		{ float i=t-1; float v=t*2; if(v < 1) return t*v*v*v; return 1-i*i*i*i*8; };
		case EaseType::Quintic:		{ float i=t-1; float v=t*2; if(v < 1) return t*v*v*v*v; return 1+i*i*i*i*i*16; };
		case EaseType::Sine:		return float((1-cos(PI*t))*0.5f);
		case EaseType::Exp:			{ float v=t*2;  if(v < 1) return exp(10*(v-1)); return 1-exp(-10*t); }
		default:					printf("Unrecognised easing requested.\n"); return t;
	}
}


//////////////////////////////////////////////////////////////////////////
// Colour
//////////////////////////////////////////////////////////////////////////

f3 RgbToHsv(f3 rgb)
{
	rgb = clamp(rgb, f3(0), f3(1));

	float low  = min(rgb.x, min(rgb.y, rgb.z));
	float high = max(rgb.x, max(rgb.y, rgb.z));
	float delta = high - low;

	if(high == 0 || delta == 0)
		return f3(0, 0, high);

	float h = 0;
	float s = delta / high;
	float v = high;

	if(rgb.x == high)
		h = (rgb.y - rgb.z) / delta;
	else if(rgb.y == high)
		h = 2 + (rgb.z - rgb.x) / delta;
	else
		h = 4 + (rgb.x - rgb.y) / delta;

	h *= 60;
	if(h < 0)
		h += 360;

	return f3(h/360.0f, s, v);
}

f3 HsvToRgb(f3 hsv)
{
	if(hsv.y == 0)
		return f3(hsv.z);

	if(hsv.x < 0)
		hsv.x = fmod(hsv.x, 1.0f) + 1;
	hsv.x *= 6;

	int i	= (int)floor(hsv.x);
	float f	= hsv.x - i;
	float p = hsv.z * (1 - hsv.y);
	float q = hsv.z * (1 - hsv.y * f);
	float t = hsv.z * (1 - hsv.y * (1 - f));

	switch(i)
	{
		case 0: return f3(hsv.z, t, p);
		case 1: return f3(q, hsv.z, p);
		case 2: return f3(p, hsv.z, t);
		case 3: return f3(p, q, hsv.z);
		case 4: return f3(t, p, hsv.z);
		default:return f3(hsv.z, p, q);
	};
}
