#include "CameraOwn.h"
#include "GUtility.h"
CameraOwn::CameraOwn(void)
{
	Pos = XMFLOAT3(0.0f, 5, -5); //Position just behind the target
	XMVECTOR Posy = XMVectorSet(0, 5, -5,0);
	Eye = XMFLOAT3(0, 1.0f, 0.0f);   //Set the target
	XMVECTOR RotationEye = XMVectorSet(Eye.x, Eye.y, Eye.z);
	Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	EyeDefault = XMVectorSet(0, 1.0f, 0.0f,0);
	yaw = 0;
	roll = 0;
	pitch = 0;
	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;
	RotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, 0); //https://www.braynzarsoft.net/viewtutorial/q16390-8-world-view-and-local-spaces-static-camera
	RotationEye = XMVector3TransformCoord(EyeDefault, RotationMatrix); //https://www.braynzarsoft.net/viewtutorial/q16390-19-first-person-camera
	RotationEye = XMVector3Normalize(RotationEye);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(yaw);

	XMVECTOR Right = XMVector3TransformCoord(EyeDefault, RotateYTempMatrix);
	XMVECTOR Upa = XMVector3TransformCoord(Upa, RotateYTempMatrix);
	XMVECTOR Forward = XMVector3TransformCoord(EyeDefault, RotateYTempMatrix);

	Posy += moveLeftRight * Right;
	Posy += moveBackForward * RotationEye;


	this->StartViewMatrix();

	angle = 0.0f;
	ClientHeight = 0.0f;
	ClientWidth = 0.0f;
	Nearest = 0.0f;
	Furthest = 0.0f;

	//Not really sure what this does https://docs.microsoft.com/en-us/windows/desktop/api/directxmath/nf-directxmath-xmstorefloat4x4
	XMStoreFloat4x4(&ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&ProjectionMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&OrthographicMatrix, XMMatrixIdentity());
}

void CameraOwn::UpdateCam()
{


}



void CameraOwn::Rotate(float angle, XMFLOAT3 axis)
{
	//XMFLOAT4X4 yaw;
	//XMStoreFloat4x4(&yaw, XMMatrixIdentity());
	//XMMatrixRotationZ(angle);
	//Create Rotation Vectors
	if (XMVector3Equal(GMathFV(axis), XMVectorZero()) ||
		angle == 0.0f)
		return;


	XMFLOAT3 look_at_target = GMathVF(GMathFV(Eye) - GMathFV(Pos));
	XMFLOAT3 look_at_up = GMathVF(GMathFV(Up) - GMathFV(Pos));

	look_at_target= GMathVF(XMVector3Transform(GMathFV(look_at_target),
		XMMatrixRotationAxis(GMathFV(axis), XMConvertToRadians(angle))));

	look_at_up = GMathVF(XMVector3Transform(GMathFV(look_at_up),
		XMMatrixRotationAxis(GMathFV(axis), XMConvertToRadians(angle))));

	// restore vectors's end points mTarget and mUp from new rotated vectors
	Eye = GMathVF(GMathFV(Pos) + GMathFV(look_at_target));
	Up = GMathVF(GMathFV(Pos) + GMathFV(look_at_up));

	this->StartViewMatrix();

}

void CameraOwn::StartViewMatrix()
{
	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(XMLoadFloat3(&Pos), XMLoadFloat3(&Eye),
		XMLoadFloat3(&this->Ups())));
}

void CameraOwn::StartProjectionMatrix(const float angles,const float W_Widths,const float W_Heights,const float nearests, const float farthests)
{
	angle = angles;
	ClientWidth = W_Widths;
	ClientHeight = W_Heights;
	Nearest = nearests;
	Furthest = farthests;
	XMStoreFloat4x4(&ProjectionMatrix, XMMatrixPerspectiveFovLH(angle, ClientWidth / ClientHeight,
		Nearest, Furthest));
}
//static CameraOwn::XMFLOAT3 operator +(XMFLOAT3 a, XMFLOAT3 b);
//{
	// XMFLOAT3 temp;
	 //temp.x = a.x + b.x;
	 //temp.y = a.y + b.y;
	 //temp.z = a.z + b.z;
	 //return temp;
//}
void CameraOwn::SetPos(XMFLOAT3 npos)
{
	Pos = npos;
	//Eye.x = Pos.x + EyeDefault.x;
//	Eye.y = Pos.y + EyeDefault.y;
	//Eye.z = Pos.z + EyeDefault.z;
}


XMMATRIX CameraOwn::ReturnViewMatrix()
{
	return XMMatrixLookAtLH(XMVectorSet(Pos.x,Pos.y,Pos.z,0), XMVectorSet(Eye.x, Eye.y, Eye.z, 0), XMVectorSet(Up.x, Up.y, Up.z, 0));
	//return XMMatrixLookAtLH(XMStoreFloat4x4(&ViewMatrix, XMMatrixIdentity()) ,Eye,Up);
}
