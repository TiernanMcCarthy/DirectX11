#include "GUtility.h"
class CameraOwn
{
public:
	CameraOwn(void); //No operator camera creator, default it at 0,0,0


private:
	void StartViewMatrix(); //Take the view matrix from the camera's position and target and the up to work out what's happening
public:
	// Camera Angle, window dimensions(well client in Windows Programming), nearest and furthest planes for projection
	//Perspective Matrix
	void StartProjectionMatrix(const float angle, const float W_Width, const float W_Height, const float nearest, const float farthest);


	void DimensionChange(int width, int height); //If window dimensions change

	void SetPos(XMFLOAT3 pos); //Set the position of the camera

	void Rotate(float angle, XMFLOAT3 axis);

	void UpdateCam();


	const XMFLOAT3 Ups() { return GMathVF(GMathFV(Up) - GMathFV(Pos)); }

	XMMATRIX ReturnViewMatrix();
//	static XMFLOAT3 operator +(XMFLOAT3 a, XMFLOAT3 b);

private:

	XMFLOAT3 Pos; //Camera Position
	XMFLOAT3 Eye; //Camera Target Location
	XMFLOAT3 Up; //Camera Up parameter 
	XMVECTOR EyeDefault;
	//Projection parameters
	float angle; //Angle of frustrum
	float ClientWidth; //Client window dimensions
	float ClientHeight;
	float Nearest; //Nearest and furthest Planes for frustrum
	float Furthest;

	float pitch;
	float yaw;
	float roll;

	XMMATRIX RotationMatrix;
	XMFLOAT4X4 ViewMatrix; //Duh
	XMFLOAT4X4 ProjectionMatrix;
	XMFLOAT4X4 OrthographicMatrix; 
};