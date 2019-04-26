#pragma once
#include "GUtility.h"
#include "AABB.h"
class Object //Base object for all rendered objects in the scene, other classes may derive from it
{
public:
	Object();//Blank constructor that will start the object at 0,0
	Object(XMFLOAT3 position); //Object that will start at the designated location
	Object(XMFLOAT3 position, bool bound);



	void SetPos(XMFLOAT3 newpos); //Set postion in XMFLOAT3
	void SetPos(float x, float y, float z); //Set pos with 3 floats
	XMFLOAT3 GetPos(); //Get the location of the object
	bool Intersects(Object b); //Checks if this object intersects with object B

	void Move(XMFLOAT3 newpos);
	void Move(float x, float y, float z);

	void SetTexture(wchar_t Name[40], ID3D11Device* g_pd3dDevice);
	void SetTexture(ID3D11ShaderResourceView* local);
	void SetRotation(XMFLOAT3 t);
	void Rotate(XMFLOAT3 t);
	//Return Rotation as XMFLOAT3
	XMFLOAT3 GetRotation();
	XMMATRIX GetRotationMatrix();
	ID3D11ShaderResourceView* Texture();
	//Bounding Box
	float maxX;
	float maxY;
	float minX;
	float minY;
	float minZ;
	float maxZ;
	AABB box = AABB(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1)); //AABB box temporarily public for testing
private:
	XMFLOAT3 pos; //Position of the object. Position can be accessed by a get position function
	XMFLOAT3 rot; //Rotation of the object. Accessed through get and set functions
	XMMATRIX RotationMatrix;
	bool active;
	bool bounding;
	ID3D11ShaderResourceView* g_pTextureRV = nullptr;


};