#ifndef MODEL_GEOCOORDINATE
#define MODEL_GEOCOORDINATE

#include <string>

namespace model
{
	struct GeoCoordinate
	{
		double degrees;
		double minutues;
		double seconds;
		char direction;
	};
}

#endif // !MODEL_GEOCOORDINATE