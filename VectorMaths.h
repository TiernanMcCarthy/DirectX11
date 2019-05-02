#pragma once
#include "GUtility.h"

float DegtoRag(float v); //Degrees to Radians

XMFLOAT3 VectorSubtraction(XMFLOAT3 v1, XMFLOAT3 v2); //Function for subtracting two vectors and returning the result

XMFLOAT3 EulerToDirection(float x, float y, float z); //Geting a direction from an Euler Vector


XMFLOAT3 ForwardDirection(XMFLOAT3 euler); //Supply a Forward Direction Vector
float VectorMagnitude(XMFLOAT3 v1);
float LengthSq(XMFLOAT3 v1);

XMFLOAT3 operator *(XMFLOAT3 a, float t); //Allow the multiplication of two XMFLOAT3 Vectors, as it is not natively supported

XMFLOAT3 operator +(XMFLOAT3 a, XMFLOAT3 t); //Allow the Addition of two XMFLOAT3 Vectors, as it isn't natively supported

bool operator ==(XMFLOAT3 a, XMFLOAT3 t); //Allow compairson of two XMFLOAT3s

XMFLOAT3 operator -(XMFLOAT3 a, XMFLOAT3 t); //Allow the minus of two XMFLOAT3s

XMFLOAT2 operator -(XMFLOAT2 a, XMFLOAT2 t); //Allow the minus of two XMFLOAT2s

XMFLOAT3 operator +=(XMFLOAT3 a, XMFLOAT3 b);

XMFLOAT3 VectorLerp(XMFLOAT3 A, XMFLOAT3 B, float t); //Lerp values between two Vectors

XMFLOAT3 VectorCrossProduct(XMFLOAT3 A, XMFLOAT3 B);

