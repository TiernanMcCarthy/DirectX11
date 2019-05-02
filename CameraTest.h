#pragma once

#include "GUtility.h"
class CameraTest
{
public:

	//All Explained in CameraTest.cpp
	CameraTest(void);
	const XMMATRIX & GetViewMatrix() const;
	const XMMATRIX & GetProjectionMatrix() const;
	const XMFLOAT3 &GetPos() const;
	const XMVECTOR &GetPosVector() const;
	const XMVECTOR & GetRotationVector() const;
	const XMFLOAT3 & GetRotionFloat3() const;
	void SetPos(const XMVECTOR & pos);
	void SetPos(float x, float y, float z);
	void MoveFrom(float x, float y, float z);
	void UpdateViewMatrix();
	void SetRotation(float x, float y, float z);
	void SetRotation(const XMVECTOR &rot);
	void Rotate(float x, float y, float z);
	void MoveFrom(XMFLOAT3 a);
private:
	void SetProjection(float fovDeg,float aspectRation,float nears,float fars);





private:

	XMFLOAT3 Eye; //Pos
	XMFLOAT3 Target; //Focus point
	XMFLOAT3 Up; //FOV e.t.c
	XMFLOAT3 Rotation;
	float speed;

	XMVECTOR posVector; //? https://www.youtube.com/watch?v=sROnNl_7Ex8
	XMVECTOR rotVector;
	XMMATRIX ViewMatix;

	XMMATRIX ProjectionMatrix;


	const XMVECTOR DefaultForward = XMVectorSet(0, 0, 1.0f, 0);
	const XMVECTOR DefaultUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
};
