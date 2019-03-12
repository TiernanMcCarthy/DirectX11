#include "CameraOwn.h"
#include "GUtility.h"

CameraOwn::CameraOwn(void)
{
	Pos = XMFLOAT3(0.0f, 5, -5); //Position just behind the target
	Eye = XMFLOAT3(0, 1.0f, 0.0f);   //Set the target
	Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

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

XMMATRIX CameraOwn::ReturnViewMatrix()
{
	return XMMatrixLookAtLH(XMVectorSet(Pos.x,Pos.y,Pos.z,0), XMVectorSet(Eye.x, Eye.y, Eye.z, 0), XMVectorSet(Up.x, Up.y, Up.z, 0));
	//return XMMatrixLookAtLH(XMStoreFloat4x4(&ViewMatrix, XMMatrixIdentity()) ,Eye,Up);
}
