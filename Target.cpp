#include "Target.h"

Target::Target(XMFLOAT3 tempy)
{
	SetPos(tempy);
}
Target::Target()
{

}

//Transfering the camera to this scope, and designating that this is currently being held by the camera
void Target::Selected(CameraTest *localref)
{
	camera = localref; //Camera passed by reference
	held = true; 
}

void Target::Spin(float rate)
{
	Rotate(XMFLOAT3(rate,rate,rate));
}

void Target::Dropped()
{
	camera = NULL; //Clear the camera pointer whilst it isn't needed.
	held = false; //Set the state to not be held 
	if (Intersects(LocalGoal->function)==true) //If the block has intersected with the target, end the game if this is the case
	{
		delete camera; //Clear this memory location entirely
		SetPos(LocalGoal->function.GetPos()); //Set the position to be exactly the goal location
		LocalGoal->Finished();
		finished = true;
	}
}

void Target::Update()// A general function that will run through all the functions of this object.
{
	if (held!=true && finished==false)
	{
		Spin(0.02f);
	}
	else if(held==true)
	{
		SetPos(camera->GetPos() + ForwardDirection(camera->GetRotionFloat3()) * 5);
	}
}