#include "Target.h"


//Transfering the camera to this scope, and designating that this is currently being held by the camera
void Target::Selected(CameraTest *localref)
{
	camera = localref; //Camera passed by reference
	held = true; 
}

void Target::Spin(float rate)
{
	
}

void Target::Dropped()
{
	camera = NULL; //Clear the camera pointer whilst it isn't needed.
	held = false; //Set the state to not be held 
	if (Intersects(LocalGoal->functions)==true) //If the block has intersected with the target, end the game if this is the case
	{
		delete camera; //Clear this memory location entirely
		SetPos(LocalGoal->functions.GetPos()); //Set the position to be exactly the goal location
	}
}

void Target::Update()// A general function that will run through all the functions of this object.
{

}