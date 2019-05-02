#pragma once
#include "GUtility.h"
#include "VectorMaths.h"
#include <math.h>
XMFLOAT3 Right = XMFLOAT3(1, 0, 0);



const float pi = 3.14159265359f;
float DegtoRag(float v) //Degrees to Radians
{
	float y = v * pi / 180;
	return y;
}



XMFLOAT3 VectorSubtraction(XMFLOAT3 v1, XMFLOAT3 v2)//Function for subtracting two vectors and returning the result
{
	XMFLOAT3 v3 = XMFLOAT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	//XMFLOAT3 v3 = v1 - v2;
	return (v3);
}

XMFLOAT3 EulerToDirection(float x, float y, float z)//Geting a direction from an Euler Vector
{
	XMFLOAT3 temp;
	temp.x = cosf(x) * sinf(y);
	temp.y = sinf(x);
	temp.z = cosf(y) * cosf(x);
	return temp;
}


XMFLOAT3 ForwardDirection(XMFLOAT3 euler)  //Supply a Forward Direction Vector
{
	XMFLOAT3 EulerRotation;
	EulerRotation.x = euler.x;
	EulerRotation.y = euler.y;
	EulerRotation.z = euler.z;
	return EulerToDirection(EulerRotation.x, EulerRotation.y, EulerRotation.z);
}

float VectorMagnitude(XMFLOAT3 v1) // Return the Length/Magnitude of the vector
{
	float temp = sqrtf((v1.x * v1.x) + (v1.y * v1.y) + (v1.z * v1.z));
	return temp;
}


float LengthSq(XMFLOAT3 v1) //Return the Length Squared of the Vector
{
	float temp = VectorMagnitude(v1);
	return (temp * temp);
}

//As I began I was unsure of the ideal data types availabe to me for storing coordinates, as such I began using XMFLOAT3s for the simplicity of writing XMFLOAT3(0,0,0)

//However I realised no operations for the data type were written and for convenience wrote my own operator functions

XMFLOAT3 operator *(XMFLOAT3 a, float t) //Allow the multiplication of two XMFLOAT3 Vectors, as it is not natively supported
{
	XMFLOAT3 result = XMFLOAT3(a.x*t, a.y*t, a.z*t);
	return result;
}
XMFLOAT3 operator +(XMFLOAT3 a, XMFLOAT3 t) //Allow the Addition of two XMFLOAT3 Vectors, as it isn't natively supported
{
	XMFLOAT3 result = XMFLOAT3(a.x + t.x, a.y + t.y, a.z + t.z);
	return result;
}
bool operator ==(XMFLOAT3 a, XMFLOAT3 t) //Allow comparisons of two XMFLOAT3s because DirectX doesn't support it
{
	if (a.x==t.x && a.y==t.y && a.z==t.z)
	{
		return true;
	}
	return false;
}
XMFLOAT3 operator -(XMFLOAT3 a, XMFLOAT3 t)
{
	XMFLOAT3 result = XMFLOAT3(a.x - t.x, a.y - t.y, a.z - t.z);
	return result;
}
XMFLOAT2 operator -(XMFLOAT2 a, XMFLOAT2 t) //Allow the minus of two XMFLOAT2s
{
	return XMFLOAT2(a.x - t.x, a.y - t.y);
}
XMFLOAT3 VectorLerp(XMFLOAT3 A, XMFLOAT3 B, float t)  //Lerp values between two Vectors
{
	return A * (1.0f - t) + B * t;
}
XMFLOAT3 operator +=(XMFLOAT3 a, XMFLOAT3 b) //Simplify addition and saving with +=
{
	return XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

XMFLOAT3 VectorCrossProduct(XMFLOAT3 A, XMFLOAT3 B) //Return the Crossproduct of two XMFLOAT3s
{
	XMFLOAT3 C = XMFLOAT3();
	C.x = A.y * B.z - A.z * B.y;
	C.y = A.z * B.x - A.x * B.z;
	C.z = A.x * B.y - A.y * B.x;
	return C;
}