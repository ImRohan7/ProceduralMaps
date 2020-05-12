#include "Generator.h"
#include "Math/Vector.h"
#include "math.h"

namespace Helpers {

	FVector Generator::getRandomPointInCircle(float radius)
	{
		FVector res;
		float r = radius * sqrt(FMath::RandRange(0.0f, 1.0f));
		float theta = (FMath::RandRange(0.0f, 1.0f)) * 2 * PI;

		res.X = r * cos(theta);
		res.Y = r * sin(theta);
		//res.Z = 0;

		return res;
	}
}