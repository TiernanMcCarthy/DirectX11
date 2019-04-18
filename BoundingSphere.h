#include "VectorMaths.h"
#include "GUtility.h"
class BoundingSphere {
public:
	// Use this for initialization
	 XMFLOAT2 CentrePoint;
	 XMFLOAT3 CentrePoint3D;
	 float Radius;

	 BoundingSphere(XMFLOAT2 Centre, float radius);

	 BoundingSphere(XMFLOAT3 Centre, float radius);


	bool Intersects(BoundingSphere Comparison); //d2 <= (r1 + r2)2


private:

};