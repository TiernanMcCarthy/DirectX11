#pragma once
#include "GUtility.h"
#include "VectorMaths.h"
class AABB
{
public:
	AABB(XMFLOAT3 Min, XMFLOAT3 Max); // Define a minimum and maximum extent for this Bounding Box
	AABB();
	//Getters and Setters to make the process a lot easier
	float Top();
	float Bottom();
	float Left();
	float Right();
	float Front();
	float Back();


	//bool Intersects(AABB Box1, AABB Box2); //A function that is supplied two bounding boxes and checks if they collide
	bool IntersectingAxis(XMFLOAT3 Axis, AABB Box, XMFLOAT3 StartPoint, XMFLOAT3 EndPoint, float *Lowest, float *Highest);
	bool LineIntersection(AABB Box, XMFLOAT3 StartPoint, XMFLOAT3 EndPoint, XMFLOAT3 *IntersectionPoint);
	
		//Set Directional Vectors local to AABB
	XMFLOAT3 Righty;
	XMFLOAT3 Up;
	XMFLOAT3 Forward;
	XMFLOAT3 pos;
private:
	XMFLOAT3 MinExtent;
	XMFLOAT3 MaxExtent;
};

bool Intersects(AABB Box1, AABB Box2);