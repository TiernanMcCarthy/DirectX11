
#include "Object.h"
#include "GUtility.h"

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
	if (bounding == true)
	{
		box = AABB(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	}
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
void Object::Move(float x,float y,float z)
{
	this->pos = XMFLOAT3(this->pos.x + x, this->pos.y + y, this->pos.z);
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