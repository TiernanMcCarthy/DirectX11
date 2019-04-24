#pragma once
#include "Object.h"

class Goal
{
public:
	//Object functions;
	Object function;
	bool GetFinished(); //Used to check if the goal has finished its job.
	Object SupplyObject();

	bool Finished(); //Used to set the finished value

private:
	bool Done = false;
};