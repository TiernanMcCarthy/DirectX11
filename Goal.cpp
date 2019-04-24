#include "Goal.h"

bool Goal::Finished()
{
	return Done;
}
Object Goal::SupplyObject()
{
	return function;
}