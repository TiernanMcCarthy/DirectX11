#pragma once
#include "Object.h"
#include "CameraTest.h"
#include "Goal.h"
class Target:public Object
{
public:
	Target(XMFLOAT3 tempy);
	Target();
	void Update();// A general function that will run through all the functions of this object. 
	void Dropped();
	void Selected(CameraTest *localref); //Public method for setting the pickup state, whilst initialising the camera
private:
	void Spin(float rate); //A local function that will rotate the object on the all axis whilst it isn't being held
	bool held=false; //A boolean that designates if the target block is being held
	CameraTest *camera = new CameraTest; //A pointer that references the scene camera for direct manipulation
	Goal *LocalGoal= new Goal; //A pointer that references the target object for intersection tests
	bool finished = false;
};