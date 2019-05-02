
#include "Object.h"
#include "GUtility.h"
#include "DDSTextureLoader.h"
Object::Object(void) //Default object with origin at 0,0,0
{
	this->pos = XMFLOAT3(0, 0, 0);
	bounding = false;
	maxX = 1;
	maxY = 1;
	minX = -1;
	minY = -1;
	minZ = -1;
	maxZ = 1;
}
Object::Object(XMFLOAT3 position) //Create the object at the specified position
{
	this->pos = position;
	bounding = false;
	maxX = 1;
	maxY = 1;
	minX = -1;
	minY = -1;
	minZ = -1;
	maxZ = 1;
}
Object::Object(XMFLOAT3 position, bool bound)
{
	pos = position;
	bounding = bound;
	maxX = 1;
	maxY = 1;
	minX = -1;
	minY = -1;
	minZ = -1;
	maxZ = 1;
}
void Object::SetPos(float x, float y, float z) //Set position to specified location in floats
{
	this->pos = XMFLOAT3(x, y, z);
}
void Object::SetPos(XMFLOAT3 newpos) //Set position to specified location in XMFLOAT3 Form
{
	this->pos = XMFLOAT3(newpos.x, newpos.y, newpos.z);
}

XMFLOAT3 Object::GetPos() //Return the position as it is privately held
{
	return XMFLOAT3(this->pos.x, this->pos.y, this->pos.z);
}

void Object::Move(XMFLOAT3 newpos)
{
	this->pos = XMFLOAT3(this->pos.x + newpos.x, this->pos.y + newpos.y, this->pos.z + newpos.z);
}
void Object::Move(float x, float y, float z)
{
	this->pos = XMFLOAT3(this->pos.x + x, this->pos.y + y, this->pos.z);
}

bool Object::Intersects(Object b)
{
	return!(b.pos.x + b.minX > this->pos.x + this->maxX
		|| b.pos.x + b.maxX<this->pos.x + this->minX
		|| b.pos.y + b.maxY<this->pos.y + this->minY
		|| b.pos.y + b.minY>this->pos.y + this->maxY
		|| b.pos.z + b.minZ>this->pos.z + this->maxZ
		|| b.pos.z + b.maxZ < this->pos.z + this->minZ
		);
}

ID3D11ShaderResourceView* Object::Texture() //Return the texture of this object for rendering
{
	return g_pTextureRV;
}
void Object::SetTexture(wchar_t Name[40], ID3D11Device* g_pd3dDevice)
{
	CreateDDSTextureFromFile(g_pd3dDevice, Name, nullptr, &g_pTextureRV);
}
void Object::SetTexture(ID3D11ShaderResourceView* local)
{
	g_pTextureRV = local;
}
void Object::SetRotation(XMFLOAT3 t)
{
	rot = t;
	RotationMatrix = XMMatrixRotationRollPitchYaw(t.x, t.y, t.z);
}
void Object::Rotate(XMFLOAT3 t)
{
	rot = XMFLOAT3(rot.x+t.x, rot.y+t.y, rot.z+t.z);
	RotationMatrix = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
}

//Return Rotation as XMFLOAT3
XMFLOAT3 Object::GetRotation() 
{
	return rot;
} 
XMMATRIX Object::GetRotationMatrix()
{
	return RotationMatrix;
}

//bool Intersects(object2 a, object2 b) //Check all axis to see if they're more than any of the others and if they are collision has not occurred
//{
	//return!(b.x + b.minX > a.maxX
///		|| b.x + b.maxX<a.minX
//		|| b.y + b.maxY<a.minY
//		|| b.y + b.minY>a.maxY
//		|| b.z + b.minZ>a.maxZ
//		|| b.z + b.maxZ < a.minZ);
//}


//bool Object::Intersects(Object b)
//{
//	XMFLOAT3 c = b.GetPos();
//	return!(c.x + b.minX > this->maxX
	//	|| c.x + b.maxX<this->minX
		//|| c.y + b.maxY<this->minY
		//|| c.y + b.minY>this->maxY
		//|| c.z + b.minZ>this->maxZ
		//|| c.z + b.maxZ < this->minZ);
//}