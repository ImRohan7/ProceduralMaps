#include "Generator.h"
#include "Math/Vector.h"
#include "math.h"

namespace Helpers {
	
	/* Returns random point in a circle
	@Parameter radius : float number specifying the radius
	*/
	FVector Generator::getRandomPointInCircle(float radius)
	{
		FVector res = FVector::ZeroVector;;
		float r = radius * sqrt(FMath::RandRange(0.0f, 1.0f));
		float theta = (FMath::RandRange(0.0f, 1.0f)) * 2 * PI;

		res.X = r * cos(theta);
		res.Y = r * sin(theta);

		return res;
	}
}