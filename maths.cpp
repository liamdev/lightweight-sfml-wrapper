#include "maths.h"

//////////////////////////////////////////////////////////////////////////
// Geometry helpers.
//////////////////////////////////////////////////////////////////////////

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
