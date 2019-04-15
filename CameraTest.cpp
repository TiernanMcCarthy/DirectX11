
#include "CameraTest.h"
#include "GUtility.h"
CameraTest::CameraTest(void)
{
	Eye = XMFLOAT3(0.0f, 5, -5.0f);
	posVector = XMLoadFloat3(&this->Eye);
	Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rotVector = XMLoadFloat3(&this->Rotation);
	UpdateViewMatrix();
}




void CameraTest::SetProjection(float fovDeg, float aspectRation, float nears, float fars)
{
	float fovrads = (fovDeg / 360.0f)*XM_2PI;
	ProjectionMatrix = XMMatrixPerspectiveFovLH(fovrads, aspectRation, nears, fars);
}

const XMMATRIX & CameraTest::GetViewMatrix() const
{
	return this->ViewMatix;
}

const XMVECTOR &CameraTest::GetPosVector() const
{
	return posVector;
}
const XMFLOAT3 &CameraTest::GetPos() const
{
	return Eye;
}

const XMVECTOR & CameraTest::GetRotationVector() const
{
	return this->rotVector;
}
const XMMATRIX & CameraTest::GetProjectionMatrix() const
{
	return this->ProjectionMatrix;
}
const XMFLOAT3 & CameraTest::GetRotionFloat3() const
{
	return this->Rotation;
}

void CameraTest::SetPos(const XMVECTOR & pos)
{
	XMStoreFloat3(&this->Eye, pos);
	this->posVector = pos;
	this->UpdateViewMatrix();
}

void CameraTest::SetPos(float x, float y, float z)
{
	this->Eye = XMFLOAT3(x, y, z);
	this->posVector = XMLoadFloat3(&this->Eye);
	this->UpdateViewMatrix();
}
void CameraTest::MoveFrom(float x, float y, float z)
{
	this->Eye.x += x;
	this->Eye.y += y;
	this->Eye.z += z;
	this->posVector = XMLoadFloat3(&this->Eye);
	this->UpdateViewMatrix();
}

void CameraTest::SetRotation(float x, float y, float z)
{
	this->Rotation = XMFLOAT3(x, y, z);
	this->rotVector = XMLoadFloat3(&this->Rotation);
	this->UpdateViewMatrix();
}
void CameraTest::SetRotation(const XMVECTOR & rot)
{
	this->rotVector = rot;
	XMStoreFloat3(&this->Rotation, rot);
	this->UpdateViewMatrix();
}


void CameraTest::Rotate(float x, float y, float z)
{
	Rotation.x += x;
	Rotation.y += y;
	Rotation.z += z;
	this->UpdateViewMatrix();
}
//https://www.youtube.com/watch?v=yKscl6GuUVI&list=PLcacUGyBsOIBlGyQQWzp6D1Xn6ZENx9Y2&index=33
void CameraTest::UpdateViewMatrix() //Update the view matrix and the movement
{
	//Calculate Rotation Matrix
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(this->Rotation.x, this->Rotation.y, this->Rotation.z);
	//Calculate where the target vector will be now
	XMVECTOR TargetVector = XMVector3TransformCoord(this->DefaultForward, camRotationMatrix);

	//Offset Target by current position
	TargetVector += this->posVector;

	//Calculate updirection on current rotation
	XMVECTOR upDirection = XMVector3TransformCoord(this->DefaultUp, camRotationMatrix);

	//create the view matrix with the new data
	this->ViewMatix = XMMatrixLookAtLH(this->posVector, TargetVector, upDirection);


}
