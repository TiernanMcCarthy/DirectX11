#include "AABB.h"
#include <math.h>
#include "GUtility.h"
#include "VectorMaths.h"

//Define Direcitonal Vectors
#define Righty (XMFLOAT3(1,0,0))
#define Up (XMFLOAT3(0,1,0))
#define Forward (XMFLOAT3(0,0,1))

AABB::AABB(XMFLOAT3 Min, XMFLOAT3 Max)
{
	MinExtent = Min;
	MaxExtent = Max;

}
AABB::AABB()
{

}

//Getters and Setters to make the process a lot easier
float AABB::Top()
{
	return this->MaxExtent.y;
}

float AABB::Bottom()
{
	return this->MinExtent.y;
}

float AABB::Left()
{
	return this->MinExtent.x;
}

float AABB::Right()
{
	return this->MaxExtent.x;
}

float AABB::Front()
{
	return this->MaxExtent.z;
}

float AABB::Back()
{
	return this->MinExtent.z;
}

//bool Intersects(AABB Box1, AABB Box2)
//{
//	return !(Box2.Left() > Box1.Right()
//		|| Box2.Right() < Box1.Left()
//		|| Box2.Top() < Box1.Bottom()
//		|| Box2.Bottom() > Box1.Top()
//		|| Box2.Back() > Box1.Front()
//		|| Box2.Front() > Box1.Back());
//}

bool Intersects(AABB a, AABB b) //Check all axis to see if they're more than any of the others and if they are collision has not occurred
{
	return!(b.pos.x + b.Left() > a.Right()
		|| b.pos.x + b.Right<a.Left()
		|| b.pos.y + b.Top()<a.Bottom()
		|| b.pos.y + b.Bottom()>a.Top()
		|| b.pos.z + b.Back()>a.Front()
		|| b.pos.z + b.Front() < a.Back());
}


bool AABB::IntersectingAxis(XMFLOAT3 Axis, AABB Box, XMFLOAT3 StartPoint, XMFLOAT3 EndPoint, float *Lowest, float *Highest)
{
	//Calculate Minimum and Maximum based on the current axis
	float minimum = 0.0f, maximum = 1.0f;
	if (Axis == Righty)
	{
		minimum = (Box.Left() - StartPoint.x) / (EndPoint.x - StartPoint.x);
		maximum = (Box.Right() - StartPoint.x) / (EndPoint.x - StartPoint.x);
	}
	else if (Axis == Up)
	{
		minimum = (Box.Bottom() - StartPoint.y) / (EndPoint.y - StartPoint.y);
		maximum = (Box.Top() - StartPoint.y) / (EndPoint.y - StartPoint.y);
	}
	else if (Axis == Forward)
	{
		minimum = (Box.Back() - StartPoint.z) / (EndPoint.z - StartPoint.z);
		maximum = (Box.Front() - StartPoint.z) / (EndPoint.z - StartPoint.z);
	}

	if (maximum < minimum)
	{
		//Swap values
		float temp = maximum;
		maximum = minimum;
		minimum = temp;
	}

	//Eliminate non intersections to optimise

	if (maximum < *Lowest)
		return false;
	if (minimum > *Highest)
		return false;

	*Lowest = max(minimum, *Lowest);
	*Highest = min(maximum, *Highest);

	if (*Lowest > *Highest)
	{
		return false;
	}
	return true;
}
bool AABB::LineIntersection(AABB Box, XMFLOAT3 StartPoint, XMFLOAT3 EndPoint, XMFLOAT3 *IntersectionPoint)
{
	//Define initial lowest and highest values
	float *Lowest = new float;
	*Lowest = 0.0f;
	float *Highest = new float;
	*Highest = 1.0f;

	//Default Value for intersection point
	*IntersectionPoint = XMFLOAT3(0, 0, 0);

	//Intersection Checks on every axis 
	if (!IntersectingAxis(Righty, Box, StartPoint, EndPoint, Lowest, Highest))
		return false;
	if (!IntersectingAxis(Up, Box, StartPoint, EndPoint, Lowest, Highest))
		return false;
	if (!IntersectingAxis(Forward, Box, StartPoint, EndPoint, Lowest, Highest))
		return false;

	//Calculate Intersection point using interpolation
	//Not the only method
	*IntersectionPoint = VectorLerp(StartPoint, EndPoint, *Lowest);
	Lowest = NULL;
	delete Lowest;
	Highest = NULL;
	delete Highest;
	return true;
}