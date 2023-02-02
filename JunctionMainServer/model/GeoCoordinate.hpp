#ifndef MODEL_GEOCOORDINATE_HPP
#define MODEL_GEOCOORDINATE_HPP

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
} // namespace model

#endif // #MODEL_GEOCOORDINATE_HPP