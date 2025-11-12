#pragma once

#include "raymath.h"

inline Vector2 Rotate(const Vector2& lhs, const float radians) { return Vector2Transform(lhs, MatrixRotateZ(radians)); }
inline Vector2 RotateDeg(const Vector2& lhs, const float degrees) { return Vector2Transform(lhs, MatrixRotateZ(DEG2RAD * degrees)); }
inline Vector2 GetPerpendicularCW(const Vector2& v) { return Vector2{ v.y, -v.x }; }
inline Vector2 GetPerpendicularCCW(const Vector2& rhs) { return Vector2{ -rhs.y, rhs.x }; }


// Why does raylib not have one with doubles?
inline double Clamp(double value, double min, double max)
{
	double result = (value < min) ? min : value;

	if (result > max)
	{
		result = max;
	}

	return result;
}

// Rotation in degrees. A rotation of zero returns a 1,0 vector, pointing to the screen right side.
inline Vector2 MakeVector(float rotationDegrees)
{
	return Vector2{ cosf(DEG2RAD * rotationDegrees), sinf(DEG2RAD * rotationDegrees) };
}

// Rotation in radians. A rotation of zero returns a 1,0 vector, pointing to the screen right side.
inline Vector2 MakeVectorRad(float rotationRad)
{
	return Vector2{ cosf(rotationRad), sinf(rotationRad) };
}

// Returns the rotation in radians, between -pi and pi.
inline float Atan2(float y, float x)
{
	return atan2f(y, x);
}

// Returns the rotation in degrees, between -180 and 180 degrees.
inline float Atan2Degrees(float y, float x)
{
	return RAD2DEG * atan2f(y, x);
}

// Converts the general mathematical degrees to Raylibs coordinate system.
// Use when interfacing with Raylib functions such as DrawRectangle.
inline float ToRayDeg(float inDegrees)
{
	float temp = inDegrees + 90.0f; 
	if (temp < 0)
	{
		temp += 360.f;
	}

	if (temp > 360)
	{
		temp -= 360.f;
	}

	return temp;
}

//Calculates the shortest direction to move from the original angle to the desired angle
//From is the original angle, to is the desired angle
inline int GetShortestRotation(float from, float to) 
{
	return int((to - from + 540)) % 360 - 180;
}

inline float LerpRotation(float from, float to, float alpha)
{
	const Quaternion fromQuat = QuaternionFromEuler(from * DEG2RAD, 0.0f, 0.0f);
	const Quaternion toQuat = QuaternionFromEuler(to * DEG2RAD, 0.0f, 0.0f);
	Quaternion lerpQuat = QuaternionLerp(fromQuat, toQuat, alpha);

	Vector3 eulerQuat = QuaternionToEuler(lerpQuat);
	return eulerQuat.x * RAD2DEG;
}

inline Vector2 GetRectCenter(Rectangle rect)
{
	return { rect.width / 2.0f, rect.height / 2.0f };
}



// --- Math Notes
// - MakeVector2 with 0 degrees returns a 1,0 vector, pointing to the right side of the screen.
//
// - Positive degrees rotates cw, negative rotates ccw.
//
// - For shapes like rectangles, height is the size on the y-axis before rotations, and width is x.
//
// - origin in calls like DrawRectangle should be thought of as the shapes centerpoint, so the draw rectangles origin. This allows it to rotate around it's own axis
// - It might be desired to not have it be on self for rotating around other objects tho.