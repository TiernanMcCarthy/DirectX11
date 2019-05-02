
#include "CameraTest.h"
#include "GUtility.h"

//Adapted and learned from https://www.youtube.com/watch?v=yKscl6GuUVI&list=PLcacUGyBsOIBlGyQQWzp6D1Xn6ZENx9Y2&index=33 https://github.com/Pindrought/DirectX-11-Engine-VS2017/blob/Tutorial_33/DirectX%2011%20Engine%20VS2017/DirectX%2011%20Engine%20VS2017/Graphics/Camera.cpp


CameraTest::CameraTest(void) //Construct the camera with default parameters 
{
	Eye = XMFLOAT3(0.0f, 5, -5.0f); //Set the Eyeof the camera to be 0,5,-5.0f as a default postion 
	posVector = XMLoadFloat3(&this->Eye);  
	Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f); //Basic Unrotated object
	rotVector = XMLoadFloat3(&this->Rotation); //Load this into a Matrix
	UpdateViewMatrix(); //Update the view matrix with this new information
}




void CameraTest::SetProjection(float Fov, float aspect, float nears, float fars)
{
	float fovrads = (Fov / 360.0f)*XM_2PI;  //Convert the FOV to radians
	ProjectionMatrix = XMMatrixPerspectiveFovLH(fovrads, aspect, nears, fars); //Set the projection Matrix to match this setup
}

const XMMATRIX & CameraTest::GetViewMatrix() const //Return the View Matrix
{
	return this->ViewMatix;
}

const XMVECTOR &CameraTest::GetPosVector() const //Return the postion of the object as a XMVECTOR
{
	return posVector;
}
const XMFLOAT3 &CameraTest::GetPos() const //Return the position of the camera as an XMFLOAT3
{
	return Eye;
}

const XMVECTOR & CameraTest::GetRotationVector() const //Return the Rotation as XMVECTOR
{
	return this->rotVector;
}
const XMMATRIX & CameraTest::GetProjectionMatrix() const //Return the Projection Matrix 
{
	return this->ProjectionMatrix;
}
const XMFLOAT3 & CameraTest::GetRotionFloat3() const //Return the rotation as a XMFLOAT3
{
	return this->Rotation;
}

void CameraTest::SetPos(const XMVECTOR & pos) //Set the position of the Camera as an XMVECTOR
{
	XMStoreFloat3(&this->Eye, pos); //Store this into Eye
	this->posVector = pos;
	this->UpdateViewMatrix(); //Update the position and the view matrix
}

void CameraTest::SetPos(float x, float y, float z) //Set the position as three floats
{
	this->Eye = XMFLOAT3(x, y, z);
	this->posVector = XMLoadFloat3(&this->Eye);
	this->UpdateViewMatrix();
}
void CameraTest::MoveFrom(float x, float y, float z) //Move from using three floats
{
	this->Eye.x += x;
	this->Eye.y += y;
	this->Eye.z += z;
	this->posVector = XMLoadFloat3(&this->Eye); // Store this in Eye
	this->UpdateViewMatrix(); //Apply this change in the view matrix
}

void CameraTest::MoveFrom(XMFLOAT3 a) //Move from in the form XMFLOAT3
{
	this->Eye.x += a.x;
	this->Eye.y += a.y;
	this->Eye.z += a.z;
	this->posVector = XMLoadFloat3(&this->Eye);
	this->UpdateViewMatrix();
}

void CameraTest::SetRotation(float x, float y, float z) //Set the Rotation as 3 floats
{
	this->Rotation = XMFLOAT3(x, y, z);
	this->rotVector = XMLoadFloat3(&this->Rotation);
	this->UpdateViewMatrix();
}
void CameraTest::SetRotation(const XMVECTOR & rot) //Set the rotation as an XMVECTOR
{
	this->rotVector = rot;
	XMStoreFloat3(&this->Rotation, rot);
	this->UpdateViewMatrix();
}


void CameraTest::Rotate(float x, float y, float z) //Rotate with three floats 
{
	Rotation.y += y; //Add this change to y axis rotation

	if (Rotation.x> 1.570796f)//Lock the camera from going more than 90 degrees up or down in Radians
	{
		Rotation.x = 1.570796f;
	}
	else if (Rotation.x< -1.570796f) 
	{
		Rotation.x = -1.570796f;
	}
	else  //Just add the rotation
	{
		Rotation.x += x; 
	}
	Rotation.z += z; //Rotations on the Z axis cannot flip the perspective, this is fine
	this->UpdateViewMatrix(); //Update the view matrix
}

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
