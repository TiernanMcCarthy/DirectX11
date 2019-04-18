#include "BoundingSphere.h"






BoundingSphere::BoundingSphere(XMFLOAT2 Centre, float radius)
{
	CentrePoint = Centre;
	Radius = radius;
}
BoundingSphere::BoundingSphere(XMFLOAT3 Centre, float radius)
{
	CentrePoint3D = Centre;
	Radius = radius;
}


bool BoundingSphere::Intersects(BoundingSphere Comparison) //d2 <= (r1 + r2)2

{
	//Create a vector to represent the direction and length in comparison to the other circle
	XMFLOAT3 VectorToOther = Comparison.CentrePoint3D - CentrePoint3D;

	//Calculate the combined radio squared;
	float combinedRadiusSqrt = (Comparison.Radius + Radius);
	combinedRadiusSqrt *= combinedRadiusSqrt;

	return LengthSq(VectorToOther) <= combinedRadiusSqrt;
}